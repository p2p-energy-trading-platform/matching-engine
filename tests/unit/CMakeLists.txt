find_package(GTest REQUIRED)

add_executable(
    unit-tests

    VersionTest.cpp
    validation/OrderValidatorTest.cpp
    orderbook/MarketBookTest.cpp
    orderbook/ZoneOrderBookTest.cpp
    orderbook/OrderBookTest.cpp
    matching/SameZoneMatcherTest.cpp
    adapters/kafka/OrderEventMapperTest.cpp
    adapters/kafka/ProtobufOrderCodecTest.cpp

)

target_link_libraries(
    unit-tests

    PRIVATE
        gridx_matching_core
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(
    unit-tests
    
    PROPERTIES
        LABELS "unit"
)