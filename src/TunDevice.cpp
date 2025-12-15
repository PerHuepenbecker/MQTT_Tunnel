#include "TunDevice.hpp"

// C based TUN device creation and configuration function wrapper

TunDevice::TunDevice(){
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        std::cerr << "open(/dev/net/tun) failed: " << strerror(errno) << std::endl;
        tun_fd_ = fd;
        return;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        std::cerr << "ioctl(TUNSETIFF) failed: " << strerror(errno) << std::endl;
        close(fd);
        tun_fd_ = err;
        return;
    }

    tun_fd_ = fd;
}

// Proper cleanup via RAII

TunDevice::~TunDevice(){
    if(tun_fd_ >= 0){
        close(tun_fd_);
    }
}