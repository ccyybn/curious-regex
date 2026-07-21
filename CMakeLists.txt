cmake_minimum_required(VERSION 3.14)
project(curious-regex CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(curious_regex_lib INTERFACE)
target_include_directories(curious_regex_lib
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(curious-regex src/main.cpp)

target_link_libraries(curious-regex PRIVATE curious_regex_lib)
