#pragma once

#include <sodium.h>
#include <string>
#include <sstream>
#include <cereal/archives/json.hpp>
#include <cereal/types/array.hpp>

template <typename MessageType>
class MessageSerializer{
    public:
    static std::string to_string(const MessageType &message) {
        std::ostringstream oss;
        {
            cereal::JSONOutputArchive archive(oss);
            archive(message);
        }

        return oss.str();
    }

    static MessageType from_string(const std::string& str){
        MessageType message;

        std::istringstream iss(str);

        {
            cereal::JSONInputArchive archive(iss);
            archive(message);
        }

        return message;
    }
};

// Client Hello for initiating a session handshake

struct ClientHello {
    std::string message_identifier;
    std::string client_base_id;
    bool authentication;
    std::string auth_data; // optional authentication or identification data
    std::string handshake_identifier; // 256 bit random number

    // using cereal for robust serialization

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(client_base_id),
                CEREAL_NVP(authentication),
                CEREAL_NVP(auth_data),
                CEREAL_NVP(handshake_identifier));
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
    std::string server_address;
    std::string assigned_inbound_topic;
    std::string assigned_outbound_topic;
    std::string session_id;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(handshake_identifier),
                CEREAL_NVP(assigned_client_id_),
                CEREAL_NVP(assigned_client_ip),
                CEREAL_NVP(server_address),
                CEREAL_NVP(assigned_inbound_topic),
                CEREAL_NVP(assigned_outbound_topic),
                CEREAL_NVP(session_id));
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

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(handshake_identifier));
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

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(handshake_identifier));
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

struct SessionTermination {
    std::string message_identifier;
    std::string client_id;
    std::string session_id; // Possible additional session identifier for proper identification
    std::string reason;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(client_id),
                CEREAL_NVP(session_id),
                CEREAL_NVP(reason));
    }   

    std::string to_string() const {
        return MessageSerializer<SessionTermination>::to_string(*this);
    }

    static SessionTermination from_string(const std::string& str) {
        return MessageSerializer<SessionTermination>::from_string(str);
    }
};

struct ClientHelloCrypto {
    std::string message_identifier = "CLIENT_HELLO_CRYPTO";

    std::string client_base_id;
    uint8_t client_ephemeral_public_key[crypto_kx_PUBLICKEYBYTES];
    
    // uint8_t signature_message[crypto_sign_BYTES];  Possible for future mTLS like authentication

    template <class Archive>
    void serialize(Archive& archive) {
    archive(cereal::make_nvp("message_identifier", message_identifier),
            cereal::make_nvp("client_base_id", client_base_id),
            cereal::make_nvp("client_ephemeral_public_key", client_ephemeral_public_key));
}

    std::string to_string() const {
        return MessageSerializer<ClientHelloCrypto>::to_string(*this);
    }

    static ClientHelloCrypto from_string(const std::string& str) {
        return MessageSerializer<ClientHelloCrypto>::from_string(str);
    }
};

struct ServerHelloCrypto {
    std::string message_identifier = "SERVER_HELLO_CRYPTO";

    std::string unique_identifier; // Session specific unique identifier to find correct decryption keys
    uint8_t server_ephemeral_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t signature_message[crypto_sign_BYTES];

    template <class Archive>
    void serialize(Archive& archive) {
    archive(cereal::make_nvp("message_identifier", message_identifier),
            cereal::make_nvp("unique_identifier", unique_identifier),
            cereal::make_nvp("server_ephemeral_public_key", server_ephemeral_public_key),
            cereal::make_nvp("signature_message", signature_message));
}

    std::string to_string() const {
        return MessageSerializer<ServerHelloCrypto>::to_string(*this);
    }

    static ServerHelloCrypto from_string(const std::string& str) {
        return MessageSerializer<ServerHelloCrypto>::from_string(str);
    }
};