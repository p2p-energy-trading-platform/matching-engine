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
    adapters/kafka/ProtobufGridTransferCodecTest.cpp
    adapters/kafka/GridTransferEventMapperTest.cpp
    expiry/ExpiryManagerTest.cpp
    orderbook/MarketBookManagerTest.cpp
    orderbook/OrderBookUpdaterTest.cpp
    matching/CrossZoneMatcherTest.cpp
    matching/EffectivePriceCalculatorTest.cpp
    adapters/kafka/TradeEventMapperTest.cpp
    adapters/kafka/OrderUpdateEventMapperTest.cpp
    matching/TradeManagerTest.cpp
    config/EnvConfigLoaderTest.cpp

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