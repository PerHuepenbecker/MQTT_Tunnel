#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

// Function to create a TUN device. 
// dev: Name of the TUN device to create, e.g. "TUN0"

int tun_create(const char *dev) {
    struct ifreq ifr;
    int fd, err;

    // Open the TUN device file

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("open(/dev/net/tun) failed");
        return fd;
    }


    // clear the struct and set the flags
    // IFF_TUN   - TUN device, so strictly IP traffic
    // IFF_NO_PI -  Do not provide packet information

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    // Configure the TUN device

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        perror("ioctl(TUNSETIFF) failed");
        close(fd);
        return err;
    }

    return fd;
}



