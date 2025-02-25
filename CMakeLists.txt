cmake_minimum_required(VERSION 3.16)
project(RPC_Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 递归添加子目录
add_subdirectory(include)
add_subdirectory(protobuf)
add_subdirectory(rpc)
add_subdirectory(src)
add_subdirectory(test)

# 头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/protobuf)
include_directories(${CMAKE_SOURCE_DIR}/rpc)
