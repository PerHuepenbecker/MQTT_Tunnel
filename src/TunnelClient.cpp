#include "TunnelClient.hpp"

TunnelClient::TunnelClient(const std::string& broker_address,
                             const std::string& command_channel_name,
                             const std::string& client_base_id,
                             const std::string& tun_device_name)
    : mqtt_channels_(broker_address, client_base_id),
      tun_device_(tun_device_name),
      command_channel_name_(command_channel_name),
      client_base_id_(client_base_id) {}

void TunnelClient::start_tunnel() {
    connect_command_channel();
    setup_session();
    connect_data_channel();
    mqtt_channels_.set_tun_callback(tun_device_.fd());
    tunnel_active_ = true;
    async_tun_read();
} 

void TunnelClient::stop_tunnel() {
    tunnel_active_ = false;
    mqtt_channels_.get_data_client().disconnect()->wait();
    spdlog::info("Data channel disconnected");
}

void TunnelClient::setup_session() {
    ClientHello client_hello;
    client_hello.message_identifier = "CLIENT_HELLO";
    client_hello.client_base_id =  client_base_id_;
    client_hello.authentication = false; // not implemented yet
    client_hello.auth_data = "";
    

    std::random_device rd;
    std::uniform_int_distribution<uint64_t> dist;
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << dist(rd)
       << std::setw(16) << std::setfill('0') << dist(rd);

    client_hello.handshake_identifier = ss.str();

    mqtt::message_ptr hello_msg = mqtt::make_message(command_channel_name_ + "_RX", client_hello.to_string());
    hello_msg->set_qos(1);
    mqtt_channels_.get_command_client().publish(hello_msg);

    spdlog::info("Sent Client Hello, waiting for Server Hello...");

    mqtt::const_message_ptr response = mqtt_channels_.get_command_client().consume_message();
    if(!response) {
        throw std::runtime_error("Failed to receive Server Hello message");
    }

    ServerHello server_hello = ServerHello::from_string(response->get_payload());

    spdlog::info("Received Server Hello");

    session_config_.client_id = server_hello.assigned_client_id_;
    session_config_.client_address = server_hello.assigned_client_ip;
    session_config_.topic_inbound = server_hello.assigned_inbound_topic;
    session_config_.topic_outbound = server_hello.assigned_outbound_topic;
    
    spdlog::info("Session configured with Client ID: {}, IP: {}, Inbound Topic: {}, Outbound Topic: {}",
                 session_config_.client_id,
                 session_config_.client_address,
                 session_config_.topic_inbound,
                 session_config_.topic_outbound);

    ClientACK client_ack;
    client_ack.message_identifier = "CLIENT_ACK";
    client_ack.handshake_identifier = server_hello.handshake_identifier;

    mqtt::message_ptr ack_msg = mqtt::make_message(command_channel_name_ + "_RX", client_ack.to_string());
    ack_msg->set_qos(1);
    mqtt_channels_.get_command_client().publish(ack_msg); 

    mqtt::const_message_ptr ack_response = mqtt_channels_.get_command_client().consume_message();
    if(!ack_response) {
        throw std::runtime_error("Failed to receive Server ACK message"); 
    }

    ServerACK server_ack = ServerACK::from_string(ack_response->get_payload());
    if (server_ack.handshake_identifier != client_hello.handshake_identifier) {
        throw std::runtime_error("Handshake identifier mismatch in Server ACK");
    }
    spdlog::info("Received Server ACK, session handshake complete");

    char ip_addr_own[100];
    char ip_addr_dst[100];

    snprintf(ip_addr_own, sizeof(ip_addr_own), "ip addr add %s/24 dev tun0", session_config_.client_address.c_str());
    snprintf(ip_addr_dst, sizeof(ip_addr_dst), "ip route add %s dev tun0", session_config_.server_address.c_str());

    system(ip_addr_own);
    system("ip link set tun0 up");

    spdlog::info("TUN device configured with IP: {}", session_config_.client_address);
    spdlog::info("Route to server address {} added", session_config_.server_address);

    session_configured_ = true;
};

void TunnelClient::connect_command_channel() {
    auto& command_channel = mqtt_channels_.get_command_client();
    try {
        spdlog::info("Connecting to command channel...");
        command_channel.connect();
        spdlog::info("Connected! Now subscribing to topic: {}", command_channel_name_ + "_TX");
        command_channel.subscribe(command_channel_name_ + "_TX", 1);
        spdlog::info("Subscribed successfully");
        spdlog::info("Command channel connected");
    } catch (const mqtt::exception& exc) {
        spdlog::error("Error connecting to command channel: {}", exc.what());
        throw;
    }
}

void TunnelClient::connect_data_channel() {
    auto& data_channel = mqtt_channels_.get_data_client();
    try {
        data_channel.connect()->wait();
        spdlog::info("Data channel connected");
    } catch (const mqtt::exception& exc) {
        spdlog::error("Error connecting to data channel: {}", exc.what());
        throw;
    }
}

void TunnelClient::async_tun_read() {
    tun_read_thread_ = std::thread([this]() {
        char buffer[1500]; // typical MTU size
        while (tunnel_active_) {
            ssize_t bytes_read = read(tun_device_.fd(), buffer, sizeof(buffer));
            if (bytes_read < 0) {
                spdlog::error("Error reading from TUN device: {}", strerror(errno));
                continue;
            }

            mqtt::message_ptr pubmsg = mqtt::make_message(session_config_.topic_outbound, std::string(buffer, bytes_read));
            pubmsg->set_qos(1);
            mqtt_channels_.get_data_client().publish(pubmsg)->wait_for(std::chrono::seconds(10));

            spdlog::debug("Read {} bytes from TUN and published to topic {}", bytes_read, session_config_.topic_outbound);
        }
    });
}

