cmake_minimum_required(VERSION 3.12)
project(microvm_net VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Default to Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# Source files
set(MICROVM_NET_SOURCES
    src/microvm_net.c
    src/ring_buffer.c
    src/buffer_pool.c
    src/packet_io_tap.c
)

# Create shared library
add_library(microvm_net SHARED ${MICROVM_NET_SOURCES})
target_link_libraries(microvm_net Threads::Threads)

# Create static library
add_library(microvm_net_static STATIC ${MICROVM_NET_SOURCES})
target_link_libraries(microvm_net_static Threads::Threads)
set_target_properties(microvm_net_static PROPERTIES OUTPUT_NAME microvm_net)

# Set library properties
set_target_properties(microvm_net PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Install targets
install(TARGETS microvm_net microvm_net_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Examples
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Package configuration
include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/microvm_net-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/microvm_net-config.cmake
    INSTALL_DESTINATION lib/cmake/microvm_net
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/microvm_net-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/microvm_net-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/microvm_net-config-version.cmake
    DESTINATION lib/cmake/microvm_net
)

# Create pkg-config file
configure_file(
    cmake/microvm_net.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/microvm_net.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/microvm_net.pc
    DESTINATION lib/pkgconfig
)
