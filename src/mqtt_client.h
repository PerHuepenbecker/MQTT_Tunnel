#include <string>
#include <vector>
#include <memory>

enum Role {
    CONNECTOR,
    GATEWAY
};

class MQTTClient {
public:
    MQTTClient(const std::string& server_uri, const std::string& client_id);

    void connect();
    void disconnect();
    void subscribe(const std::string& topic);
    void publish(const std::string& topic, const std::vector<unsigned char>& payload);
    
private:
    std::string broker_uri_;
    std::string client_id_;

    
    
    // Alle relevant für separate Klasse
    
    //std::string own_address_;
    //std::string dst_address_;

    //std::unique_ptr<class IPPool> ip_pool_; // internal IP pool for managing IP addresses. Only used in GATEWAY role.

    //Role role_;

    //std::pair<std::string, std::string> command_topics_; // first is outbound, second is inbound
    //std::vector<std::pair<std::string, std::string>> data_topics_; // vector of (outbound, inbound) topic pairs. Will be size 1 for a CONNECTOR, size N for a GATEWAY

};