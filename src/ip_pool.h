#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>



class IPPool {

    public:
        IPPool(const std::string& base_ip, int pool_size);
        std::string allocate_ip();
        void release_ip(const std::string& ip);

    private:
        std::string base_ip_;
        int pool_size_;
        std::vector<bool> ip_usage_;
        std::mutex mutex_;
};

IPPool::IPPool(const std::string& base_ip, int pool_size)
    : base_ip_(base_ip), pool_size_(pool_size), ip_usage_(pool_size, false) {}