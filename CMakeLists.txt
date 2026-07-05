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
        /W4               # MSVC version of -Wall and -Wextra
        /WX               # MSVC version of -Werror
        /permissive-      # Strict standards compliance 
    )
else()
    add_compile_options(
        -Wall            # Enable most useful compiler warnings.
        -Wextra          # Enable extra compiler warnings that are not included by -Wall.
        -Werror          # Treat warnings as errors
        -Wpedantic      # Issue all the warnings demanded by strict ISO C and ISO C++.
        -Wshadow         # Warn if a variable shadows another
    )

    # Enable aggressive compiler optimizations for production builds.
    add_compile_options($<$<CONFIG:Release>:-O3>)   # Optimize for speed in Release builds
    add_compile_options($<$<CONFIG:Release>:-flto>) # Enable Compile Time Optimization in Release builds
    add_link_options($<$<CONFIG:Release>:-flto>)    # Enable Link Time Optimization in Release builds

endif()

 # Core Matching Library
 #[[
 The gridx_matching_core is built as a static library to allow for easy integration 
 into other projects and to enable better optimization by the compiler.
 ]]
add_library(
    gridx_matching_core
    STATIC
)

# Add source files to the gridx_matching_core library
#[[
NOTE: PRIVATE is used here to indicate that the source files are only used for 
compiling the gridx_matching_core library and not for any targets that link against it.
]]
target_sources(
    gridx_matching_core
    PRIVATE
     src/common/Version.cpp
    # or we can use file(GLOB ...) to automatically include all .cpp files in the src directory
)

# Main Executable
add_executable(
    matching-engine
    src/main.cpp
)

# Link gridx_matching_core library to the matching-engine executable
target_link_libraries(
    matching-engine
    PRIVATE
        gridx_matching_core
)

# Include directories for the gridx_matching_core library
#[[ 
Note: The include directory is specified as PUBLIC,
which means that it will be used both for compiling the gridx_matching_core library
and for any targets(matching-engine) that link against it.
]]
target_include_directories(
    gridx_matching_core
    PUBLIC
        include
)

# Testing
enable_testing()


# We can add a test for the matching-engine executable later if needed. For now, we can leave it commented out.
# test(
#     NAME MatchingEngineTest
#     COMMAND matching-engine
# )