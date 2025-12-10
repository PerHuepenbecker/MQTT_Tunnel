#include "ip_pool.h"

IPPool::IPPool(const std::string& net_ip, int pool_size){
        base_ip_ = net_ip_.substr(0, net_ip_.rfind('.') + 1);
        if(net_ip_.empty() || pool_size_ <=0 || pool_size_ > 253){
            throw std::invalid_argument("Invalid network IP or pool size");
        }

        if(net_ip.back() != '0'){
            throw std::invalid_argument("Currently only /24 networks are supported with base IP ending in .0");
        }

        net_ip_ = net_ip;
        pool_size_ = pool_size;
        ip_usage_.resize(pool_size_, false);
}


std::string IPPool::allocate_ip(){
    std::lock_guard<std::mutex> lock(mutex_);
    for(size_t i = 1; i <ip_usage_.size(); ++i){
        if(!ip_usage_[i]){
            ip_usage_[i] = true;
            
            return base_ip_ + std::to_string(i + 1);
        }
    }
    return "";
}

size_t IPPool::release_ip(const std::string& ip){
    std::lock_guard<std::mutex> lock(mutex_);
    size_t last_dot = ip.rfind('.');
    if(last_dot == std::string::npos) return 1;

    int last_octet = std::stoi(ip.substr(last_dot + 1));

    if(last_octet < 2 || last_octet >= pool_size_ + 2) return 1;
    
    ip_usage_[last_octet - 1] = false;
    return 0;
}

size_t IPPool::count_allocated() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for(const auto& address: ip_usage_){
        if(address) ++count;
    }
    return count;
}

