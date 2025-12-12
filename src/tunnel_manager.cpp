#include "tunnel_manager.h"

TunnelManager::TunnelManager(std::string command_channel_name, std::string client_base_id,std::string broker_address){
    command_channel_name_ = command_channel_name;

    mqtt_command_client_ = std::make_unique<MQTTClientWrapper>(broker_address, client_base_id + "_cmd");
    
}

SessionConfig TunnelManager::setup_session(){
    SessionConfig config;
    // Client hello message to the server with optional authentication or identification data


    // Expecting Server hello message with assigned cliet ID, client IP-address for TUN-Device and topic set for the data channel

    // Client ack message to finalize session setup

    // Optional server ack message to confirm correct session setup

    return config;
}