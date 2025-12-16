#include <gtest/gtest.h>
#include "../src/TunDevice.hpp"
#include <sys/stat.h>

// Check because TUN device might not be available in containerized test environments

static bool IsTunAvailable() {
    struct stat st;
    return stat("/dev/net/tun", &st) == 0;
}

TEST(TunDeviceTest, CreateAndDestroy) {
    if (!IsTunAvailable()) {
        GTEST_SKIP() << "TUN device not available in this environment";
    }
    EXPECT_NO_THROW({
        TunDevice tun_device("tun0");
    });
}

TEST(TunDeviceTest, GetFileDescriptor) {
    if (!IsTunAvailable()) {
        GTEST_SKIP() << "TUN device not available in this environment";
    }
    TunDevice tun_device("tun0");
    int fd = tun_device.fd();
    EXPECT_GT(fd, 0); 
}



