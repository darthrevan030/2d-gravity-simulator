add_executable(grav-soa)
target_sources(grav-soa PRIVATE 
    src/main.cpp
    include/particle_system.hpp
    src/particle_system.cpp
    src/collisions.cpp
    src/integrate.cpp
    src/file_io.cpp
)

target_include_directories(grav-soa PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(grav-soa PRIVATE project_warnings GSL common)