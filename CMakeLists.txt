cmake_minimum_required(VERSION 3.16)
project(ServerProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加子目录
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(protobuf)
add_subdirectory(srpc)
add_subdirectory(test)
