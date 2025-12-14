#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

// Function to create a TUN device. 

int tun_create(const char *dev) {
    struct ifreq ifr;
    int fd, err;

    // Open the TUN device file

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("open(/dev/net/tun) failed");
        return fd;
    }

    // clear the struct and set the flags

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

int tun_fd_static = -1;


int main(int argc, char** argv) {

    // definition of command line argument variables
    
    int opt;
    char* broker_address = NULL;
    char* client_id = NULL;
    char* outbound_topic = NULL;
    char* inbound_topic = NULL;
    char* own_address = NULL;
    char* dst_address = NULL;

    const char *optstring = "s:d:a:c:o:i:";

    // parse the command line arguments

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'a':
                broker_address = optarg;
                break;
            case 'c':
                client_id = optarg;
                break;
            case 'o':
                outbound_topic = optarg;
                break;
            case 'i':
                inbound_topic = optarg;
                break;
            case 's':
                own_address = optarg;
                break;
            case 'd':
                dst_address = optarg;   
                break;
            default:
                std::cerr << "Usage: " << argv[0] << "-s <own_ip_adress> -d <dst_ip_address> -a <broker_address> -c <client_id> -o <outbound_topic> -i <inbound_topic>" << std::endl;
                return 1;
        }
    }
    

    const char *tun_name = "tun0";
    int tun_fd = tun_create(tun_name);
    if (tun_fd < 0) {
        std::cerr << "Error creating TUN device" << std::endl;
        return 1;
    }
    tun_fd_static = tun_fd;

    // Can be cleaned up to use netlink
 
    // declare command strings for setting up TUN device
    char ip_addr_own[100];
    char ip_addr_dst[100];

    snprintf(ip_addr_own, sizeof(ip_addr_own), "ip addr add %s/24 dev tun0", own_address);
    snprintf(ip_addr_dst, sizeof(ip_addr_dst), "ip route add %s dev tun0", dst_address);

    // execute commands to set up TUN device

    system(ip_addr_own);
    system("ip link set tun0 up");
    system(ip_addr_dst);    


    
    return 0;
}