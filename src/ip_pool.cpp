#include "ip_pool.h"

std::string IPPool::allocate_ip(){
    std::lock_guard<std::mutex> lock(mutex_);
}