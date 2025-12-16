#include <gtest/gtest.h>
#include "../src/helpers.h"

TEST(HelpersTest, SHA256Hash) {
    std::string data = "Hello, World!";
    std::string expected_hash = "dffd6021bb2bd5b0af676290809ec3a53191dd81c7f70a4b28688a362182986f";
    std::string computed_hash = get_sha256_string(data);
    EXPECT_EQ(computed_hash, expected_hash);
}

TEST(HelpersTest, ConvertIPtoString) {
    uint32_t ip = 0xC0A80101;
    auto ip_network_order = htonl(ip);
    std::string expected_ip_str = "192.168.1.1";

    std::string ip_str = ip_to_string(ip_network_order);
    EXPECT_EQ(ip_str, expected_ip_str);
}

