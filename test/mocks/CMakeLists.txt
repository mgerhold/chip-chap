add_library(mocks INTERFACE
        mock_time_source.hpp
        mock_input_source.hpp
        mock_screen.hpp
)
target_include_directories(mocks INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mocks INTERFACE emulator)
