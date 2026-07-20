from conan import ConanFile
from conan.tools.cmake import cmake_layout


class GridXMatchingEngineConan(ConanFile):
    name = "gridx-matching-engine"
    version = "0.1.0"

    # Package Metadata
    license = "Proprietary"
    author = "GridX Team"
    url = "https://github.com/p2p-energy-trading-platform/matching-engine"
    description = "GridX High-Performance Low-Latency Matching Engine"

    # Build Settings
    settings = (
        "os",
        "compiler",
        "build_type",
        "arch",
    )

    # Build Generators
    generators = (
        "CMakeDeps",
        "CMakeToolchain",
    )

    # Dependencies
    requires = (
        "spdlog/1.15.3",
        "fmt/11.2.0",
        "protobuf/7.35.0",
        "librdkafka/2.14.2",
    )

    test_requires = (
        "gtest/1.16.0",
    )

    # Project Layout
    def layout(self):
        cmake_layout(self)