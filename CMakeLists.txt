cmake_minimum_required(VERSION 3.10)
project(mqtt_tunnel)

set(CMAKE_CXX_STANDARD 23)

add_executable(mqtt_tunnel src/main.cpp)

target_link_libraries(mqtt_tunnel 
    paho-mqtt3c
    pthread
)

