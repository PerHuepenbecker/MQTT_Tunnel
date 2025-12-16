#include <gtest/gtest.h>
#include "../src/ip_pool.h"
#include <iostream>

TEST(IpPoolTest, ExpectedBaseIp) {
    IPPool ip_pool("192.168.0.0", 253);
    EXPECT_EQ(ip_pool.get_base_ip(), "192.168.0.");
}

TEST(IpPoolTest, AllocateIP) {
    IPPool ip_pool("192.168.0.0", 253);
    std::string allocated_ip = ip_pool.allocate_ip();
    std::cout << "Allocated IP: " << allocated_ip << std::endl;
    EXPECT_EQ(allocated_ip, "192.168.0.2");
}

TEST(IpPoolTest, CountAllocated) {
    IPPool ip_pool("192.168.0.0", 253);
    EXPECT_EQ(ip_pool.count_allocated(), 0);
    ip_pool.allocate_ip();
    EXPECT_EQ(ip_pool.count_allocated(), 1);
    ip_pool.allocate_ip();
    EXPECT_EQ(ip_pool.count_allocated(), 2);
    auto ret = ip_pool.release_ip("192.168.0.2");
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(ip_pool.count_allocated(), 1);
}

