cmake_minimum_required(VERSION 3.21.0)

project(cpplog)

add_library(cpplog STATIC src/cpplog.cpp)
target_include_directories(cpplog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
