cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(Sphinx)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gdb-index")
# set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--gdb-index")
# set(CMAKE_EXE_LINKER_FLAGS "-Wl,-cache_path_lto,${CMAKE_BINARY_DIR}/cache")

add_definitions(
    -std=c++1z
    -g
    -O1
    -gsplit-dwarf
    #-Wall
    # -Weverything
    # -Wno-c++98-compat
    # -Wno-c++98-compat-pedantic
    # -Wno-exit-time-destructors
    # -Wno-global-constructors
    -Wno-padded
    # -Wno-weak-vtables
    # -Wno-shadow
    # -Wno-reserved-user-defined-literal
    # -Weffc++
    -Werror
    -DSPDLOG_FMT_EXTERNAL
    # -DSPHINX_NDEBUG
    # -flto=thin
    # -fvisibility=hidden
    # -fsanitize=undefined,cfi,safe-stack,dataflow,address
    # -fsanitize=undefined
    )
find_library(FMT fmt libfmt)
find_package(Threads REQUIRED)
find_package(Boost 1.62 COMPONENTS system filesystem program_options)
find_package(PostgreSQL REQUIRED)

find_package(PythonInterp)
find_program(iwyu_tool_path NAMES iwyu_tool.py)
if (iwyu_tool_path AND PYTHONINTERP_FOUND)
    add_custom_target(iwyu
        # ALL      # Remove ALL if you don't iwyu to be run by default.
        COMMAND "${PYTHON_EXECUTABLE}" "${iwyu_tool_path}" -p "${CMAKE_BINARY_DIR}" -- --mapping_file="${CMAKE_CURRENT_SOURCE_DIR}/sphinx.imp"
        COMMENT "Running include-what-you-use tool"
        VERBATIM
        )
endif()


# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( docs 
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)



set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_COLOR_MAKEFILE   ON)

include_directories(SYSTEM ${PROJECT_BINARY_DIR}/include)


include(ExternalProject)

ExternalProject_Add(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    INSTALL_DIR ${PROJECT_BINARY_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBuildTests=OFF
    UPDATE_COMMAND ""
    )

add_subdirectory(src/shared_lib)
add_subdirectory(src/db)
add_subdirectory(src/backend)


# set_property(TARGET SphinxDb PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
# set_property(TARGET SphinxBackend PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
# set_property(TARGET SphinxSharedLib PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})

add_test(NAME rest_api COMMAND abao ${CMAKE_SOURCE_DIR}/docs/rest_api/sphinx-backend-rest.raml --server http://localhost:9999)
