add_library(chissembler STATIC
        include/chissembler/chissembler.hpp
        chissembler.cpp
)

target_link_libraries(chissembler
        PRIVATE
        project_options
        PUBLIC
        common
)

target_include_directories(chissembler
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/chissembler
)
