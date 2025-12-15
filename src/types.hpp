#pragma once
#include <string>


struct SessionConfig {
    std::string client_id;
    std::string client_address;
    std::string topic_inbound;
    std::string topic_outbound;
};
