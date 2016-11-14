
project(SphinxBackend CXX)

include_directories(${SphinxSharedLib_SOURCE_DIR})
include_directories(${SphinxBackend_SOURCE_DIR})

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(SphinxBackend ${SRC_FILES})

add_dependencies(SphinxBackend gen_tables_header sqlpp11 sqlpp11-connector-sqlite3) 
target_link_libraries(SphinxBackend LINK_PUBLIC SphinxSharedLib ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${FMT} ${SQLPP11_SQLITE3} ${SQLITE3_LIBRARIES})
