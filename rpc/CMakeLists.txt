find_package(Protobuf REQUIRED)

# 生成 Protobuf C++ 代码
protobuf_generate_cpp(RPC_PROTO_SRCS RPC_PROTO_HDRS rpc.proto)

# 避免重复创建 rpc_lib
if(NOT TARGET rpc_lib)
    add_library(rpc_lib ${RPC_PROTO_SRCS} rpc.pb.cc)

    # 链接到 Protobuf 库
    target_link_libraries(rpc_lib protobuf_lib protobuf::libprotobuf)

    # 确保 `rpc.srpc.h` 可以被 `src/` 访问
    target_include_directories(rpc_lib PUBLIC ${CMAKE_SOURCE_DIR}/include ${Protobuf_INCLUDE_DIRS})
endif()
