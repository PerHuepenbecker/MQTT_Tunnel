#include "tunnel_manager.h"

TunnelManager::TunnelManager(std::string command_channel_name, std::string client_base_id,std::string broker_address){
    command_channel_name_ = command_channel_name;

    mqtt_command_client_ = std::make_unique<MQTTClientWrapper>(broker_address, client_base_id + "_cmd");    
    

}