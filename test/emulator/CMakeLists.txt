add_executable(emulator_tests
        test_emulator.cpp
)

target_link_libraries(emulator_tests PRIVATE emulator mocks)
target_link_system_libraries(emulator_tests PRIVATE GTest::gtest GTest::gtest_main)

gtest_discover_tests(emulator_tests)
