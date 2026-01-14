#pragma once

#include <string>
#include <iomanip>
#include <sstream>
#include <arpa/inet.h>
#include <sodium.h>

inline std::string get_sha256_string(const std::string& input) {
    
    unsigned char hash[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(hash, reinterpret_cast<const unsigned char*>(input.data()), input.size());

    char hex_hash[crypto_hash_sha256_BYTES * 2 + 1];
    sodium_bin2hex(hex_hash, sizeof(hex_hash), hash, sizeof(hash));

    return std::string(hex_hash);
}

inline std::string ip_to_string(uint32_t ip_addr) {
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_addr, ip_str, INET_ADDRSTRLEN);
    return std::string(ip_str);
}



inline std::string extract_client_id_from_topic(const std::string& topic) {
    
    size_t first_slash = topic.find('/');
    size_t second_slash = topic.find('/', first_slash + 1);
    
    if (first_slash != std::string::npos && second_slash != std::string::npos) {
        return topic.substr(first_slash + 1, second_slash - first_slash - 1);
    }
    return ""; 
}
