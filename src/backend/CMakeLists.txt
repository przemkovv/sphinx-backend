
project(SphinxBackend CXX)

include_directories(${SphinxBackend})
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(SphinxBackend ${SRC_FILES})

target_link_libraries(SphinxBackend LINK_PUBLIC SphinxSharedLib ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${FMT})
