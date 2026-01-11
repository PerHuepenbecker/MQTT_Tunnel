
#include <sodium.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>

#pragma once

class CryptoManager {
    public:
        struct X25519KeyPair {
        uint8_t public_key[crypto_kx_PUBLICKEYBYTES];
        uint8_t secret_key[crypto_kx_SECRETKEYBYTES];
    };

        struct ServerIdentity {
        uint8_t private_key_id[crypto_sign_PUBLICKEYBYTES];
        uint8_t secret_key_id[crypto_sign_SECRETKEYBYTES];
    };

        struct CryptoSessionConfig {
        uint8_t Client_to_Server[crypto_kx_SESSIONKEYBYTES];
        uint8_t Server_to_Client[crypto_kx_SESSIONKEYBYTES];
        uint8_t nonce[crypto_secretbox_NONCEBYTES];
        uint8_t signature_message[crypto_sign_BYTES*2];
        bool role_client; // true if client
    };

    enum Role {
        ROLE_CLIENT,
        ROLE_SERVER
    };

        CryptoManager(Role role, bool enable_encryption = true, bool skip_server_identity_verification = false);
        std::string encrypt_data(const std::vector<unsigned char>& plaintext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& nonce);
        std::string decrypt_data(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& nonce);

        ServerIdentity generate_static_server_identity();
        void store_server_identity(const ServerIdentity& id);
        X25519KeyPair generate_x25519_keypair();
        ServerIdentity load_local_server_identity();
        CryptoSessionConfig establish_server_session();
        void generate_client_hello();
        CryptoSessionConfig establish_client_session();

    private:
        ServerIdentity server_identity_; // Only relevant for server side
        X25519KeyPair client_buffer_ephemeral_; // Only relevant for client side
        bool initialized_ = false;
        Role role_;
        bool server_identity_set_ = false;
        bool enable_encryption_ = false;
        bool skip_server_identity_verification_ = false;

        // Map of session configs by client ID - relevant for server and client side. 
        // Overhead for client side is acceptable for simplicity since only one active session needed. 

        std::unordered_map<std::string, CryptoSessionConfig> session_map_; 
};

