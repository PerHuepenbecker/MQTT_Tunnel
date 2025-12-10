cmake_minimum_required(VERSION 3.10)
project(mqtt_tunnel)

set(CMAKE_CXX_STANDARD 23)

enable_testing()


find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(mqtt_tunnel src/main.cpp)

target_link_libraries(mqtt_tunnel 
    paho-mqtt3c
    pthread
)

add_executable(mqtt_tunnel_tests
    tests/ip_pool_test.cpp
    src/ip_pool.cpp
    
)

target_link_libraries(mqtt_tunnel_tests
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)

include(GoogleTest)
gtest_discover_tests(mqtt_tunnel_tests)

