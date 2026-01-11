#pragma once

#include "TunDevice.hpp"
#include "MQTTChannels.hpp"
#include "types.hpp"
#include "messages.h"
#include "CryptoManager.hpp"

#include <atomic>
#include <thread>
#include <memory>
#include <random>


class TunnelClient;

class TunnelClientBuilder {
    public:
        TunnelClientBuilder& set_broker_address(const std::string& broker_address) {
            broker_address_ = broker_address;
            return *this;
        }

        TunnelClientBuilder& set_command_channel_name(const std::string& command_channel_name) {
            command_channel_name_ = command_channel_name;
            return *this;
        }

        TunnelClientBuilder& set_client_base_id(const std::string& client_base_id) {
            client_base_id_ = client_base_id;
            return *this;
        }

        TunnelClientBuilder& set_tun_device_name(const std::string& tun_device_name) {
            tun_device_name_ = tun_device_name;
            return *this;
        }

        TunnelClientBuilder& set_enable_encryption(bool enable_encryption) {
            enable_encryption_ = enable_encryption;
            return *this;
        }
        TunnelClientBuilder& set_ignore_server_authentication(bool ignore_server_authentication) {
            ignore_server_authentication_ = ignore_server_authentication;
            return *this;
        }

        std::unique_ptr<TunnelClient> build() {
            if (broker_address_.empty() || command_channel_name_.empty() || client_base_id_.empty()) {
                spdlog::error("Broker address, command channel name, and client base ID must be set");
                throw std::runtime_error("Broker address, command channel name, and client base ID must be set");
            }

            return std::make_unique<TunnelClient>(broker_address_, command_channel_name_, client_base_id_, tun_device_name_, enable_encryption_, ignore_server_authentication_);
        }

    private:
        std::string broker_address_;
        std::string command_channel_name_;
        std::string client_base_id_;
        std::string tun_device_name_ = "tun0"; // default TUN device name
        bool enable_encryption_ = true;
        bool ignore_server_authentication_ = false;
};

class TunnelClient {
    public:
        TunnelClient(const std::string& broker_address,
                     const std::string& command_channel_name,
                     const std::string& client_base_id,
                     const std::string& tun_device_name,
                     bool enable_encryption,
                     bool ignore_server_authentication);
                     
        void start_tunnel();
        void stop_tunnel();

    private:
        MQTTChannels mqtt_channels_;
        TunDevice tun_device_;
        std::string command_channel_name_;
        std::string client_base_id_;
        SessionConfig session_config_;
        bool session_configured_ = false;
        bool encryption_enabled = false;
        std::atomic<bool> tunnel_active_{false};
        std::thread tun_read_thread_;
        CryptoManager crypto_manager_;
        bool ignore_server_authentication_ = false;

        void setup_session();
        void connect_command_channel();
        void connect_data_channel();
        void async_tun_read();
};