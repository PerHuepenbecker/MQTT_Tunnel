// Channel config struct for storing MQTT channel information
#include <string>
#include <memory>
#include <random>
#include <sstream>
#include <iomanip>
#include "mqtt_client_wrapper.h"

struct SessionConfig {
    std::string client_id;
    std::string client_address;
    std::string topic_inbound;
    std::string topic_outbound;
};

// Client Hello for initiating a session handshake

struct ClientHello {
    std::string message_identifier;
    std::string client_base_id;
    bool authentication;
    std::string auth_data; // optional authentication or identification data
    std::string handshake_identifier; // 256 bit random number
    std::string data_hash;
};

// Server Hello for client configuration

struct ServerHello {
    std::string message_identifier;
    std::string handshake_identifier;
    std::string assigned_client_id_;
    std::string assigned_client_ip;
    std::string assigned_inbound_topic;
    std::string assigned_outbound_topic;
    std::string data_hash;
};

struct ClientACK {
    std::string message_identifier;
    std::string handshake_identifier;
    std::string server_hash;
};

struct ServerACK {
    std::string message_identifier;
    std::string handshake_identifier;
    std::string client_hash;
};

struct HandshakeRST {
    std::string message_identifier;
    std::string handshake_identifier;
    std::string error_message;
};

class TunnelManager {
    public:
        TunnelManager(std::string commad_channel_name, std::string client_base_id, std::string broker_address);

    private:
        std::string command_channel_name_; // Variable to indentify the command channel - only information to be preshared
        std::unique_ptr<MQTTClientWrapper> mqtt_command_client_; // Two MQTT clients here for clear channel separation
        std::unique_ptr<MQTTClientWrapper> mqtt_data_client_;    // One for command/control, one for tunneled data transfer;

        SessionConfig session_config_;

        SessionConfig setup_session(); // establishes session via command channel using the preshared channel name variable

};