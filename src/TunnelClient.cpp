#include "TunnelClient.hpp"

TunnelClient::TunnelClient(const std::string& broker_address,
                             const std::string& command_channel_name,
                             const std::string& client_base_id,
                             const std::string& tun_device_name,
                            bool enable_encryption,
                            bool ignore_server_authentication)
    : mqtt_channels_(broker_address, client_base_id),
      tun_device_(tun_device_name),
      command_channel_name_(command_channel_name),
      client_base_id_(client_base_id),
    encryption_enabled(enable_encryption),
    ignore_server_authentication_(ignore_server_authentication),
      crypto_manager_(CryptoManager::ROLE_CLIENT, enable_encryption, ignore_server_authentication) {}

void TunnelClient::start_tunnel() {
    connect_command_channel();
    setup_session();
    connect_data_channel();
    mqtt_channels_.set_tun_callback(tun_device_.fd(), encryption_enabled, crypto_manager_);
    tunnel_active_ = true;
    tun_read_thread_ = std::thread(&TunnelClient::async_tun_read, this);
    spdlog::info("Tunnel client started");
} 

void TunnelClient::stop_tunnel() {
    tunnel_active_ = false;
    if (tun_read_thread_.joinable()) {
        tun_read_thread_.join();
        spdlog::debug("TUN read thread joined");
    }

    // Create and send session termination message
    // TODO: proper reason codes and message generation

    SessionTermination term_msg;
    
    term_msg.client_id = session_config_.client_id;
    term_msg.session_id = session_config_.session_id;
    term_msg.reason = "Client request";

    mqtt::message_ptr term_mqtt_msg = mqtt::make_message(command_channel_name_ + "_RX", term_msg.to_string());
    term_mqtt_msg->set_qos(1);
    mqtt_channels_.get_command_client().publish(term_mqtt_msg);
    

    mqtt_channels_.get_data_client().disconnect()->wait();
    spdlog::info("Data channel disconnected");
}

void TunnelClient::setup_session() {
    ClientHello client_hello;
    client_hello.client_base_id =  client_base_id_;
    client_hello.authentication = false; // not implemented yet
    client_hello.auth_data = "";

    if (encryption_enabled) {

        // Exchange process for Cryptographic keys before further session setup if crypto is enabled

        ClientHelloCrypto client_hello_crypto = crypto_manager_.generate_client_hello(client_base_id_);

        std::string client_hello_crypto_serialized = client_hello_crypto.to_string();

        mqtt::message_ptr crypto_hello_msg = mqtt::make_message(command_channel_name_ + "_RX", client_hello_crypto_serialized);
        crypto_hello_msg->set_qos(1);
        mqtt_channels_.get_command_client().publish(crypto_hello_msg);

        spdlog::debug("Sent Client Hello Crypto, waiting for Server Hello Crypto...");

        mqtt::const_message_ptr crypto_response = mqtt_channels_.get_command_client().consume_message();
        
        // Currently blocking wait for simplicity - refactor to async with state machine later still open TODO

        if(!crypto_response) {
            throw std::runtime_error("Failed to receive Server Hello Crypto message");
        }

        // Process server hello crypto message and establish session keys
        ServerHelloCrypto server_hello_crypto = ServerHelloCrypto::from_string(crypto_response->get_payload());

        spdlog::debug("Received Server Hello Crypto");

        crypto_manager_.establish_client_session(server_hello_crypto);
        // Hotfix - possible shadowing bug here - verify later
        client_base_id_ = server_hello_crypto.unique_identifier;
        client_hello.client_base_id = server_hello_crypto.unique_identifier; // Use unique identifier for further session identification TODO: Fix this logic later

    }
    
    std::random_device rd;
    std::uniform_int_distribution<uint64_t> dist;
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << dist(rd)
       << std::setw(16) << std::setfill('0') << dist(rd);

    client_hello.handshake_identifier = ss.str();
    client_hello.authentication = false; // not implemented yet
    client_hello.auth_data = "";
    
    std::string client_hello_serialized = client_hello.to_string();

    if(encryption_enabled){
        EncryptedWrapper encrypted_wrapper;
        encrypted_wrapper.client_id = client_base_id_;
        
        auto encrypted_payload = crypto_manager_.encrypt_data(std::vector<unsigned char>(client_hello_serialized.begin(), client_hello_serialized.end()), client_base_id_);

        encrypted_wrapper.encrypted_payload = std::vector<uint8_t>(encrypted_payload.begin(), encrypted_payload.end());

        client_hello_serialized = encrypted_wrapper.to_string();
    }

    mqtt::message_ptr hello_msg = mqtt::make_message(command_channel_name_ + "_RX", client_hello_serialized);
    hello_msg->set_qos(1);
    mqtt_channels_.get_command_client().publish(hello_msg);

    spdlog::debug("Sent Client Hello, waiting for Server Hello...");

    mqtt::const_message_ptr response = mqtt_channels_.get_command_client().consume_message();
    if(!response) {
        throw std::runtime_error("Failed to receive Server Hello message");
    }

    std::string response_payload = response->get_payload();

    if(encryption_enabled) {
        EncryptedWrapper encrypted_wrapper = EncryptedWrapper::from_string(response_payload);

        response_payload = crypto_manager_.decrypt_data(
            std::vector<unsigned char>(encrypted_wrapper.encrypted_payload.begin(), encrypted_wrapper.encrypted_payload.end()),
            client_base_id_
        );
    }

    ServerHello server_hello = ServerHello::from_string(response_payload);



    spdlog::debug("Received Server Hello");

    // QUick and dirty session config fix - Refactor later - TODO

    if (!encryption_enabled) {
    session_config_.client_id = server_hello.assigned_client_id_;}
     else {
    session_config_.client_id = client_base_id_; 
}

    session_config_.client_id = server_hello.assigned_client_id_;
    session_config_.client_address = server_hello.assigned_client_ip;
    session_config_.server_address = server_hello.server_address;
    session_config_.topic_inbound = server_hello.assigned_inbound_topic;
    session_config_.topic_outbound = server_hello.assigned_outbound_topic;
    session_config_.session_id = server_hello.session_id;
    
    spdlog::debug("Session configured with Client ID: {}, IP: {}, ServerIP: {} Inbound Topic: {}, Outbound Topic: {}, Session ID: {}",
                 session_config_.client_id,
                 session_config_.client_address,
                 session_config_.server_address,
                 session_config_.topic_inbound,
                 session_config_.topic_outbound,
                session_config_.session_id);

    ClientACK client_ack;
    
    client_ack.handshake_identifier = server_hello.handshake_identifier;
    client_ack.client_id = session_config_.client_id;

    std::string ack_payload = client_ack.to_string();

    if(encryption_enabled) {
        EncryptedWrapper encrypted_wrapper;
        encrypted_wrapper.client_id = client_base_id_;
        
        auto encrypted_payload = crypto_manager_.encrypt_data(std::vector<unsigned char>(ack_payload.begin(), ack_payload.end()), client_base_id_);

        encrypted_wrapper.encrypted_payload = std::vector<uint8_t>(encrypted_payload.begin(), encrypted_payload.end());

        ack_payload = encrypted_wrapper.to_string();
    }

    mqtt::message_ptr ack_msg = mqtt::make_message(command_channel_name_ + "_RX", ack_payload);
    ack_msg->set_qos(1);
    mqtt_channels_.get_command_client().publish(ack_msg); 

    mqtt::const_message_ptr ack_response = mqtt_channels_.get_command_client().consume_message();
    if(!ack_response) {
        throw std::runtime_error("Failed to receive Server ACK message"); 
    }

    std::string ack_response_payload = ack_response->get_payload();

    if(encryption_enabled) {
        EncryptedWrapper encrypted_wrapper = EncryptedWrapper::from_string(ack_response_payload);

        ack_response_payload = crypto_manager_.decrypt_data(
            std::vector<unsigned char>(encrypted_wrapper.encrypted_payload.begin(), encrypted_wrapper.encrypted_payload.end()),
            client_base_id_
        );
    }

    ServerACK server_ack = ServerACK::from_string(ack_response_payload);
    if (server_ack.handshake_identifier != client_hello.handshake_identifier) {
        throw std::runtime_error("Handshake identifier mismatch in Server ACK");
    }
    spdlog::debug("Received Server ACK, session handshake complete");

    char ip_addr_own[100];
    char ip_addr_dst[100];

    snprintf(ip_addr_own, sizeof(ip_addr_own), "ip addr add %s/24 dev tun0", session_config_.client_address.c_str());
    snprintf(ip_addr_dst, sizeof(ip_addr_dst), "ip route add %s dev tun0", session_config_.server_address.c_str());

    spdlog::debug("Configuring TUN device with IP and routes...");
    spdlog::debug("Executing command: {}", ip_addr_own);
    spdlog::debug("Executing command: {}", ip_addr_dst);

    system(ip_addr_own);
    system("ip link set tun0 up");
    system(ip_addr_dst);

    spdlog::debug("TUN device configured with IP: {}", session_config_.client_address);
    spdlog::debug("Route to server address {} added", session_config_.server_address);

    session_configured_ = true;
};

void TunnelClient::connect_command_channel() {
    auto& command_channel = mqtt_channels_.get_command_client();
    try {
        spdlog::debug("Connecting to command channel...");
        command_channel.connect();
        spdlog::debug("Connected! Now subscribing to topic: {}", command_channel_name_ + "_TX");
        command_channel.subscribe(command_channel_name_ + "_TX", 1);
        spdlog::debug("Subscribed successfully");
        spdlog::debug("Command channel connected");
    } catch (const mqtt::exception& exc) {
        spdlog::error("Error connecting to command channel: {}", exc.what());
        throw;
    }
}

void TunnelClient::connect_data_channel() {
    auto& data_channel = mqtt_channels_.get_data_client();
    try {
        data_channel.connect()->wait();
        data_channel.subscribe(session_config_.topic_inbound, 1)->wait();
        spdlog::debug("Data channel connected");
    } catch (const mqtt::exception& exc) {
        spdlog::error("Error connecting to data channel: {}", exc.what());
        throw;
    }
}

void TunnelClient::async_tun_read() {
    
    char buffer[1500];
    std::string data_to_send;
    if (encryption_enabled)
    {
       data_to_send.reserve(1500); 
    }

        while (tunnel_active_) {
            ssize_t bytes_read = read(tun_device_.fd(), buffer, sizeof(buffer));
            if (bytes_read < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                spdlog::error("Error reading from TUN device: {}", strerror(errno));
                continue;
            }
            

            if(encryption_enabled) {
                data_to_send = crypto_manager_.encrypt_data(std::vector<unsigned char>(buffer, buffer + bytes_read), client_base_id_);

                

            } else {
                data_to_send.assign(buffer, bytes_read);
            }

            mqtt::message_ptr pubmsg = mqtt::make_message(session_config_.topic_outbound, data_to_send);
            pubmsg->set_qos(1);
            mqtt_channels_.get_data_client().publish(pubmsg)->wait_for(std::chrono::seconds(10));  
            
        }
    }

