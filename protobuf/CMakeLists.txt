find_package(Protobuf REQUIRED)

# 生成 C++ 代码
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS Signup.proto)

# 创建 protobuf_lib
add_library(protobuf_lib ${PROTO_SRCS} ${PROTO_HDRS})

# 让其他模块可以找到 `Signup.pb.h`
target_include_directories(protobuf_lib PUBLIC ${CMAKE_SOURCE_DIR}/protobuf)

# 链接 Protobuf 库
target_link_libraries(protobuf_lib protobuf::libprotobuf)
