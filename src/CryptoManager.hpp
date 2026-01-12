#pragma once

#include <sodium.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <spdlog/fmt/bin_to_hex.h>

#include "messages.h"

class CryptoManager {
    public:
        struct X25519KeyPair {
        uint8_t public_key[crypto_kx_PUBLICKEYBYTES];   
        uint8_t secret_key[crypto_kx_SECRETKEYBYTES];
    };

        struct ServerIdentity {
        uint8_t public_key_id[crypto_sign_PUBLICKEYBYTES];
        uint8_t secret_key_id[crypto_sign_SECRETKEYBYTES];
    };

        struct CryptoSessionConfig {
        uint8_t rx[crypto_kx_SESSIONKEYBYTES];
        uint8_t tx[crypto_kx_SESSIONKEYBYTES];
        uint8_t nonce[crypto_secretbox_NONCEBYTES];
        uint8_t signature_message[crypto_sign_BYTES*2];
        uint8_t server_ephemeral_public_key[crypto_kx_PUBLICKEYBYTES];

        bool role_client; // true if client
    };

    enum Role {
        ROLE_CLIENT,
        ROLE_SERVER
    };

        CryptoManager(Role role, bool enable_encryption = true, bool skip_server_identity_verification = false);
        std::string encrypt_data(const std::vector<unsigned char>& plaintext, std::string& client_id);
        std::string decrypt_data(const std::vector<unsigned char>& full_packet, std::string& client_id);

        ServerIdentity generate_static_server_identity();
        void store_server_identity(const ServerIdentity& id);
        X25519KeyPair generate_x25519_keypair();
        ServerIdentity load_local_server_identity();
        ServerHelloCrypto establish_server_session(ClientHelloCrypto& client_hello_crypto);
        ClientHelloCrypto generate_client_hello(const std::string& client_base_id);
        ServerHelloCrypto generate_server_hello();
        void load_server_public_key();
        void establish_client_session(ServerHelloCrypto& server_hello_crypto);

    private:
        ServerIdentity server_identity_; // Only relevant for server side
        X25519KeyPair client_buffer_ephemeral_; // Only relevant for client side
        bool initialized_ = false;
        Role role_;
        bool server_identity_set_ = false;
        bool enable_encryption_ = false;
        bool skip_server_identity_verification_ = false;
        CryptoSessionConfig client_crypto_session_;

        std::string hex_public_key(const ServerIdentity& id);
        // Map of session configs by client ID - relevant for server and client side. 
        // Overhead for client side is acceptable for simplicity since only one active session needed. 

        std::unordered_map<std::string, CryptoSessionConfig> session_map_;
};

