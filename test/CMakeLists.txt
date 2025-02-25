file(GLOB TEST_FILES *.cc)

add_executable(test_runner ${TEST_FILES})
target_link_libraries(test_runner core rpc_lib protobuf_lib)
