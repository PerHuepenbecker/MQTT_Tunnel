#include "MQTTChannels.hpp"

MQTTChannels::MQTTChannels(const std::string& broker_address, const std::string& client_base_id) {
    mqtt_command_client_ = std::make_unique<mqtt::client>(broker_address, client_base_id + "_cmd");
    mqtt_data_client_ = std::make_unique<mqtt::async_client>(broker_address, client_base_id + "_data");
}

mqtt::client& MQTTChannels::get_command_client() {
    return *mqtt_command_client_;
}

mqtt::async_client& MQTTChannels::get_data_client() {
    return *mqtt_data_client_;
}

void MQTTChannels::set_tun_callback(int tun_fd, bool encryption_enabled, CryptoManager& crypto_manager) {
    tun_callback_ = std::make_shared<TunCallback>(tun_fd, crypto_manager, encryption_enabled);
    mqtt_data_client_->set_callback(*tun_callback_);
}

