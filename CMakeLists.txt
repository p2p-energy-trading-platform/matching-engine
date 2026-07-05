cmake_minimum_required(VERSION 3.24)

project(
    gridx_matching_engine
    VERSION 0.1.0
    DESCRIPTION "GridX High-Performance Low-Latency Matching Engine"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for clangd and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default Build Type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Compiler Warnings
if(MSVC)
    add_compile_options(
        /W4
        /WX
        /permissive-
    )
else()
    add_compile_options(
        -Wall
        -Wextra
        -Werror
        -Wpedantic
        -Wshadow
    )
endif()

# Main Executable
add_executable(
    matching-engine
    src/main.cpp
)

target_include_directories(
    matching-engine
    PRIVATE
        include
)

target_compile_features(
    matching-engine
    PRIVATE
        cxx_std_20
)

# Testing
enable_testing()