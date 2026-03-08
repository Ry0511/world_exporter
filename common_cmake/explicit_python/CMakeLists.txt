if(NOT DEFINED EXPLICIT_PYTHON_VERSION)
    message(FATAL_ERROR "EXPLICIT_PYTHON_VERSION not set!")
endif()
if(NOT DEFINED EXPLICIT_PYTHON_ARCH)
    message(FATAL_ERROR "EXPLICIT_PYTHON_ARCH not set!")
endif()

add_library(explicit_python INTERFACE)

include(FetchContent)
FetchContent_Declare(
    _explicit_python_base
    URL      https://www.python.org/ftp/python/${EXPLICIT_PYTHON_VERSION}/python-${EXPLICIT_PYTHON_VERSION}-${EXPLICIT_PYTHON_ARCH}.zip
    URL_HASH ${EXPLICIT_PYTHON_HASH_BASE}
)
# The test build has all the debug symbols, so we use it for all debug builds
FetchContent_Declare(
    _explicit_python_test
    URL      https://www.python.org/ftp/python/${EXPLICIT_PYTHON_VERSION}/python-${EXPLICIT_PYTHON_VERSION}-test-${EXPLICIT_PYTHON_ARCH}.zip
    URL_HASH ${EXPLICIT_PYTHON_HASH_TEST}
)
FetchContent_MakeAvailable(_explicit_python_base _explicit_python_test)

# Slightly stupid: download the embeddable build too, just to grab the zip inside it
# Could make it ourself with PyZipFile, but it's easier this way for cross compiling
FetchContent_Declare(
    _explicit_python_embeddable
    URL      https://www.python.org/ftp/python/${EXPLICIT_PYTHON_VERSION}/python-${EXPLICIT_PYTHON_VERSION}-embeddable-${EXPLICIT_PYTHON_ARCH}.zip
    URL_HASH ${EXPLICIT_PYTHON_HASH_EMBEDDABLE}
)
FetchContent_MakeAvailable(_explicit_python_embeddable)


target_include_directories(
    explicit_python INTERFACE
    "$<IF:$<CONFIG:DEBUG>,${_explicit_python_test_SOURCE_DIR}/include,${_explicit_python_base_SOURCE_DIR}/include>"
)

file(GLOB _libs_base "${_explicit_python_base_SOURCE_DIR}/libs/*.lib")
file(GLOB _libs_test "${_explicit_python_test_SOURCE_DIR}/libs/*.lib")
target_link_libraries(explicit_python INTERFACE "$<IF:$<CONFIG:DEBUG>,${_libs_test},${_libs_base}>")

file(GLOB _zip_embeddable "${_explicit_python_embeddable_SOURCE_DIR}/python*.zip")
install(FILES ${_zip_embeddable} DESTINATION ".")

# Want to install the same set of files, but swap source folder based on configuration
function(_install_python_files source_folder)
    file(GLOB dlls "${source_folder}/python*.dll")
    file(GLOB pyds "${source_folder}/python*.pyd")

    install(
        FILES ${dlls}
        CONFIGURATIONS ${ARGN}
        DESTINATION "."
    )
    # Regardless of the arg, only ever install pdbs on debug builds
    # Don't expect to match any of the base build
    install(
        FILES ${pdbs}
        CONFIGURATIONS Debug
        DESTINATION "DLLs"
    )

    # Repeat for the dlls folder
    file(GLOB dlls "${source_folder}/DLLs/*.dll")
    file(GLOB pyds "${source_folder}/DLLs/*.pyd")
    file(GLOB pdbs "${source_folder}/DLLs/*.pdb")

    install(
        FILES ${dlls} ${pyds}
        CONFIGURATIONS ${ARGN}
        DESTINATION "DLLs"
    )
    install(
        FILES ${pdbs}
        CONFIGURATIONS Debug
        DESTINATION "DLLs"
    )
endfunction()

_install_python_files(${_explicit_python_base_SOURCE_DIR} Release RelWithDebInfo MinSizeRel)
_install_python_files(${_explicit_python_test_SOURCE_DIR} Debug)

