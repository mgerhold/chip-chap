add_executable(chissembler_tests
        test_chissembler.cpp
)

target_link_system_libraries(chissembler_tests PRIVATE GTest::gtest GTest::gtest_main)

gtest_discover_tests(chissembler_tests)
