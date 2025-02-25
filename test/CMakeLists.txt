file(GLOB TEST_FILES *.cc)
add_executable(test_runner ${TEST_FILES})
target_link_libraries(test_runner core srpc_lib protobuf_lib)
