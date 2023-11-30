include(${CMAKE_SOURCE_DIR}/project_options.cmake)

add_executable(chip_chap main.cpp
        window.cpp
        window.hpp
        renderer.cpp
        renderer.hpp
        color.hpp
        types.hpp
        event.hpp
        key_code.hpp
        visitor.hpp)

target_link_libraries(chip_chap PRIVATE project_options)

target_link_libraries(chip_chap PRIVATE Microsoft.GSL::GSL)
target_link_libraries(chip_chap
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
target_link_libraries(chip_chap PRIVATE tl::expected)
target_link_libraries(chip_chap PRIVATE tl::optional)
target_link_system_libraries(chip_chap PRIVATE imgui)

# target_link_system_libraries(chip_chap PRIVATE imgui SDL2::SDL2)
