add_library(common STATIC)
target_sources(common PRIVATE
    src/progargs.cpp
    src/input_parser.cpp
    src/config_writer.cpp
    src/collision_grid.cpp
    include/point.hpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)