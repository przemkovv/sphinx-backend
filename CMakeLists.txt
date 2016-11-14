cmake_minimum_required(VERSION 3.0)

project(Sphinx)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gdb-index")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--gdb-index")
# set(CMAKE_EXE_LINKER_FLAGS "-Wl,-cache_path_lto,${CMAKE_BINARY_DIR}/cache")

add_definitions(
  -std=c++1z
  -g
  -O1
  -gsplit-dwarf
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
  -flto=thin
  -fvisibility=hidden
  # -fsanitize=undefined,cfi,safe-stack,dataflow,address
  # -fsanitize=undefined
  )
find_library(FMT fmt libfmt)
find_package(Threads REQUIRED)
find_package(Boost 1.62 COMPONENTS system filesystem program_options)
find_package(Sqlite3 REQUIRED)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_COLOR_MAKEFILE   ON)

include_directories(SYSTEM ${PROJECT_BINARY_DIR}/include)


include(ExternalProject)

ExternalProject_Add(hh_date
    GIT_REPOSITORY https://github.com/HowardHinnant/date.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD OFF
    )

ExternalProject_Get_Property(hh_date source_dir)
set(DateIncludeDir ${source_dir})
set(HinnantDate_ROOT_DIR ${source_dir})
set(ENV{DATE_INCLUDE_DIR} ${source_dir})

include_directories(AFTER SYSTEM ${HinnantDate_ROOT_DIR})


ExternalProject_Add(sqlpp11
    GIT_REPOSITORY https://github.com/rbock/sqlpp11.git
    INSTALL_DIR ${PROJECT_BINARY_DIR}
    BUILD_COMMAND DATE_INCLUDE_DIR=${DateIncludeDir} ${MAKE} 
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DHinnantDate_ROOT_DIR=${HinnantDate_ROOT_DIR}
    LOG_DOWNLOAD OFF
    LOG_INSTALL OFF
    DEPENDS hh_date)

ExternalProject_Get_Property(sqlpp11 source_dir)
set(sqlpp11_SOURCE_DIR ${source_dir})

ExternalProject_Add(sqlpp11-connector-sqlite3
    GIT_REPOSITORY https://github.com/przemkovv/sqlpp11-connector-sqlite3.git
    INSTALL_DIR ${PROJECT_BINARY_DIR}
    BUILD_COMMAND DATE_INCLUDE_DIR=${DateIncludeDir} ${MAKE}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DDATE_INCLUDE_DIR=${HinnantDate_ROOT_DIR} -DSQLPP11_INCLUDE_DIR=${sqlpp11_SOURCE_DIR}/include
    LOG_DOWNLOAD OFF
    LOG_INSTALL OFF
    DEPENDS sqlpp11 hh_date)

# find_library(SQLPP11_SQLITE3 sqlpp11-connector-sqlite3 libsqlpp11-connector-sqlite3 ${PROJECT_BINARY_DIR}/lib)
set(SQLPP11_SQLITE3 ${PROJECT_BINARY_DIR}/lib/libsqlpp11-connector-sqlite3.a)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/db/tables.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/docs/db.sql
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/sqlite_ddl2cpp ${CMAKE_CURRENT_SOURCE_DIR}/docs/db.sql ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/db/tables Sphinx::Db::Tables
  COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/db/tables.cpp
  )

add_custom_target(
  gen_tables_header
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/db/tables.h
  )

add_subdirectory(src/shared_lib)
add_subdirectory(src/backend)

add_test(NAME rest_api COMMAND abao ${CMAKE_SOURCE_DIR}/docs/rest_api/sphinx-backend-rest.raml --server http://localhost:9999)
