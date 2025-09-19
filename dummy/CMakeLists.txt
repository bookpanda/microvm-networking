cmake_minimum_required(VERSION 3.16)

project(dummy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(dummy src/main.cpp)
