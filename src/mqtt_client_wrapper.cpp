#include "mqtt_client_wrapper.h"

MQTTClientWrapper::MQTTClientWrapper(const std::string& broker_uri, const std::string& client_id){

    MQTTClient_create(&mqtt_client_, broker_uri.c_str(), client_id.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts_ = MQTTClient_connectOptions_initializer;

    // Set default connectio options
    conn_opts_.keepAliveInterval = 20;
    conn_opts_.cleansession = 1;

}

void MQTTClientWrapper::connect(){
    MQTTClient_connect(mqtt_client_, &conn_opts_);
}

void MQTTClientWrapper::disconnect(){
    MQTTClient_disconnect(mqtt_client_, 1000);
}

void MQTTClientWrapper::subscribe(const std::string& topic) {
    MQTTClient_subscribe(mqtt_client_, topic.c_str(), 1);
}

void MQTTClientWrapper::publish(const std::string& topic, const std::vector<unsigned char>& payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = (void*)payload.data();
    pubmsg.payloadlen = static_cast<int>(payload.size());
    pubmsg.qos = 1; // Standard QoS level 1, so message will be delivered at least once
    pubmsg.retained = 0;
    MQTTClient_publishMessage(mqtt_client_, topic.c_str(), &pubmsg, NULL);
}


void MQTTClientWrapper::connopts_set_keepalive(int keepalive){
    conn_opts_.keepAliveInterval = keepalive;
}

void MQTTClientWrapper::connopts_set_cleansession(bool cleansession){
    conn_opts_.cleansession = cleansession ? 1 : 0;
}

// TODO: Check handling here, since password will be kept quite long in memory

void MQTTClientWrapper::connopts_set_username_password(const std::string& username, const std::string& password){
    conn_opts_.username = username.c_str();
    conn_opts_.password = password.c_str();
}