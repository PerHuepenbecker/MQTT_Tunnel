#pragma once

#include <mqtt/async_client.h>
#include <mqtt/client.h>
#include <system_error>
#include "TunCallback.hpp"


class MQTTChannels {
    public:
        MQTTChannels(const std::string& broker_address, const std::string& client_base_id);
        mqtt::client& get_command_client();
        mqtt::async_client& get_data_client();


    private:
        std::unique_ptr<mqtt::client> mqtt_command_client_;         // Two MQTT clients here for clear channel separation
        std::unique_ptr<mqtt::async_client> mqtt_data_client_;      // One for command/control, one for tunneled data transfer;
        std::shared_ptr<TunCallback> tun_callback_;                 // Callback for data channel to write incoming messages to TUN device
};
