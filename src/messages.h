#include <string>
#include <sstream>
#include <cereal/archives/json.hpp>

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
        std::ostringstream oss;

        {
            cereal::JSONOutputArchive archive(oss);

            archive(*this);
        }

        return oss.str();
    }

    static ClientHello from_string(const std::string& str) {
        ClientHello ch;
        std::istringstream iss(str);

        {
            cereal::JSONInputArchive archive(iss);

            archive(ch);
        }

        return ch;
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
        std::ostringstream oss;

        {
            cereal::JSONOutputArchive archive(oss);

            archive(*this);
        }

        return oss.str();
    }

    static ServerHello from_string(const std::string& str) {
        ServerHello sh;
        std::istringstream iss(str);

        {
            cereal::JSONInputArchive archive(iss);

            archive(sh);
        }

        return sh;
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
        std::ostringstream oss;

        {
            cereal::JSONOutputArchive archive(oss);

            archive(*this);
        }

        return oss.str();
    }

    static ClientACK from_string(const std::string& str) {
        ClientACK ca;
        std::istringstream iss(str);

        {
            cereal::JSONInputArchive archive(iss);

            archive(ca);
        }

        return ca;
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
        std::ostringstream oss;

        {
            cereal::JSONOutputArchive archive(oss);

            archive(*this);
        }

        return oss.str();
    }

    static ServerACK from_string(const std::string& str) {
        ServerACK sa;
        std::istringstream iss(str);

        {
            cereal::JSONInputArchive archive(iss);

            archive(sa);
        }

        return sa;
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
        std::ostringstream oss;

        {
            cereal::JSONOutputArchive archive(oss);

            archive(*this);
        }

        return oss.str();
    }

    static HandshakeRST from_string(const std::string& str) {
        HandshakeRST hrst;
        std::istringstream iss(str);

        {
            cereal::JSONInputArchive archive(iss);

            archive(hrst);
        }

        return hrst;
    }
};