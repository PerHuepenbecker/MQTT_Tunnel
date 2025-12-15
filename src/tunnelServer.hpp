#pragma once

#include <mqtt/async_client.h>
#include <mqtt/client.h>
#include <system_error>
#include <unistd.h>
#include "helpers.h"
#include "MQTTChannels.hpp"
#include "TunDevice.hpp"
#include "ip_pool.h"
#include "types.hpp"
#include <vector>
#include <string>
#include "SessionMap.hpp"
#include <netinet/ip.h>

class TunnelServer;

class TunnelServerBuilder {
            public:
                TunnelServerBuilder& set_broker_address(const std::string& broker_address) {
                    broker_address_ = broker_address;
                    return *this;
                }
                TunnelServerBuilder& set_command_channel_name(const std::string& command_channel_name) {
                    command_channel_name_ = command_channel_name;
                    return *this;
                }

                TunnelServerBuilder& set_tun_device_name(const std::string& tun_device_name) {
                    tun_device_name_ = tun_device_name;
                    return *this;
                }

                TunnelServerBuilder& set_ip_pool_base(const std::string& base_ip) {
                    ip_pool_base_ = base_ip;
                    return *this;
                }

                TunnelServerBuilder& set_ip_pool_size(unsigned int pool_size) {
                    ip_pool_size_ = pool_size;
                    return *this;
                }

                std::unique_ptr<TunnelServer> build() {
                    if (broker_address_.empty() || command_channel_name_.empty()) {
                        throw std::runtime_error("Broker address and command channel name must be set");
                    }

                    return std::make_unique<TunnelServer>(broker_address_, command_channel_name_, tun_device_name_);
                }

            private:
                std::string broker_address_;
                std::string command_channel_name_;
                std::string tun_device_name_ = "tun0"; // default TUN device name
                std::string ip_pool_base_ = "10.0.0.0"; // default IP pool base
                unsigned int ip_pool_size_ = 253; // default IP pool size
        };


class TunnelServer {
    public:
        TunnelServer(const std::string& broker_address,
                        const std::string& command_channel_name, 
                        const std::string& tun_device_name = "tun0",
                        const std::string& ip_pool_base = "10.0.0.0",
                        unsigned int ip_pool_size = 253);

        void start_server();
        void stop_server();

    private:

        using SessionState = enum {
            HANDSHAKE_CLIENT_HELLO,
            HANDSHAKE_SERVER_HELLO,
            HANDSHAKE_CLIENT_ACK,
            HANDSHAKE_SERVER_ACK,
            ACTIVE,
            UNKNOWN
        };

        using ClientID = std::string;
        using TunnelAddress = std::string;

        MQTTChannels mqtt_channels_;         // Two MQTT clients here for clear channel separation
        TunDevice tun_device_;               // TUN device wrapper
        IPPool ip_pool_;                     // address pool for client IPs
        std::string command_channel_name_;   // command channel name - preshared information
        SessionMap active_clients_;          // Threadsafe map of currently active clients with session info
        
        bool server_running_ = false;
        bool command_channel_connected_ = false;
        bool data_channel_connected_ = false;

        std::atomic<bool> tunnel_active_{false}; 

        void connect_command_channel();
        void connect_data_channel();
        void async_tun_read();

        void handle_client_handshake(mqtt::const_message_ptr msg); // Currently blockin handshake handler
};