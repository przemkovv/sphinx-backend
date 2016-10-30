cmake_minimum_required(VERSION 3.0)

project(Sphinx)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_definitions(
    -std=c++1z
    -g
    -O1
    #-Wall
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    # -Wno-exit-time-destructors
    # -Wno-global-constructors
    -Wno-padded
    -Wno-weak-vtables
    # -Wno-shadow
    -Weffc++
    -Werror
    -DSPDLOG_FMT_EXTERNAL
    )
find_library(FMT fmt libfmt)
find_library(SQLPP11_SQLITE3 sqlpp11-connector-sqlite3 libsqlpp11-connector-sqlite3)
find_package(Threads REQUIRED)
find_package(Boost 1.61 COMPONENTS filesystem program_options)
find_package(Sqlite3 REQUIRED)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_COLOR_MAKEFILE   ON)

add_subdirectory(src/shared_lib)
add_subdirectory(src/backend)

add_test(NAME rest_api COMMAND abao ${CMAKE_SOURCE_DIR}/docs/rest_api/sphinx-backend-rest.raml --server http://localhost:9999)
