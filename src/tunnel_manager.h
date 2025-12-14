// Channel config struct for storing MQTT channel information
#include <string>
#include <memory>
#include <random>
#include <sstream>
#include <iomanip>
#include "mqtt_client_wrapper.h"
#include "messages.h"

#include <mqtt/client.h>
#include <mqtt/async_client.h>

struct SessionConfig {
    std::string client_id;
    std::string client_address;
    std::string topic_inbound;
    std::string topic_outbound;
};



class TunnelManager {
    public:
        TunnelManager(std::string commad_channel_name, std::string client_base_id, std::string broker_address);

    private:
        std::string command_channel_name_; // Variable to indentify the command channel - only information to be preshared
        std::unique_ptr<mqtt::client> mqtt_command_client_; // Two MQTT clients here for clear channel separation
        std::unique_ptr<mqtt::async_client> mqtt_data_client_;    // One for command/control, one for tunneled data transfer;

        SessionConfig session_config_;
        bool session_established_ = false;

        SessionConfig setup_session(); // establishes session via command channel using the preshared channel name variable

        class CommandClientCallback : public virtual mqtt::callback {

        };
};