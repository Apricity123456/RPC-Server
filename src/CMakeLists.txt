file(GLOB SRC_FILES *.cc)

add_library(core ${SRC_FILES})

target_link_libraries(core protobuf_lib rpc_lib)
