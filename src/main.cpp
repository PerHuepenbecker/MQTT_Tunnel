#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <MQTTClient.h>

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


int mqtt_incoming_message_callback(void* context, char* topic_name, int topic_len, MQTTClient_message* message) {
    
    if (tun_fd_static < 0) {
        std::cerr << "TUN device not initialized" << std::endl;
        return;
    }

    unsigned char* data = static_cast<unsigned char*>(message->payload);
    int len = message->payloadlen;

    int bytes_written = write(tun_fd_static, data, len);
    if (bytes_written < 0) {
        std::cerr << "Error writing to TUN device: " << strerror(errno) << std::endl;
    } 

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic_name);

    return 1;
}


int main(int argc, char** argv) {

    // definition of command line argument variables
    
    int opt;
    char* broker_address = NULL;
    char* client_id = NULL;
    char* outbound_topic = NULL;
    char* inbound_topic = NULL;

    const char *optstring = "a:c:o:i:";

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
            default:
                std::cerr << "Usage: " << argv[0] << " -a <broker_address> -c <client_id> -o <outbound_topic> -i <inbound_topic>" << std::endl;
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
 
    system("ip addr add 10.8.0.1/24 dev tun0");
    system("ip link set tun0 up");
    system("ip route add 10.8.0.2 dev tun0");

    // MQTT Client setup

    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_create(&client, broker_address, client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, NULL, mqtt_incoming_message_callback, NULL);

    if(MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        return 1;
    }

    MQTTClient_subscribe(client, inbound_topic, 1);

    unsigned char buffer[2000];

    while(1){

        int read_bytes = read(tun_fd, buffer, sizeof(buffer));
        if (read_bytes < 0) {
            std::cerr << "Error reading from TUN device: " << strerror(errno) << std::endl;
            break;
        }

        MQTTClient_message pubmsg = MQTTClient_message_initializer;
        pubmsg.payload = buffer;
        pubmsg.payloadlen = read_bytes;
        pubmsg.qos = 0;
        pubmsg.retained = 0;

        MQTTClient_deliveryToken token;
        int return_code = MQTTClient_publishMessage(client, outbound_topic, &pubmsg, &token);
        
        if (return_code != MQTTCLIENT_SUCCESS) {
            std::cerr << "Failed to publish message, return code " << return_code << std::endl;
        }
    }


    close(tun_fd);
    return 0;
}