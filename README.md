# MQTT_Tunnel
Educational Project - Implementation of a MQTT-Network tunnel

# Dependencies

- paho-MQTT C++
- GTest
- OpenSSL
- libsodium
- cereal 
- spdlog
- fmt


# Installation

1. Install dependencies either via packet manager or compile from source
2. Clone git repository
3. Navigate into cloned repo
4. run mkdir build && cd build
5. run cmake ..
6. run make

# Usage

### Execution Syntax
```bash
sudo ./mqtt_tunnel [ARGUMENTS]
```

Arguments are:

| Argument          | Type  | Flag                     | Required | Description                                                      |
|-------------------|-------|--------------------------|----------|------------------------------------------------------------------|
| Operating Mode    | `<str>` | `-m, --mode`             | Yes      | Defines the tunnel operation mode (client, server).            |
| Broker            | `<url>` | `-b, --broker`           | Yes      | The MQTT broker URI (e.g., `tcp://localhost:8883`).            |
| Client ID         | `<str>` | `-i, --client-id`        | Yes      | Identifier                                                     |
| Command Channel   | `<str>` | `-c, --command-channel`  | Yes      | The MQTT topic used for tunnel setuo                           |
| Verbose Mode      | `<flag>`| `-v, --verbose`          | No       | Verbose Mode for deeper insights.                              |
| Encryption        | `<flag>`| `-x, --insecure`         | No       | No Encryption                               |
| Server-Auth       | `<flag>`| `-t --ignore-server-auth`| No       | Client skips Public Key Identification of the server           | 
 

You need to run the compiled application with elevated privileges. 

Example: 

```bash
    ./mqtt_tunnel -m server -b tcp://192.168.0.2:1883 -c tunnelServer -C tunnelServerCommand -e
```

This will setup a server / gateway.


