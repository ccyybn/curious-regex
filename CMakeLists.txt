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

include(CTest)

if(BUILD_TESTING)
    include(FetchContent)
    
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(unit_tests
        tests/test_engine.cpp
    )

    target_link_libraries(unit_tests
        PRIVATE
            curious_regex_lib
            Catch2::Catch2WithMain
    )

    include(Catch)
    catch_discover_tests(unit_tests)
endif()
