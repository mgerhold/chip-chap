if (BUILD_TESTS)
    add_executable(emulator_tests
            test_emulator.cpp
            mock_time_source.hpp
            mock_input_source.hpp
            mock_screen.hpp
    )

    target_link_libraries(emulator_tests PRIVATE emulator)
    target_link_system_libraries(emulator_tests PRIVATE GTest::gtest GTest::gtest_main)

    gtest_discover_tests(emulator_tests)
endif ()
