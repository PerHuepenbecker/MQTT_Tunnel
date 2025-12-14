#include <string>
#include <sstream>
#include <cereal/archives/json.hpp>

template <typename MessageType>
class MessageSerializer{
    public:
    static std::string to_string(const MessageType &message) {
        std::ostringstream oss;
        {
            cereal::JSONOutputArchive(oss);
            archive(message)
        }

        return oss.str();
    }

    static MessageType from_string(const std::string& str){
        MessageType message;

        std::istringstream iss;

        {
            cereal::JSONInputArchive(iss);
            archive(str);
        }

        return msg;
    }
};

// Client Hello for initiating a session handshake

struct ClientHello {
    std::string message_identifier;
    std::string client_base_id;
    bool authentication;
    std::string auth_data; // optional authentication or identification data
    std::string handshake_identifier; // 256 bit random number
    std::string data_hash;

    // using cereal for robust serialization

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(client_base_id),
                CEREAL_NVP(authentication),
                CEREAL_NVP(auth_data),
                CEREAL_NVP(handshake_identifier),
                CEREAL_NVP(data_hash));
    } 

    std::string to_string() const {
        return MessageSerializer<ClientHello>::to_string(*this);
    }

    static ClientHello from_string(const std::string& str) {
        return MessageSerializer<ClientHello>::from_string(str);
    }
};

// Server Hello for client configuration

struct ServerHello {
    std::string message_identifier;
    std::string handshake_identifier;
    std::string assigned_client_id_;
    std::string assigned_client_ip;
    std::string assigned_inbound_topic;
    std::string assigned_outbound_topic;
    std::string data_hash;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(handshake_identifier),
                CEREAL_NVP(assigned_client_id_),
                CEREAL_NVP(assigned_client_ip),
                CEREAL_NVP(assigned_inbound_topic),
                CEREAL_NVP(assigned_outbound_topic),
                CEREAL_NVP(data_hash));
    }   

    std::string to_string() const {
        return MessageSerializer<ServerHello>::to_string(*this);   
    }

    static ServerHello from_string(const std::string& str) {
        return MessageSerializer<ServerHello>::from_string(str);
    }
};

struct ClientACK {
    std::string message_identifier;
    std::string handshake_identifier;
    std::string server_hash;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(handshake_identifier),
                CEREAL_NVP(server_hash));
    }   

    std::string to_string() const {
        return MessageSerializer<ClientACK>::to_string(*this);
    }

    static ClientACK from_string(const std::string& str) {
        return MessageSerializer<ClientACK>::from_string(str);
    }
};

struct ServerACK {
    std::string message_identifier;
    std::string handshake_identifier;
    std::string client_hash;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(handshake_identifier),
                CEREAL_NVP(client_hash));
    }   

    std::string to_string() const {
        return MessageSerializer<ServerACK>::to_string(*this);
    }

    static ServerACK from_string(const std::string& str) {
        return MessageSerializer<ServerACK>::from_string(str);
    }
};

struct HandshakeRST {
    std::string message_identifier;
    std::string handshake_identifier;
    std::string error_message;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(handshake_identifier),
                CEREAL_NVP(error_message));
    }   

    std::string to_string() const {
        return MessageSerializer<HandshakeRST>::to_string(*this);
    }

    static HandshakeRST from_string(const std::string& str) {
        return MessageSerializer<HandshakeRST>::from_string(str);
    }
};