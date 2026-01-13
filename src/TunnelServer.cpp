#include "TunnelServer.hpp"

TunnelServer::TunnelServer(const std::string& broker_address, const std::string& command_channel_name, const std::string& tun_device_name, const std::string& ip_pool_base, unsigned int ip_pool_size, std::atomic<bool>* run_flag, bool enable_encryption)
    : mqtt_channels_(broker_address, command_channel_name),
      tun_device_(tun_device_name),
      ip_pool_(ip_pool_base, ip_pool_size),
      command_channel_name_(command_channel_name),
      global_run_flag_(run_flag),
      encryption_enabled_(enable_encryption),
      crypto_manager_(CryptoManager::ROLE_SERVER, enable_encryption, false){}

void TunnelServer::start_server() {

    // Old block for setting up TUN device IP address via system calls in C
    // Replace with netlink based config later or C++ wrapper library

    own_ip_address_ = ip_pool_.get_base_ip() + "1"; // currently fixed gateway address for server side in the /24 pool
     
    char ip_addr_own[100];
    //char ip_addr_dst[100];

    snprintf(ip_addr_own, sizeof(ip_addr_own), "ip addr add %s/24 dev tun0", own_ip_address_.c_str());

    system(ip_addr_own);
    system("ip link set tun0 up");

    spdlog::info("TUN device configured with IP: {}", own_ip_address_);

    //snprintf(ip_addr_dst, sizeof(ip_addr_dst), "ip route add %s dev tun0", dst_address);

    connect_command_channel();
    connect_data_channel();
    mqtt_channels_.set_tun_callback(tun_device_.fd(), encryption_enabled_, crypto_manager_);

    tunnel_active_ = true; // setting the atomic flag for async coordination
    tun_read_async_thread_ = std::thread(&TunnelServer::async_tun_read, this); // start async TUN read thread 

    spdlog::info("Tunnel server started");

    auto& command_channel = mqtt_channels_.get_command_client();
    
    mqtt::const_message_ptr msg;

    while (*global_run_flag_) {        

        bool consumed = command_channel.try_consume_message(&msg); // Use non-blocking consume to allow checking run flag

        if (consumed) {
            
            spdlog::info("Consumed message from command channel");

            // check message type and handle accordingly

            std::string payload = msg->get_payload();

            auto header = MessageHeader::from_string(payload);

            switch (header.type)
            {{
                case SESSION_TERMINATION:

                spdlog::info("Received session termination message");
                SessionTermination term_msg = SessionTermination::from_string(msg->get_payload());

                SessionConfig dummy_config;

                if (active_clients_.get_session(term_msg.client_id, dummy_config)) {
                    spdlog::info("Terminating session for client ID: {}", term_msg.client_id);
                } else {
                    spdlog::warn("No active session found for client ID: {}", term_msg.client_id);
                    break;   
                }

                active_clients_.remove_session(term_msg.client_id);
                ip_pool_.release_ip(dummy_config.client_address);
                spdlog::info("Terminated session for client ID: {}", term_msg.client_id);
                
                }
                break;
            
            default:
                spdlog::info("Handling client handshake message");
                handle_client_handshake(msg, static_cast<MessageIdentifier>(header.type));
                break;
            }
            }

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep to reduce CPU usage
    }

    spdlog::info("Server shutting down...");
    stop_server();
}


void TunnelServer::stop_server() {
    tunnel_active_ = false;

    if (tun_read_async_thread_.joinable()) {
        tun_read_async_thread_.join();
        spdlog::info("TUN read thread joined");
    }

    if (command_channel_connected_) {
        mqtt_channels_.get_command_client().disconnect();
        command_channel_connected_ = false;
        spdlog::info("Command channel disconnected");
    }

    if (data_channel_connected_) {
        mqtt_channels_.get_data_client().disconnect()->wait();
        data_channel_connected_ = false;
        spdlog::info("Data channel disconnected");
    }

    
}

// Vulnerable session handshake handler - currently blocking and interruptable by other clients connecting at the same time
// Asynchronous handling with state machines per client would be more robust
// Current implementation for mvp testing 


void TunnelServer::handle_client_handshake(mqtt::const_message_ptr msg, MessageIdentifier message_type) {

    spdlog::info("Received handshake message: {}", msg->get_payload());

    switch (message_type) {
        case CLIENT_HELLO_CRYPTO:
        {
            spdlog::debug("Received Client Hello Crypto message");
            spdlog::debug("Processing Client Hello Crypto...");

            ClientHelloCrypto client_hello_crypto = ClientHelloCrypto::from_string(msg->get_payload());

            ServerHelloCrypto server_hello_crypto = crypto_manager_.establish_server_session(client_hello_crypto);

            spdlog::debug("Processed Client Hello Crypto, sending Server Hello Crypto...");

            std::string server_hello_crypto_serialized = server_hello_crypto.to_string();

            spdlog::debug("Server Hello Crypto message: {}", server_hello_crypto_serialized);

            mqtt::message_ptr crypto_hello_msg = mqtt::make_message(command_channel_name_ + "_TX", server_hello_crypto_serialized);
            crypto_hello_msg->set_qos(1);
            mqtt_channels_.get_command_client().publish(crypto_hello_msg);

            handshake_states_[server_hello_crypto.unique_identifier].session_state = HANDSHAKE_CLIENT_HELLO;

            return;

            break;

        } case CLIENT_HELLO: {

            spdlog::debug("Received Client Hello message");
            spdlog::debug("Processing Client Hello...");

            if(encryption_enabled_) {
                EncryptedWrapper encrypted_wrapper = EncryptedWrapper::from_string(msg->get_payload());

                std::string decrypted_payload = crypto_manager_.decrypt_data(
                    std::vector<unsigned char>(encrypted_wrapper.encrypted_payload.begin(), encrypted_wrapper.encrypted_payload.end()),
                    encrypted_wrapper.client_id
                );

                msg = mqtt::make_message(msg->get_topic(), decrypted_payload);
            }

            ClientHello client_hello = ClientHello::from_string(msg->get_payload());

            // Check current handshake state for this client

            if (handshake_states_.find(client_hello.client_base_id) != handshake_states_.end() &&
               handshake_states_[client_hello.client_base_id].session_state != HANDSHAKE_CLIENT_HELLO) {
                spdlog::warn("Unexpected handshake state for client ID: {}", client_hello.client_base_id);
                return;
            }

            spdlog::info("Received Client Hello from client ID: {}", client_hello.client_base_id);

            std::string assigned_ip = ip_pool_.allocate_ip();
            std::string inbound_topic = command_channel_name_ + "/" + client_hello.client_base_id + "/A";
            std::string outbound_topic = command_channel_name_ + "/" + client_hello.client_base_id + "/B";

            spdlog::info("Assigned IP: {} Inbound Topic: {} Outbound Topic: {}", assigned_ip, inbound_topic, outbound_topic);

            SessionConfig session_config;
            session_config.client_id = client_hello.client_base_id;
            session_config.client_address = assigned_ip;
            session_config.server_address = own_ip_address_;
            session_config.topic_inbound = inbound_topic;
            session_config.topic_outbound = outbound_topic;
        
            // Hash based approach for session ID generation
            // TODO : Refactor message generation to be function based
    
            std::stringstream ss;
            ss << session_config.client_id << "_" << std::chrono::system_clock::now().time_since_epoch().count();

            session_config.session_id = get_sha256_string(ss.str());
            session_config.session_state = HANDSHAKE_CLIENT_ACK;
            session_config.handshake_identifier = client_hello.handshake_identifier;
        
            ServerHello server_hello;
            
            server_hello.handshake_identifier = client_hello.handshake_identifier;
            server_hello.assigned_client_id_ = client_hello.client_base_id;
            server_hello.assigned_client_ip = assigned_ip;
            server_hello.server_address = own_ip_address_;
            server_hello.assigned_inbound_topic = inbound_topic;
            server_hello.assigned_outbound_topic = outbound_topic;
            server_hello.session_id = session_config.session_id;

            mqtt::message_ptr hello_msg = mqtt::make_message(command_channel_name_ + "_TX", server_hello.to_string());
            hello_msg->set_qos(1);
            mqtt_channels_.get_command_client().publish(hello_msg);

            spdlog::info("Sent Server Hello to client ID: {}", client_hello.client_base_id);

            handshake_states_[client_hello.client_base_id] = session_config;

            return;
            break;

        } case CLIENT_ACK: {

            ClientACK client_ack = ClientACK::from_string(msg->get_payload());

            // Check current handshake state for this client

            if(handshake_states_.find(client_ack.client_id) == handshake_states_.end() ||
               handshake_states_[client_ack.client_id].session_state != HANDSHAKE_CLIENT_ACK) {
                spdlog::warn("Unexpected handshake state for client ID: {}", client_ack.client_id);
                return;
            }

            if(client_ack.handshake_identifier != handshake_states_[client_ack.client_id].handshake_identifier) {
                throw std::runtime_error("Handshake identifier mismatch in Client ACK");
            }

            spdlog::info("Received Client ACK from client ID: {}", client_ack.client_id);

        
            ServerACK server_ack;
        
            server_ack.handshake_identifier = handshake_states_[client_ack.client_id].handshake_identifier;
            mqtt::message_ptr server_ack_msg = mqtt::make_message(command_channel_name_ + "_TX", server_ack.to_string());
            server_ack_msg->set_qos(1);
            mqtt_channels_.get_command_client().publish(server_ack_msg);    

            spdlog::info("Sent Server ACK to client ID: {}", client_ack.client_id);

            // Here also old system call based route setup - replace asap

            char ip_addr_dst[100];
            snprintf(ip_addr_dst, sizeof(ip_addr_dst), "ip route add %s dev tun0", handshake_states_[client_ack.client_id].client_address.c_str());
            system(ip_addr_dst);

            // End of old system call based route setup block
        
            mqtt_channels_.get_data_client().subscribe(handshake_states_[client_ack.client_id].topic_outbound, 1)->wait();
            active_clients_.add_session(handshake_states_[client_ack.client_id].client_id, handshake_states_[client_ack.client_id]);
            handshake_states_.erase(client_ack.client_id); // remove from current states map
            
            spdlog::info("Session established for client ID: {} with IP: {}", client_ack.client_id, handshake_states_[client_ack.client_id].client_address);

            break;
        }

        default:
            spdlog::warn("Unknown handshake message type received");
            break;

    }
}
       

       


void TunnelServer::connect_command_channel() {
    auto& command_channel = mqtt_channels_.get_command_client();
    try {
        spdlog::info("Server: Connecting to command channel...");
        command_channel.connect();
        spdlog::info("Server: Connected! Subscribing to topic: {}", command_channel_name_ + "_RX");
        command_channel.subscribe((command_channel_name_+"_RX"), 1);
        spdlog::info("Server: Subscribed successfully");
        command_channel_connected_ = true;

    } catch (const mqtt::exception& exc) {
        std::cerr << "Error connecting to command channel: " << exc.what() << std::endl;
        throw;
    }

   spdlog::info("Command channel connected");
}
 
void TunnelServer::connect_data_channel() {
    auto& data_channel = mqtt_channels_.get_data_client();
    try {
        data_channel.connect()->wait();
        data_channel_connected_ = true;

    } catch (const mqtt::exception& exc) {
        std::cerr << "Error connecting to data channel: " << exc.what() << std::endl;
        throw;
    }

    std::cout << "Data channel connected" << std::endl;
}

void TunnelServer::async_tun_read() {

    std::vector <char> buffer (1500); // MTU size for TUN device

    while(tunnel_active_) {
        ssize_t read_bytes = read(tun_device_.fd(), buffer.data(), buffer.size());

        if(read_bytes < 0) {
            if (errno == EAGAIN && errno == EWOULDBLOCK) {
                // if no data available, sleep and continue the loop
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            } else {
                spdlog::error("Error reading from TUN device: {}", strerror(errno));
                continue;
            }
        }

        struct iphdr* ip_header = reinterpret_cast<struct iphdr*>(buffer.data());
        std::string dest_ip = ip_to_string(ip_header->daddr);

        // Lookup session for destination IP
        SessionConfig session;
        if(!active_clients_.get_session_by_ip(dest_ip, session)) {
            spdlog::warn("No active session for destination IP: {}", dest_ip);
            continue; 
        }

        std::string payload_to_send;

        if (encryption_enabled_) {
            try {
                
                std::vector<unsigned char> plaintext(buffer.begin(), buffer.begin() + read_bytes);
                
                payload_to_send = crypto_manager_.encrypt_data(plaintext, session.client_id);
                
    
            } catch (const std::exception& e) {
                spdlog::error("Encryption failed for client {}: {}", session.client_id, e.what());
                continue;
            }
        } else {

            payload_to_send.assign(buffer.data(), read_bytes);
        }
    
        mqtt::message_ptr pubmsg = mqtt::make_message(
            session.topic_inbound, 
            payload_to_send.data(), 
            payload_to_send.size()
        );

        pubmsg->set_qos(1);
        mqtt_channels_.get_data_client().publish(pubmsg);
    }
};