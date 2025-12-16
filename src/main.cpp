
#include "TunnelClient.hpp"
#include "TunnelServer.hpp"
#include "../third_party/CLI11.hpp"

enum Mode {
    MODE_CLIENT,
    MODE_SERVER
};

int main(int argc, char** argv) {

    // parse the command line arguments

    CLI::App app{"MQTT Tunnel"};

    std::string broker_address;
    std::string client_id;
    std::string command_channel_name = "mqtt_tunnel/commands";
    
    Mode mode = MODE_CLIENT;

    app.add_option("-m,--mode", mode, "Mode: client or server")->required()
        ->transform(CLI::CheckedTransformer(std::map<std::string, Mode>{{"client", MODE_CLIENT}, {"server", MODE_SERVER}}, CLI::ignore_case));
    app.add_option("-b,--broker", broker_address, "MQTT Broker Address")->required();
    app.add_option("-c,--client-id", client_id, "Client ID for MQTT connection")->required();
    app.add_option("-C,--command-channel", command_channel_name, "MQTT Command Channel Name");
    
    CLI11_PARSE(app, argc, argv);

    try {
        if (mode == MODE_CLIENT) {
            TunnelClientBuilder builder;
            builder.set_broker_address(broker_address)
                   .set_client_base_id(client_id)
                   .set_command_channel_name(command_channel_name);
            auto client = builder.build();
            client->start_tunnel();
            pause();
        }

        else if (mode == MODE_SERVER) {
            TunnelServerBuilder builder;
            builder.set_broker_address(broker_address)
                   .set_command_channel_name(command_channel_name);
            auto server = builder.build();
            server->start_server();
            
        }
    } catch (const std::exception& ex) {
        spdlog::error("Exception: {}", ex.what());
        return 1;
    }


    
    return 0;
}