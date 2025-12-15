#include "tunnel_manager.h"
#include "helpers.h"

TunnelManager::TunnelManager(std::string command_channel_name, std::string client_base_id,std::string broker_address){
    command_channel_name_ = command_channel_name;

    mqtt_command_client_ = std::make_unique<mqtt::client>(broker_address, client_base_id + "_cmd");
}

SessionConfig TunnelManager::setup_session(){

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distr;

    std::stringstream ss;

    for(int i =0; i < 4 ; ++i) {
        ss << std::setw(16)  << std::setfill('0') << std::hex << distr(gen);
    }

    SessionConfig config;
    // Client hello message to the server with optional authentication or identification data
    ClientHello client_hello;
    client_hello.message_identifier = "CLIENT_HELLO";
    client_hello.client_base_id = session_config_.client_id;
    client_hello.authentication = false; // no authentication as a base case => not implemented yet
    client_hello.auth_data = "";
    client_hello.handshake_identifier = ss.str(); 

    mqtt_command_client_->connect();
    mqtt_command_client_->subscribe(command_channel_name_, 1);
    mqtt::message_ptr pubmsg = mqtt::make_message(command_channel_name_, client_hello.to_string());
    pubmsg->set_qos(1);
    mqtt_command_client_->publish(pubmsg);

    mqtt::const_message_ptr msg = mqtt_command_client_->consume_message();
    if(!msg) {
        throw std::runtime_error("Failed to receive Server Hello message");
    }

    ServerHello server_hello = ServerHello::from_string(msg->get_payload());

    // TODDO: verify server_hello.handshake_identifier matches client_hello.handshake_identifier

    session_config_.client_id = server_hello.assigned_client_id_;
    session_config_.client_address = server_hello.assigned_client_ip;
    session_config_.topic_inbound = server_hello.assigned_inbound_topic;
    session_config_.topic_outbound = server_hello.assigned_outbound_topic;

    ClientACK client_ack;
    client_ack.message_identifier = "CLIENT_ACK";
    client_ack.handshake_identifier = server_hello.handshake_identifier;
    
    mqtt::message_ptr ack_msg = mqtt::make_message(command_channel_name_, client_ack.to_string());
    ack_msg->set_qos(1);
    mqtt_command_client_->publish(ack_msg);

    mqtt::const_message_ptr ack_response = mqtt_command_client_->consume_message();
    if(!ack_response) {
        throw std::runtime_error("Failed to receive Server ACK message");
    }

    ServerACK server_ack = ServerACK::from_string(ack_response->get_payload());

    // TODO: verify server_ack.handshake_identifier matches client_hello.handshake_identifier

    session_configured_ = true;

    return config;
};


void TunnelManager::connect_data_channel(){
    
    if(!session_configured_){
        std::cerr << "Session not configured. Cannot connect data channel." << std::endl;
        std::cerr << "Start session setup..." << std::endl;

        setup_session();
    }

    mqtt_data_client_ = std::make_unique<mqtt::async_client>(mqtt_command_client_->get_server_uri(), session_config_.client_id);
    mqtt_data_client_->set_callback(*tun_callback_);
    mqtt::connect_options conn_opts;
    conn_opts.set_clean_session(true);

    try {

        mqtt_data_client_->connect(conn_opts)->wait();
        mqtt_data_client_->subscribe(session_config_.topic_inbound, 1)->wait();
        
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    
    std::cout << "Data channel connected" << std::endl;

}
