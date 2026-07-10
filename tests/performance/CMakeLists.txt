find_package(GTest REQUIRED)

add_executable(
    performance-tests

)

target_link_libraries(
    unit-tests
    
    PRIVATE
        gridx_matching_core
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(
    performance-tests
    
    PROPERTIES
        LABELS "performance"
)