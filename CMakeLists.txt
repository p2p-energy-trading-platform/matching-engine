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

# Dependencies

# Third-party dependencies provided by Conan
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(RdKafka CONFIG REQUIRED)



# GridX generated C++ Protobuf SDK
include(FetchContent)

FetchContent_Declare(
    gridx_cpp_sdk
    GIT_REPOSITORY
        https://github.com/p2p-energy-trading-platform/cpp-sdk.git
    GIT_TAG
        v0.6.1
    GIT_SHALLOW
        TRUE
)

FetchContent_MakeAvailable(gridx_cpp_sdk)

# Compiler Warnings
function(gridx_enable_warnings target_name)
    if(MSVC)
        target_compile_options(
            ${target_name}
            PRIVATE
                /W4
                /WX
                /permissive-
        )
    else()
        target_compile_options(
            ${target_name}
            PRIVATE
                -Wall
                -Wextra
                -Werror
                -Wpedantic
                -Wshadow
        )
    endif()
endfunction()

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
     src/validation/OrderValidator.cpp
     src/adapters/kafka/OrderEventMapper.cpp
     src/adapters/kafka/ProtobufOrderCodec.cpp
     src/orderbook/MarketBook.cpp
     src/orderbook/ZoneOrderBook.cpp
     src/matching/SameZoneMatcher.cpp
     src/matching/MatchingEngine.cpp
     src/matching/TradeManager.cpp
     src/engine/OrderProcessor.cpp
     src/adapters/kafka/KafkaOrderConsumer.cpp
     src/expiry/ExpiryManager.cpp
    # or we can use file(GLOB ...) to automatically include all .cpp files in the src directory
)

target_compile_features(
    gridx_matching_core
    PUBLIC
        cxx_std_20
)

target_include_directories(
    gridx_matching_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Link gridx_matching_core library to the matching-engine executable
target_link_libraries(
    gridx_matching_core
    PUBLIC
        gridx::cpp_sdk
    PRIVATE
        fmt::fmt
        spdlog::spdlog
        RdKafka::rdkafka++
)

gridx_enable_warnings(gridx_matching_core)

# Main executable

add_executable(
    matching-engine
    src/main.cpp
)

# Include directories for the gridx_matching_core library
#[[ 
Note: The include directory is specified as PUBLIC,
which means that it will be used both for compiling the gridx_matching_core library
and for any targets(matching-engine) that link against it.
]]
target_link_libraries(
    matching-engine
    PRIVATE
        gridx_matching_core
)

gridx_enable_warnings(matching-engine)

# Enable interprocedural optimization for Release builds where supported.
include(CheckIPOSupported)

check_ipo_supported(
    RESULT GRIDX_IPO_SUPPORTED
    OUTPUT GRIDX_IPO_ERROR
)

if(GRIDX_IPO_SUPPORTED)
    set_property(
        TARGET gridx_matching_core
        PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
    )
else()
    message(
        STATUS
        "IPO/LTO not supported: ${GRIDX_IPO_ERROR}"
    )
endif()

# Find clang-format
find_program(CLANG_FORMAT_EXECUTABLE clang-format )

# Recursively find all source files for formatting
file(GLOB_RECURSE PROJECT_SOURCE_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
)

# Add custom targets for formatting and format checking
if(CLANG_FORMAT_EXECUTABLE AND PROJECT_SOURCE_FILES)
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${PROJECT_SOURCE_FILES}
        COMMENT "Formatting source files"
        VERBATIM
    )

    add_custom_target(
        format-check
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror ${PROJECT_SOURCE_FILES}
        COMMENT "Checking code formatting"
        VERBATIM
    )
endif()

# Find clang-tidy
find_program(CLANG_TIDY_EXECUTABLE clang-tidy )

file(
    GLOB_RECURSE PROJECT_CPP_SOURCE_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)

# Add custom target for linting
if(CLANG_TIDY_EXECUTABLE)
    add_custom_target(
        lint
        COMMAND ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_FILES}
        COMMENT "Running clang-tidy"
        VERBATIM
    )
endif()

# Testing
include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()



