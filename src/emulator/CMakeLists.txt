add_library(emulator STATIC
        include/chip8/chip8.hpp
        chip8.cpp
        include/chip8/basic_time_source.hpp
        include/chip8/basic_input_source.hpp
        include/chip8/basic_screen.hpp
)

target_link_libraries(emulator
        PRIVATE
        project_options
        PUBLIC
        common
)

target_include_directories(emulator
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/chip8
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    add_executable(emulator_tests
            ${CMAKE_SOURCE_DIR}/test/emulator/test_emulator.cpp
            ${CMAKE_SOURCE_DIR}/test/emulator/mock_time_source.hpp
            ${CMAKE_SOURCE_DIR}/test/emulator/mock_input_source.hpp
            ../../test/emulator/mock_screen.hpp
    )

    target_link_libraries(emulator_tests PRIVATE emulator)
    target_link_system_libraries(emulator_tests PRIVATE GTest::gtest GTest::gtest_main)

    gtest_discover_tests(emulator_tests)
endif ()
