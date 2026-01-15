#include "TunCallback.hpp"
#include "CryptoManager.hpp"
#include "helpers.h"
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <iostream>


TunCallback::TunCallback(int tun_fd, CryptoManager& crypto, bool encryption_enabled) 
    : tun_fd_(tun_fd), crypto_(crypto), encryption_enabled_(encryption_enabled) {}

void TunCallback::message_arrived(mqtt::const_message_ptr msg) {
    std::string payload = msg->get_payload();

    if (encryption_enabled_) {
        try {
            std::string topic = msg->get_topic();

            
            std::string client_id = extract_client_id_from_topic(topic);

            if (client_id.empty()) {
                spdlog::warn("Could not extract client_id from topic: {}", topic);
                return;
            }

            std::vector<unsigned char> ciphertext(payload.begin(), payload.end());
            payload = crypto_.decrypt_data(ciphertext, client_id);
            
        } catch (const std::exception& e) {
            spdlog::error("Decryption failed in callback: {}", e.what());
            return; 
        }
    }

    spdlog::debug("Message arrived with topic: {}", msg->get_topic());

    ssize_t bytes_written = write(tun_fd_, payload.data(), payload.size());

    if (bytes_written < 0) {
        spdlog::error("Error writing to TUN device: {}", strerror(errno));
    } else {
        spdlog::debug("Wrote {} bytes to TUN device", bytes_written);
    }
}

void TunCallback::connection_lost(const std::string& cause) {
    spdlog::warn("TUN Connection lost: {}", cause);
}

void TunCallback::delivery_complete(mqtt::delivery_token_ptr token) {
    
}

ServerCommandCallback::ServerCommandCallback() {}

void ServerCommandCallback::message_arrived(mqtt::const_message_ptr msg) {
    spdlog::debug("Server command message arrived");
}

void ServerCommandCallback::connection_lost(const std::string& cause) {
    spdlog::warn("Server command connection lost: {}", cause);
}

void ServerCommandCallback::delivery_complete(mqtt::delivery_token_ptr token) {
}