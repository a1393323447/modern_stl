cmake_minimum_required(VERSION 3.22)
project(modern_stl)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)
add_executable(modern_stl_test main.cpp src/utility/tuple.h)
add_executable(array_test test/collection_test/array_test.cpp src/iter/adapters/filter.h src/iter/utility.h)

