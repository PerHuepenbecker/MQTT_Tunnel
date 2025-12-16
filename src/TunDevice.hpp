#pragma once

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <spdlog/spdlog.h>

class TunDevice {
    public:
        TunDevice(const std::string& device_name);
        ~TunDevice();

        int fd() const {
            return tun_fd_;
        }

    private:
        int tun_fd_;
};