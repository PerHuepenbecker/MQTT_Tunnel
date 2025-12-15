#pragma once

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

class TunDevice {
    public:
        TunDevice();
        ~TunDevice();

        int fd() const {
            return tun_fd_;
        }

    private:
        int tun_fd_;
};