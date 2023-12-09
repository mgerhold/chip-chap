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


