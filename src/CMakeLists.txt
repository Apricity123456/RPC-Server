file(GLOB SRC_FILES *.cc)
add_library(core ${SRC_FILES})
target_link_libraries(core headers protobuf_lib srpc_lib)
