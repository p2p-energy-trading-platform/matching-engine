cmake_minimum_required(VERSION 3.24)

project(
    gridx_matching_engine
    VERSION 0.1.0
    DESCRIPTION "GridX High-Performance Low-Latency Matching Engine"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)              # Use C++20 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Require C++20 standard    
set(CMAKE_CXX_EXTENSIONS OFF)           # Do not use compiler-specific extensions

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
        /W4.              # MSVC version of -Wall and -Wextra
        /WX.              # MSVC version of -Werror
        /permissive-      # Strict standards compliance 
    )
else()
    add_compile_options(
        -Wall            # Enable most useful compiler warnings.
        -Wextra          # Enable extra compiler warnings that are not included by -Wall.
        -Werror          # Treat warnings as errors
        -Wpedantic.      # Issue all the warnings demanded by strict ISO C and ISO C++.
        -Wshadow         # Warn if a variable shadows another
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