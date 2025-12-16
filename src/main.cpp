
#include "TunnelClient.hpp"
#include "TunnelServer.hpp"
#include "../third_party/CLI11.hpp"
#include <csignal>

enum Mode {
    MODE_CLIENT,
    MODE_SERVER
};

    std::atomic<bool> run{true};


void signal_handler(int signal) {
    std::cout << "User requested shutdown..." << std::endl;
    run = false;
}


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

    std::signal(SIGINT, signal_handler);

  

    try {
        if (mode == MODE_CLIENT) {
            TunnelClientBuilder builder;
            builder.set_broker_address(broker_address)
                   .set_client_base_id(client_id)
                   .set_command_channel_name(command_channel_name);
            auto client = builder.build();
            client->start_tunnel();
            
            while (run) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep to reduce CPU usage
            }

            client->stop_tunnel();
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