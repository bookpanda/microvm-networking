cmake_minimum_required(VERSION 3.12)

# Include directories for examples
include_directories(../include)

# Example programs
add_executable(hello_world hello_world.c)
target_link_libraries(hello_world microvm_net_static)

add_executable(echo_server echo_server.c)
target_link_libraries(echo_server microvm_net_static)

add_executable(echo_client echo_client.c)
target_link_libraries(echo_client microvm_net_static)

add_executable(benchmark benchmark.c)
target_link_libraries(benchmark microvm_net_static)

# Install examples
install(TARGETS hello_world echo_server echo_client benchmark
    RUNTIME DESTINATION bin
)
