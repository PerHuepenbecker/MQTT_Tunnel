#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <spdlog/spdlog.h>

// Class for managing a pool of IP addresses.
// Currently supports only /24 nets

class IPPool {

    public:
        IPPool(const std::string& base_ip, int pool_size);
        std::string allocate_ip();
        size_t release_ip(const std::string& ip);
        std::string get_base_ip() const { return base_ip_; }
        size_t count_allocated();

    private:
        std::string net_ip_; // e.g. 192.168.0.0 
        std::string base_ip_; // e.g. 192.168.0.
        int pool_size_;       // e.g. 253 max for /24 because .0 is network, .255 is broadcast and .1 will be gateway
        std::vector<bool> ip_usage_;
        std::mutex mutex_;
};