cmake_minimum_required(VERSION 3.10)
project(mqtt_tunnel)

set(CMAKE_CXX_STANDARD 20)

enable_testing()


find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSODIUM REQUIRED IMPORTED_TARGET libsodium)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(mqtt_tunnel 
    src/main.cpp
    src/TunDevice.cpp
    src/MQTTChannels.cpp
    src/TunnelClient.cpp
    src/TunnelServer.cpp
    src/ip_pool.cpp
    src/CryptoManager.cpp
    src/TunCallback.cpp
)

target_link_libraries(mqtt_tunnel 
    PahoMqttCpp::paho-mqttpp3
    pthread
    spdlog::spdlog
    fmt::fmt
    PkgConfig::LIBSODIUM
    )


add_executable(mqtt_tunnel_tests
    tests/ip_pool_test.cpp
    tests/helpers_test.cpp
    src/ip_pool.cpp   
    src/helpers.h
)

target_link_libraries(mqtt_tunnel_tests
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    spdlog::spdlog
    fmt::fmt
    PkgConfig::LIBSODIUM
)

include(GoogleTest)
gtest_discover_tests(mqtt_tunnel_tests)

