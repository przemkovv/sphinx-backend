
project(SphinxBackend CXX)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(SphinxBackend ${SRC_FILES})

add_dependencies(SphinxBackend json) 
target_include_directories(SphinxBackend PUBLIC ${SphinxBackend_SOURCE_DIR})
target_link_libraries(SphinxBackend LINK_PUBLIC SphinxSharedLib SphinxDb ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${FMT})
