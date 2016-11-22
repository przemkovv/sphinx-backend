cmake_minimum_required(VERSION 3.0)

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
add_subdirectory(src/backend)

add_test(NAME rest_api COMMAND abao ${CMAKE_SOURCE_DIR}/docs/rest_api/sphinx-backend-rest.raml --server http://localhost:9999)
