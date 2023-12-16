add_executable(chissembler_tests
        test_chissembler.cpp
)

target_link_libraries(chissembler_tests PRIVATE chissembler mocks)
target_link_system_libraries(chissembler_tests PRIVATE GTest::gtest GTest::gtest_main)

gtest_discover_tests(chissembler_tests)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_executable(fuzzer_chissembler
            fuzzer_chissembler.cpp
    )

    target_link_libraries(fuzzer_chissembler PRIVATE project_options chissembler mocks)
    target_compile_options(fuzzer_chissembler PRIVATE -fsanitize=address,undefined,fuzzer -fno-omit-frame-pointer -g)
    target_link_options(fuzzer_chissembler PRIVATE -fsanitize=address,undefined,fuzzer)
endif ()
