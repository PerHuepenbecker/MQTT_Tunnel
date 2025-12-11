#include <string>
#include <vector>
#include <memory>

#include <MQTTClient.h> // paho mqtt client

class MQTTClientWrapper {
public:
    using MessageCallback = void(*)(const std::string& topic, const std::vector<unsigned char>& payload);

    MQTTClientWrapper(const std::string& server_uri, const std::string& client_id);

    void connect();
    void disconnect();
    void subscribe(const std::string& topic);
    void publish(const std::string& topic, const std::vector<unsigned char>& payload);
    void set_message_callback(MessageCallback callback);

    void conopts_set_keepalive(int keepalive);
    void conopts_set_cleansession(bool cleansession);
    void conopts_set_username_password(const std::string& username, const std::string& password); // For future use with authentcicated tunnels
    

private:
    MQTTClient mqtt_client_; // Paho MQTT client instance
    MQTTClient_connectOptions conn_opts_;
    
    // Alle relevant für separate Klasse für das Management des Tunnels
    
    //std::string own_address_;
    //std::string dst_address_;

    //std::unique_ptr<class IPPool> ip_pool_; // internal IP pool for managing IP addresses. Only used in GATEWAY role.

    //Role role_;

    //std::pair<std::string, std::string> command_topics_; // first is outbound, second is inbound
    //std::vector<std::pair<std::string, std::string>> data_topics_; // vector of (outbound, inbound) topic pairs. Will be size 1 for a CONNECTOR, size N for a GATEWAY

};