cmake_minimum_required(VERSION 3.22)
project(modern_stl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

include_directories(src)
add_executable(modern_stl_test main.cpp src/intrinsics.h)

enable_testing(true)
add_subdirectory(test)