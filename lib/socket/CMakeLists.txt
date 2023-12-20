cmake_minimum_required(VERSION 3.21.0)
project(socket)

add_library(socket STATIC src/Connection.cpp src/FileDescriptor.cpp src/Socket.cpp src/FDQueue.cpp)
target_include_directories(socket PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
