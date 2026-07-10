find_package(GTest REQUIRED)

add_executable(
    unit-tests

    VersionTest.cpp
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