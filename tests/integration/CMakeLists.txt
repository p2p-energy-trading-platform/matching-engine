find_package(GTest REQUIRED)

add_executable(
    integration-tests

)

target_link_libraries(
    integration-tests

    PRIVATE
        gridx_matching_core
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(
    integration-tests
    
    PROPERTIES
        LABELS "integration"
)