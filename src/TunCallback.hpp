#include <mqtt/async_client.h>
#include <mqtt/client.h>
#include <system_error>
#include <unistd.h>
#include "helpers.h"

#pragma once

class TunCallback : public virtual mqtt::callback {
    public:
        TunCallback(int tun_fd) : tun_fd_(tun_fd) {}

        void message_arrived(mqtt::const_message_ptr msg) override {

            const std::string& payload = msg->get_payload();

            ssize_t bytes_written = write(tun_fd_, payload.data(), payload.size());

            if (bytes_written < 0) {
                throw std::system_error(errno, std::generic_category(), "Failed to write to TUN device");
            }
        }

        void connection_lost(const std::string& cause) override {
            std::cerr << "Connection lost: " << cause << std::endl;
        }

        void delivery_complete(mqtt::delivery_token_ptr token) override {
        }

    private:
        int tun_fd_;
};
