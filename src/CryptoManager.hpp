
#include <sodium.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>
#include <unordered_map>

#pragma once

class CryptoManager {
    public:
        struct X25519KeyPair {
        uint8_t public_key[crypto_kx_PUBLICKEYBYTES];
        uint8_t secret_key[crypto_kx_SECRETKEYBYTES];
    };
        struct CryptoSessionConfig {
        uint8_t Client_to_Server[crypto_kx_SESSIONKEYBYTES];
        uint8_t Server_to_Client[crypto_kx_SESSIONKEYBYTES];
        uint8_t nonce[crypto_secretbox_NONCEBYTES];
        uint8_t signature_message[crypto_sign_BYTES*2];
        bool role_client; // true if client
    };

        CryptoManager();
        std::string encrypt_data(const std::vector<unsigned char>& plaintext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& nonce);
        std::string decrypt_data(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& nonce);

        X25519KeyPair generate_static_server_identity();
        X25519KeyPair generate_x25519_keypair();
        CryptoSessionConfig establish_server_session();
        CryptoSessionConfig generate_client_hello();
        CryptoSessionConfig establish_client_session();

    private:
        X25519KeyPair server_identity_; // Only relevant for server side
        X25519KeyPair client_buffer_ephemeral_; // Only relevant for client side

        // Map of session configs by client ID - relevant for server and client side. 
        // Overhead for client side is acceptable for simplicity since only one active session needed. 

        std::unordered_map<std::string, CryptoSessionConfig> session_map_; 
};

