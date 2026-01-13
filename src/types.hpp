#pragma once
#include <string>
#include "messages.h"
#include "TunnelServer.hpp"


struct SessionConfig {
    std::string client_id;
    std::string client_address;
    std::string server_address;
    std::string topic_inbound;
    std::string topic_outbound;
    std::string session_id;
    std::string handshake_identifier;
    TunnelServer::SessionState session_state;
};
