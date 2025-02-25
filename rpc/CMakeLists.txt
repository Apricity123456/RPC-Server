file(GLOB SRPC_FILES *.cc)
add_library(srpc_lib ${SRPC_FILES})
target_link_libraries(srpc_lib protobuf_lib core)
