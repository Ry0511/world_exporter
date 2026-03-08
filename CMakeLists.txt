cmake_minimum_required(VERSION 3.25)
project(world_exporter VERSION 1.0.0)

add_subdirectory(vendor/pyunrealsdk EXCLUDE_FROM_ALL)
pyunrealsdk_add_module(
    world_exporter
    "src/world_exporter/world_exporter.cpp"
    "src/world_exporter/helpers.h"
)
target_include_directories(world_exporter PUBLIC ./src)

install(
    TARGETS world_exporter
    RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/src/world_exporter/py"
    LIBRARY DESTINATION "${PROJECT_SOURCE_DIR}/src/world_exporter/py"
    ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/src/world_exporter/py"
)

install(
    FILES $<TARGET_PDB_FILE:world_exporter>
    DESTINATION "${PROJECT_SOURCE_DIR}/src/world_exporter/py"
    OPTIONAL
)