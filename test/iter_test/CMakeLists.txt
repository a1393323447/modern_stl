cmake_minimum_required(VERSION 3.22)
project(modern_stl)

set(CMAKE_CXX_STANDARD 20)
include_directories(../../src)

add_executable(map_test map_test.cpp)