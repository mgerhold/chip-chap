add_library(common INTERFACE
        include/common/random.hpp
        include/common/types.hpp
        include/common/visitor.hpp
        include/common/ostream_formatter.hpp
)
target_link_system_libraries(common INTERFACE tl::optional Microsoft.GSL::GSL)
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
