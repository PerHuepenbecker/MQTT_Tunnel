#pragma once

#include <mqtt/callback.h>
#include <mqtt/async_client.h>
#include <string>
#include <vector>


class CryptoManager;

class TunCallback : public virtual mqtt::callback {
public:
    TunCallback(int tun_fd, CryptoManager& crypto, bool encryption_enabled);

    void message_arrived(mqtt::const_message_ptr msg) override;
    void connection_lost(const std::string& cause) override;
    void delivery_complete(mqtt::delivery_token_ptr token) override;

private:
    int tun_fd_;
    CryptoManager& crypto_;
    bool encryption_enabled_;
};

class ServerCommandCallback : public virtual mqtt::callback {
public:
    ServerCommandCallback();
    void message_arrived(mqtt::const_message_ptr msg) override;
    void connection_lost(const std::string& cause) override;
    void delivery_complete(mqtt::delivery_token_ptr token) override;
};