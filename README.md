# MQTT_Tunnel
Educational Project - Implementation of a MQTT-Network tunnel

# Features

- Tunneling of all layer 3 traffic 
- dynamic session establishment via command channels
- end-to-end encryption beyond tls
- server authentication via ED25519
- Gateway-mode for routing all layer 3 traffic through the tunnel
- Connection mode for p2p connections

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
| Broker            | `<url>` | `-b, --broker`           | Yes      | The MQTT broker URI (e.g., `tcp://localhost:1883`).            |
| Client ID         | `<str>` | `-i, --client-id`        | Yes      | Identifier                                                     |
| Command Channel   | `<str>` | `-t, --command-topic`    | Yes      | The MQTT topic used for tunnel setuo                           |
| Tunnel Mode       | `<str>` | `-T, --tunnel-mode`      | Yes      | Tunnel mode - simple connection or gateway                     |
| Verbose Mode      | `<flag>`| `-v, --verbose`          | No       | Verbose Mode for deeper insights.                              |
| Encryption        | `<flag>`| `--insecure`             | No       | No Encryption                                                  |
| Server-Auth       | `<flag>`| `--no-server-auth`       | No       | Client skips Public Key Identification of the server           | 
| Interface         | `<str>` | `--interface`            | No       | Interface for the NAT rules in gateway mode (server side)      |

 

You need to run the compiled application with elevated privileges. 

Example server setup: 

```bash
    ./mqtt_tunnel -m server -b tcp://192.168.0.2:1883 -i tunnelServer -t tunnelServerCommand -v
```
Matching client setup:

```bash
    ./mqtt_tunnel -m client -b tcp://192.168.0.2:1883 -i tunnelClient -t tunnelServerCommand -T gateway --no-server-auth
```

This will setup a server and a client that connects to the server via the defined broker in gateway mode without checking the server public key.


