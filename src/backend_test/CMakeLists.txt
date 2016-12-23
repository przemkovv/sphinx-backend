

project(SphinxBackendTest CXX)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(SphinxBackendTest ${SRC_FILES})

add_dependencies(SphinxBackendTest SphinxSharedLib json) 
target_link_libraries(SphinxBackendTest LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${RESTClient})

add_test(NAME backend_test COMMAND SphinxBackendTest)
