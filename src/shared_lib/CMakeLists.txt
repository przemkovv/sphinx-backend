project(SphinxSharedLib CXX)

include_directories(${SphinxSharedLib_SOURCE_DIR})
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_library(SphinxSharedLib ${SRC_FILES})

target_link_libraries(SphinxSharedLib PUBLIC ${Boost_LIBRARIES})
