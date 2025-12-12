#include "tunnel_manager.h"
#include "helpers.h"

TunnelManager::TunnelManager(std::string command_channel_name, std::string client_base_id,std::string broker_address){
    command_channel_name_ = command_channel_name;

    mqtt_command_client_ = std::make_unique<MQTTClientWrapper>(broker_address, client_base_id + "_cmd");
}


SessionConfig TunnelManager::setup_session(){

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distr;

    std::stringstream ss;

    for(int i =0; i < 4 ; ++i) {
        ss << std::setw(16)  << std::setfill('0') << std::hex << distr(gen);
    }

    SessionConfig config;
    // Client hello message to the server with optional authentication or identification data
    ClientHello client_hello;
    client_hello.message_identifier = "CLIENT_HELLO";
    client_hello.client_base_id = session_config_.client_id;
    client_hello.authentication = false; // no authentication as a base case => not implemented yet
    client_hello.auth_data = "";
    client_hello.handshake_identifier = ss.str(); 

    std::stringstream concatenated_data;
    concatenated_data << client_hello.message_identifier << "[" << client_hello.client_base_id.length() << "]" << client_hello.client_base_id
                      << client_hello.authentication
                      << "[" << client_hello.auth_data.length() << "]" << client_hello.auth_data
                      << client_hello.handshake_identifier;

    client_hello.data_hash = get_sha256_string(concatenated_data.str());
    
    // Expecting Server hello message with assigned cliet ID, client IP-address for TUN-Device and topic set for the data channel

    // Client ack message to finalize session setup

    // Optional server ack message to confirm correct session setup

    return config;
}