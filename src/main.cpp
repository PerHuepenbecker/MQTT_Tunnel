
#include "TunnelClient.hpp"
#include "TunnelServer.hpp"
#include "../third_party/CLI11.hpp"
#include <csignal>
#include <sodium.h>


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

    bool verbose = false;
    bool unsecure = false;
    bool encryption = true;
    bool ignore_server_authentication = false;
    
    Mode mode = MODE_CLIENT;

    app.add_option("-m,--mode", mode, "Mode: client or server")->required()
        ->transform(CLI::CheckedTransformer(std::map<std::string, Mode>{{"client", MODE_CLIENT}, {"server", MODE_SERVER}}, CLI::ignore_case));
    app.add_option("-b,--broker", broker_address, "MQTT Broker Address")->required();
    app.add_option("-i,--client-id", client_id, "Client ID for MQTT connection")->required();
    app.add_option("-t,--command-topic", command_channel_name, "MQTT Command Channel Name");
    app.add_flag("-v,--verbose", verbose, "Enable verbose logging");
    app.add_flag("--insecure", unsecure, "Disable encryption (not recommended)");
    app.add_flag("-u,--ignore-server-auth", ignore_server_authentication, "Ignore server authentication (not recommended)");

    CLI11_PARSE(app, argc, argv);

    std::signal(SIGINT, signal_handler);

    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
        spdlog::debug("Verbose logging enabled");
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    if (unsecure)
    {
        encryption = false;
    }
    

    try {
        if (mode == MODE_CLIENT) {
            TunnelClientBuilder builder;
            builder.set_broker_address(broker_address)
                   .set_client_base_id(client_id)
                   .set_command_channel_name(command_channel_name)
                   .set_enable_encryption(encryption)
                    .set_ignore_server_authentication(ignore_server_authentication);

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
                   .set_command_channel_name(command_channel_name)
                   .set_global_run_flag(&run)
                   .set_enable_encryption(encryption);
            auto server = builder.build();
            server->start_server();
            
        }
    } catch (const std::exception& ex) {
        spdlog::error("Exception: {}", ex.what());
        return 1;
    }


    
    return 0;
}