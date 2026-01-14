#pragma once
#include <string>
#include "messages.h"

enum SessionState {
            HANDSHAKE_CLIENT_HELLO,
            HANDSHAKE_SERVER_HELLO,
            HANDSHAKE_CLIENT_ACK,
            HANDSHAKE_SERVER_ACK,
            ACTIVE,
            UNKNOWN
        };

enum TunnelMode {
    CONNECTION,
    GATEWAY
};

struct SessionConfig {
    std::string client_id;
    std::string client_address;
    std::string server_address;
    std::string topic_inbound;
    std::string topic_outbound;
    std::string session_id;
    std::string handshake_identifier;
    SessionState session_state;
    TunnelMode tunnel_mode;
};
