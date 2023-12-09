add_executable(chip_chap main.cpp
        window.cpp
        window.hpp
        color.hpp
        event.hpp
        key_code.hpp
        application.cpp
        application.hpp
        chip_chap.hpp
        screen.cpp
        screen.hpp
        input_source.cpp
        input_source.hpp
        time_source.cpp
        time_source.hpp
        chip_chap.cpp
)

target_link_libraries(chip_chap
        PRIVATE
        project_options
        common
        emulator
)

target_link_system_libraries(chip_chap
        PRIVATE
        imgui
        Microsoft.GSL::GSL
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        tl::expected
        tl::optional
        glad::glad
)
