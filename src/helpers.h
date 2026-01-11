#pragma once

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <arpa/inet.h>


inline std::string get_sha256_string(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, EVP_sha256(), nullptr);
    EVP_DigestUpdate(context, data.data(), data.size());
    EVP_DigestFinal_ex(context, hash, nullptr);
    EVP_MD_CTX_free(context);

    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
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

inline bool is_json_payload(const std::string& payload) {
    size_t first = payload.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return false;
    }
    if (payload[first] != '{') {
        return false;
    }
    return true;
}