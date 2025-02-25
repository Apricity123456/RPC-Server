file(GLOB PROTO_FILES *.proto)
find_package(Protobuf REQUIRED)

foreach(PROTO ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO} NAME_WE)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO})
    add_library(protobuf_${PROTO_NAME} ${PROTO_SRCS} ${PROTO_HDRS})
    target_link_libraries(protobuf_${PROTO_NAME} protobuf::libprotobuf)
endforeach()

add_library(protobuf_lib ALIAS protobuf_signup)
