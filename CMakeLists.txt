cmake_minimum_required(VERSION 3.25)
project(world_exporter VERSION 1.0.0)

set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(vendor/tinygltf EXCLUDE_FROM_ALL)

add_subdirectory(vendor/pyunrealsdk EXCLUDE_FROM_ALL)
pyunrealsdk_add_module(
    world_exporter
    "src/world_exporter/world_exporter.cpp"
    "src/world_exporter/cpp/world_exporter.cpp"
    "src/world_exporter/cpp/static_mesh_exporter.cpp"
    "src/world_exporter/cpp/static_mesh_component_exporter.cpp"
    "src/world_exporter/cpp/tinygltf_init.cpp"
    "src/world_exporter/cpp/helpers.cpp"
)
target_include_directories(world_exporter PUBLIC ./src)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "vendor")

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

add_executable(tinygltftest "./src/world_exporter/other/tinygltftest.cpp")
target_include_directories(tinygltftest PRIVATE "vendor")
set_target_properties(
    tinygltftest
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)