cmake_minimum_required(VERSION 3.16)

project(dummy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


file(GLOB_RECURSE SOURCE_FILES 
${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)
add_executable(dummy ${SOURCE_FILES})
# include_directories(include)
target_include_directories(dummy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
