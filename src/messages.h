#pragma once

#include <sodium.h>
#include <string>
#include <sstream>
#include <vector>
#include <cereal/archives/json.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>

enum MessageIdentifier {
    CLIENT_HELLO_CRYPTO = 1,
    SERVER_HELLO_CRYPTO = 2,
    CLIENT_HELLO = 3,
    SERVER_HELLO = 4,
    CLIENT_ACK = 5,
    SERVER_ACK = 6,
    HANDSHAKE_RST = 7, // unused currently
    SESSION_TERMINATION = 8
};


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
    MessageIdentifier message_identifier = CLIENT_HELLO;
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
    MessageIdentifier message_identifier = SERVER_HELLO;
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
    MessageIdentifier message_identifier = CLIENT_ACK;
    std::string client_id;
    std::string handshake_identifier;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(message_identifier),
                CEREAL_NVP(client_id),
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
    MessageIdentifier message_identifier = SERVER_ACK;
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
    MessageIdentifier message_identifier = HANDSHAKE_RST;
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
    MessageIdentifier message_identifier = SESSION_TERMINATION;
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
    MessageIdentifier message_identifier = CLIENT_HELLO_CRYPTO;

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
    MessageIdentifier message_identifier = SERVER_HELLO_CRYPTO;

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

struct MessageHeader {
    int type;

    template <class Archive>
    void serialize(Archive& archive) {
        
        archive(cereal::make_nvp("message_identifier", type)); 
    }

    static MessageHeader from_string(const std::string& str) {
        return MessageSerializer<MessageHeader>::from_string(str);
    }
};

struct EncryptedWrapper {
    std::string client_id; // Lookup client session for decryption
    std::vector <uint8_t> encrypted_payload; // actual encrypted message payload

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("client_id", client_id),
                cereal::make_nvp("encrypted_payload", encrypted_payload));
    }   

    std::string to_string() const {
        return MessageSerializer<EncryptedWrapper>::to_string(*this);
    }
    
    static EncryptedWrapper from_string(const std::string& str) {
        return MessageSerializer<EncryptedWrapper>::from_string(str);
    }
};