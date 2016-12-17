
project(SphinxDb CXX)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_library(SphinxDb ${SRC_FILES})

# target_include_directories(SphinxDb PUBLIC ${SphinxDb_SOURCE_DIR})
target_link_libraries(SphinxDb LINK_PUBLIC SphinxSharedLib   ${FMT} ${PostgreSQL_LIBRARIES})
