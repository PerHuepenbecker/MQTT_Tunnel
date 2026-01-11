#include "CryptoManager.hpp"


std::string CryptoManager::encrypt_data(
    const std::vector<unsigned char>& plaintext,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& nonce
) {
    if (key.size() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error("Invalid key size");
    }
    if (nonce.size() != crypto_secretbox_NONCEBYTES) {
        throw std::runtime_error("Invalid nonce size");
    }

    std::vector<unsigned char> ciphertext(plaintext.size() + crypto_secretbox_MACBYTES);

    if (crypto_secretbox_easy(
            ciphertext.data(),
            plaintext.data(),
            plaintext.size(),
            nonce.data(),
            key.data()
        ) != 0) {

        throw std::runtime_error("Encryption failed");
    }

    return std::string(ciphertext.begin(), ciphertext.end());
}

// Decrypt data using secretbox_easy API

std::string CryptoManager::decrypt_data(
    const std::vector<unsigned char>& ciphertext,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& nonce
) {
    if (key.size() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error("Invalid key size");
    }

    std::vector<unsigned char> plaintext(ciphertext.size() - crypto_secretbox_MACBYTES);

    if (crypto_secretbox_open_easy(
            plaintext.data(),
            ciphertext.data(),
            ciphertext.size(),
            nonce.data(),
            key.data()
        ) != 0) {

        throw std::runtime_error("Decryption failed");
    }

    return std::string(plaintext.begin(), plaintext.end());
}

// Necessary struct for ephemeral x25519 keypair
struct X25519KeyPair {
    uint8_t public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t secret_key[crypto_kx_SECRETKEYBYTES];
};

struct CryptoSessionConfig {
    uint8_t rx[crypto_kx_SESSIONKEYBYTES];
    uint8_t tx[crypto_kx_SESSIONKEYBYTES];
    uint8_t nonce[crypto_secretbox_NONCEBYTES];
    uint8_t signature_message[crypto_sign_BYTES*2];

};

// Function to generate a ephemeral x25519 keypair for key exchange

CryptoManager::X25519KeyPair CryptoManager::generate_x25519_keypair() {
    CryptoManager::X25519KeyPair keypair;
    if (crypto_kx_keypair(keypair.public_key, keypair.secret_key) != 0) {
        throw std::runtime_error("Failed to generate X25519 keypair");
    }
    return keypair;
}

// Function to generate a static server identity keypair for identification to prevent MITM
// Current approach uses public key pinning as a simple method of server identification

CryptoManager::X25519KeyPair CryptoManager::generate_static_server_identity(){

    CryptoManager::X25519KeyPair server_identity;

    uint8_t server_id_public_key[crypto_kx_PUBLICKEYBYTES];
    uint8_t server_id_secret_key[crypto_kx_SECRETKEYBYTES];

    crypto_sign_keypair(server_identity.public_key, server_identity.secret_key);
    return server_identity;

}

CryptoManager::CryptoSessionConfig CryptoManager::establish_server_session(){
    CryptoSessionConfig crypto_session;

    auto server_ephemeral = generate_x25519_keypair();

    uint8_t signed_server_ephemeral_key[crypto_sign_BYTES];

    uint8_t message[crypto_kx_PUBLICKEYBYTES + crypto_kx_SECRETKEYBYTES];

    memcpy(message, server_ephemeral.public_key, 32);
    memcpy(message + 32, server_ephemeral.secret_key, 32);

    crypto_sign_detached(signed_server_ephemeral_key, nullptr, message, 64, server_identity_.secret_key);

    memcpy(crypto_session.signature_message, signed_server_ephemeral_key, crypto_sign_BYTES);

    if (crypto_kx_server_session_keys(
            crypto_session.Client_to_Server,
            crypto_session.Server_to_Client,
            server_ephemeral.public_key,
            server_ephemeral.secret_key,
            server_identity_.public_key
        ) != 0) {
        throw std::runtime_error("Failed to establish server session keys");
    }

    return crypto_session;
}

CryptoManager::CryptoSessionConfig CryptoManager::establish_client_session() {
    CryptoManager::CryptoSessionConfig crypto_session;
    auto client_ephemeral = generate_x25519_keypair();

    return crypto_session;
}

CryptoManager::CryptoSessionConfig CryptoManager::generate_client_hello() {
    CryptoManager::CryptoSessionConfig crypto_session;
    auto client_ephemeral = generate_x25519_keypair();

    return crypto_session;
}

