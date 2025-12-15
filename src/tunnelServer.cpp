#include "tunnelServer.hpp"


TunnelServer::TunnelServer(const std::string& broker_address, const std::string& command_channel_name, const std::string& tun_device_name, const std::string& ip_pool_base, unsigned int ip_pool_size)
    : mqtt_channels_(broker_address, command_channel_name),
      tun_device_(tun_device_name),
      ip_pool_(ip_pool_base, ip_pool_size),
      command_channel_name_(command_channel_name) {};

void TunnelServer::start_server() {
    connect_command_channel();
    connect_data_channel();
    server_running_ = true;

    while (server_running_) {
        auto& command_channel = mqtt_channels_.get_command_client();
        mqtt::const_message_ptr msg = command_channel.consume_message();

        if (msg) {
            handle_client_handshake(msg);
        }
    }
}


void TunnelServer::stop_server() {}

void TunnelServer::handle_client_handshake(mqtt::const_message_ptr msg) {
    // Placeholder for handshake handling logic
}

void TunnelServer::connect_command_channel() {
    auto& command_channel = mqtt_channels_.get_command_client();
    try {
        command_channel.connect();
        command_channel.subscribe((command_channel_name_+"_RX"), 1);
        command_channel_connected_ = true;

    } catch (const mqtt::exception& exc) {
        std::cerr << "Error connecting to command channel: " << exc.what() << std::endl;
        throw;
    }

    std::cout << "Command channel connected" << std::endl;
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
            throw std::system_error(errno, std::generic_category(), "Failed to read from TUN device");
        }

        struct iphdr* ip_header = reinterpret_cast<struct iphdr*>(buffer.data());
        std::string dest_ip = ip_to_string(ip_header->daddr);

        // Lookup session for destination IP
        SessionConfig session;
        if(!active_clients_.get_session(dest_ip, session)) {
            spdlog::warn("No active session for destination IP: {}", dest_ip);
            continue; // No active session for this IP
        }
    
        mqtt::message_ptr pubmsg = mqtt::make_message(session.topic_outbound, std::string(buffer.data(), read_bytes));
        pubmsg->set_qos(1);
        mqtt_channels_.get_data_client().publish(pubmsg);

    }
};