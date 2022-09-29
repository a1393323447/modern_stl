cmake_minimum_required(VERSION 3.22)
project(modern_stl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")  # Debug Mode

include_directories(src)
add_executable(modern_stl_test main.cpp)

enable_testing(true)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
set(CMAKE_CXX_FLAGS "-fconcepts-diagnostics-depth=10 -Wall -Wextra -Wno-attributes --coverage")
endif()

add_subdirectory(test)