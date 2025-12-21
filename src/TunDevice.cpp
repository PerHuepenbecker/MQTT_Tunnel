#include "TunDevice.hpp"

// C based TUN device creation and configuration function wrapper

TunDevice::TunDevice(const std::string& device_name) {
    struct ifreq ifr;
    int fd, err;

    spdlog::debug("Creating TUN device: {}", device_name);

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        spdlog::error("open(/dev/net/tun) failed: {}", strerror(errno));
        tun_fd_ = fd;
        return;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (device_name.size() > 0) {
        strncpy(ifr.ifr_name, device_name.c_str(), IFNAMSIZ);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        spdlog::error("ioctl(TUNSETIFF) failed: {}", strerror(errno));
        close(fd);
        tun_fd_ = err;
        return;
    }

    spdlog::debug("TUN device {} created", ifr.ifr_name);

    tun_fd_ = fd;
}

// Proper cleanup via RAII

TunDevice::~TunDevice(){
    if(tun_fd_ >= 0){
        close(tun_fd_);
    }
    spdlog::debug("TUN device closed");
}