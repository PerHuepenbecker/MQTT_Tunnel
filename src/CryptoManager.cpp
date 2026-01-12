#include "CryptoManager.hpp"



CryptoManager::CryptoManager(CryptoManager::Role role, bool enable_encryption, bool skip_server_identity_verification) : role_(role), enable_encryption_(enable_encryption), skip_server_identity_verification_(skip_server_identity_verification) {
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }
    initialized_ = true;

    if (role_ == ROLE_SERVER) {
        try {
            server_identity_ = load_local_server_identity();
            server_identity_set_ = true;
        } catch (const std::exception& e) {
            spdlog::warn("Server identity not set. Generateing and storing a new identity before starting the server.");
            server_identity_ = generate_static_server_identity();
            store_server_identity(server_identity_);
            spdlog::info("New server identity generated and stored.");

            std::stringstream ss;
            ss << "Server Public Key (hex): ";
            for (size_t i = 0; i < crypto_sign_PUBLICKEYBYTES; ++i) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(server_identity_.public_key_id[i]);
            }
            spdlog::info(ss.str());
            
            spdlog::info("Please distribute this public key for server pinning.");
        }
    
        server_identity_set_ = true;
    }

}

std::string CryptoManager::encrypt_data(const std::vector<unsigned char>& plaintext, std::string& client_id) {

    std::vector<unsigned char> ciphertext(plaintext.size() + crypto_secretbox_MACBYTES);

    auto it = session_map_.find(client_id);
    if (it == session_map_.end()) {
        throw std::runtime_error("Session not found for client ID: " + client_id);
    }

    const CryptoSessionConfig& session = it->second;

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    std::vector<unsigned char> full_packet(crypto_secretbox_NONCEBYTES + plaintext.size() + crypto_secretbox_MACBYTES);
    memcpy(full_packet.data(), nonce, crypto_secretbox_NONCEBYTES);

    if (crypto_secretbox_easy(
            full_packet.data() + crypto_secretbox_NONCEBYTES, // So that only the packet data after nonce is encrypted
            plaintext.data(),
            plaintext.size(),
            nonce,
            session.tx
        ) != 0) {
        throw std::runtime_error("Encryption failed");
    }

    return std::string(full_packet.begin(), full_packet.end());
}

// Decrypt data using secretbox_easy API

std::string CryptoManager::decrypt_data(
    const std::vector<unsigned char>& full_packet,
    std::string& client_id) 
    {

        spdlog::debug("Decrypting data for client ID: {}", client_id);

        auto it = session_map_.find(client_id);
        if (it == session_map_.end()) {
            throw std::runtime_error("Session not found for client ID: " + client_id);
        }  

        const CryptoSessionConfig& session = it->second;

        const uint8_t* nonce_pointer = full_packet.data();
        const uint8_t* ciphertext_pointer = full_packet.data() + crypto_secretbox_NONCEBYTES;
        size_t ciphertext_len = full_packet.size() - crypto_secretbox_NONCEBYTES;

        std::vector<unsigned char> plaintext(ciphertext_len - crypto_secretbox_MACBYTES);
        
        if(crypto_secretbox_open_easy(
            plaintext.data(),
            ciphertext_pointer,
            ciphertext_len,
            nonce_pointer,
            session.rx
        ) != 0) {
            throw std::runtime_error("Decryption failed");
        }

         return std::string(plaintext.begin(), plaintext.end());
        }

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

CryptoManager::ServerIdentity CryptoManager::generate_static_server_identity() {
    CryptoManager::ServerIdentity id{};

    if (crypto_sign_keypair(id.public_key_id, id.secret_key_id) != 0) {
        throw std::runtime_error("Failed to generate static server identity (Ed25519)");
    }

    return id;
}



// load from local files

#include <fstream>

CryptoManager::ServerIdentity CryptoManager::load_local_server_identity() {
    ServerIdentity id{};
    {
        std::ifstream sk("ServerID.key", std::ios::binary);
        if (!sk)
            throw std::runtime_error("ServerID.key missing");
        sk.read(reinterpret_cast<char*>(id.secret_key_id),
                crypto_sign_SECRETKEYBYTES);
        if (sk.gcount() != crypto_sign_SECRETKEYBYTES)
            throw std::runtime_error("ServerID.key has wrong size");
    }

    {
        std::ifstream pk("ServerID.pub", std::ios::binary);
        if (!pk)
            throw std::runtime_error("ServerID.pub missing");
        pk.read(reinterpret_cast<char*>(id.public_key_id),
                crypto_sign_PUBLICKEYBYTES);
        if (pk.gcount() != crypto_sign_PUBLICKEYBYTES)
            throw std::runtime_error("ServerID.pub has wrong size");
    }

    return id;
}

ServerHelloCrypto CryptoManager::establish_server_session(ClientHelloCrypto& client_hello_crypto) {
    CryptoManager::CryptoSessionConfig crypto_session;  
    ServerHelloCrypto server_hello_crypto;

    auto server_ephemeral = generate_x25519_keypair();
    // Create unique identifier for this session will be used as areplacement for the client ID in session map
    server_hello_crypto.unique_identifier = client_hello_crypto.client_base_id+"_"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()); 
    
    if (crypto_sign_detached(
            crypto_session.signature_message, 
            nullptr, 
            server_ephemeral.public_key,      
            crypto_kx_PUBLICKEYBYTES, 
            server_identity_.secret_key_id    
        ) != 0) {
        throw std::runtime_error("Failed to sign ephemeral key");
    } 

    if (crypto_kx_server_session_keys(
            crypto_session.rx,   
            crypto_session.tx,    
            server_ephemeral.public_key,
            server_ephemeral.secret_key,
            client_hello_crypto.client_ephemeral_public_key                  
        ) != 0) {
        throw std::runtime_error("Failed to establish server session keys");
    }

    memcpy(server_hello_crypto.server_ephemeral_public_key, server_ephemeral.public_key, crypto_kx_PUBLICKEYBYTES);
    memcpy(server_hello_crypto.signature_message, crypto_session.signature_message, crypto_sign_BYTES);

    sodium_memzero(server_ephemeral.secret_key, sizeof(server_ephemeral.secret_key));

    spdlog::debug("Established server session for client ID: {}", client_hello_crypto.client_base_id);
    spdlog::debug("Session Unique Identifier: {}", server_hello_crypto.unique_identifier);

    session_map_.emplace(server_hello_crypto.unique_identifier, crypto_session);
    
    return server_hello_crypto;
}

ClientHelloCrypto CryptoManager::generate_client_hello(const std::string& client_base_id) {
    client_buffer_ephemeral_ = generate_x25519_keypair();
    ClientHelloCrypto client_hello;
    client_hello.client_base_id = client_base_id; 
    memcpy(client_hello.client_ephemeral_public_key, client_buffer_ephemeral_.public_key, crypto_kx_PUBLICKEYBYTES);
    return client_hello;
}


// Serialize helper functio fr public key to hex string for logging or display

std::string CryptoManager::hex_public_key(const ServerIdentity& id) {
    std::ostringstream ss;
    for (size_t i = 0; i < crypto_sign_PUBLICKEYBYTES; ++i) {

        ss << std::hex << std::setw(2) << std::setfill('0')
           << (int)id.public_key_id[i];
    }
    return ss.str();
}

void CryptoManager::load_server_public_key() {
    std::ifstream private_key_file("identity.pub", std::ios::binary);
    if (!private_key_file) {
        throw std::runtime_error("Failed to open identity.pub for reading");
    }
    private_key_file.read(reinterpret_cast<char*>(server_identity_.public_key_id), crypto_sign_PUBLICKEYBYTES);
    if(private_key_file.gcount() != crypto_sign_PUBLICKEYBYTES) {
        throw std::runtime_error("identity.pub has wrong size");
    }
    server_identity_set_ = true;
    spdlog::info("Server public key loaded successfully.");
}


void CryptoManager::store_server_identity(const ServerIdentity& id) {
    
    {
        std::ofstream sk("ServerID.key", std::ios::binary | std::ios::trunc);
        if (!sk)
            throw std::runtime_error("Failed to open ServerID.key for writing");
        sk.write(reinterpret_cast<const char*>(id.secret_key_id),
                 crypto_sign_SECRETKEYBYTES);
        if (!sk)
            throw std::runtime_error("Failed to write ServerID.key");
    }

    {
        std::ofstream pk("ServerID.pub", std::ios::binary | std::ios::trunc);
        if (!pk)
            throw std::runtime_error("Failed to open ServerID.pub for writing");
        pk.write(reinterpret_cast<const char*>(id.public_key_id),
                 crypto_sign_PUBLICKEYBYTES);
        if (!pk)
            throw std::runtime_error("Failed to write ServerID.pub");
    }
}

void CryptoManager::establish_client_session(ServerHelloCrypto& server_hello_crypto) {

    // Laden der server identity aus lokalen Dateien
       if(!skip_server_identity_verification_){
        load_server_public_key();

        if (crypto_sign_verify_detached(
            server_hello_crypto.signature_message,
            server_hello_crypto.server_ephemeral_public_key,
            crypto_kx_PUBLICKEYBYTES,
            server_identity_.public_key_id
        ) != 0) {
            throw std::runtime_error("Server identity verification failed");
        }
        }
    
    CryptoManager::CryptoSessionConfig crypto_session;

    if (crypto_kx_client_session_keys(
            crypto_session.rx,
            crypto_session.tx,
            client_buffer_ephemeral_.public_key,
            client_buffer_ephemeral_.secret_key,
            server_hello_crypto.server_ephemeral_public_key
        ) != 0) {
        throw std::runtime_error("Failed to establish client session keys");
    }
    // Zero out here aswell because of security reasons
    sodium_memzero(client_buffer_ephemeral_.secret_key, sizeof(client_buffer_ephemeral_.secret_key));

    session_map_.emplace(server_hello_crypto.unique_identifier, crypto_session);
}

ServerHelloCrypto CryptoManager::generate_server_hello() {
    auto server_ephemeral = generate_x25519_keypair();
    ServerHelloCrypto server_hello;
    memcpy(server_hello.server_ephemeral_public_key, server_ephemeral.public_key, crypto_kx_PUBLICKEYBYTES);

    if (crypto_sign_detached(
            server_hello.signature_message,
            nullptr,
            server_ephemeral.public_key,
            crypto_kx_PUBLICKEYBYTES,
            server_identity_.secret_key_id
        ) != 0) {
        throw std::runtime_error("Failed to sign server ephemeral key");
    }

    // Zero out for security reasons
    sodium_memzero(server_ephemeral.secret_key, sizeof(server_ephemeral.secret_key));

    return server_hello;
}