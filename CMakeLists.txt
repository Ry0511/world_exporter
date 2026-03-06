cmake_minimum_required(VERSION 3.23)
project(world_exporter VERSION 1.0.0)

add_subdirectory(vendor/unrealsdk)

add_library(world_exporter SHARED)
target_precompile_headers(world_exporter PRIVATE src/world_exporter/pch.h)
target_include_directories(world_exporter PUBLIC src)
set_target_properties(
    world_exporter
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED 23
)

file(GLOB_RECURSE WORLD_EXPORTER_SOURCES "src/world_exporter/*.cpp")
message(STATUS "${WORLD_EXPORTER_SOURCES}")
target_sources(world_exporter PRIVATE ${WORLD_EXPORTER_SOURCES})

target_link_libraries(world_exporter PRIVATE unrealsdk)