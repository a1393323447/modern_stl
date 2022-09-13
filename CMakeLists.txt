cmake_minimum_required(VERSION 3.22)
project(modern_stl)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)
add_executable(modern_stl_test main.cpp src/iter/iterator.h src/iter/iter_concepts.h src/option/option.h src/iter/adapters/map.h src/ops/callable.h src/collection/arrary.h)
