cmake_minimum_required(VERSION 3.22)
project(modern_stl)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)
add_executable(modern_stl_test main.cpp)

set(CMAKE_CXX_FLAGS -fconcepts-diagnostics-depth=10)
add_executable(array_test test/collection_test/array_test.cpp)
