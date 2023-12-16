add_library(chissembler STATIC
        include/chissembler/chissembler.hpp
        chissembler.cpp
        source_location.hpp
        token_type.hpp
        token.hpp
        lexer.cpp
        lexer.hpp
        instruction.cpp
        instruction.hpp
        target.hpp
        emitter.cpp
        emitter.hpp
        utils.hpp
        include/chissembler/errors.hpp
)

target_link_libraries(chissembler
        PRIVATE
        project_options
        PUBLIC
        common
)

target_link_system_libraries(chissembler PRIVATE magic_enum::magic_enum)

target_include_directories(chissembler
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/chissembler
)
