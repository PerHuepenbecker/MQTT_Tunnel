#include "MQTTChannels.hpp"

MQTTChannels::MQTTChannels(const std::string& broker_address, const std::string& client_base_id) {
    spdlog::info("Initializing MQTT Channels");
    mqtt_command_client_ = std::make_unique<mqtt::client>(broker_address, client_base_id);
    mqtt_data_client_ = std::make_unique<mqtt::async_client>(broker_address, client_base_id);
}

mqtt::client& MQTTChannels::get_command_client() {
    return *mqtt_command_client_;
}

mqtt::async_client& MQTTChannels::get_data_client() {
    return *mqtt_data_client_;
}

