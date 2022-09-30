add_executable(array_test collection_test/array_test.cpp)
add_executable(array_benchmark collection_test/array_benchmark.cpp)
add_executable(fold_test iter_test/fold_test.cpp)


find_package(Boost COMPONENTS unit_test_framework)

if (Boost_FOUND)
    # For Unit Test Frame of Boost

    add_executable(tuple_test utility_test/tuple_test.cpp)
    target_link_libraries(tuple_test PRIVATE Boost::unit_test_framework)
    add_test(
            NAME tuple_test
            COMMAND tuple_test
    )

    add_executable(option_test option_test/option_test.cpp)
    target_link_libraries(option_test PRIVATE Boost::unit_test_framework)
    add_test(
            NAME option_test
            COMMAND option_test
    )

    add_executable(vector_test collection_test/vector_test.cpp)
    target_link_libraries(vector_test PRIVATE Boost::unit_test_framework)
    add_test(
            NAME vector_test
            COMMAND vector_test
    )

    add_executable(range_test range_test.cpp)
    target_link_libraries(range_test PRIVATE Boost::unit_test_framework)
    add_test(
            NAME range_test
            COMMAND range_test
    )

    add_executable(result_test collection_test/result_test.cpp)
    target_link_libraries(result_test PRIVATE Boost::unit_test_framework)
    add_test(
            NAME result_test
            COMMAND result_test
    )

    add_executable(list_test collection_test/list_test.cpp)
    target_link_libraries(list_test PRIVATE Boost::unit_test_framework)
    add_test(
            NAME list_test
            COMMAND list_test
    )
endif()

find_package(benchmark)
if (benchmark_FOUND)
    link_directories(lib)

    add_executable(try_benchmark collection_test/try_benchmark.cpp)
    target_link_libraries(try_benchmark benchmark::benchmark)

    add_executable(array_benchmark_2 collection_test/array_benchmark_2.cpp)
    target_link_libraries(array_benchmark_2 benchmark::benchmark)

    add_executable(vector_benchmark collection_test/vector_benchmark.cpp)
    target_link_libraries(vector_benchmark benchmark::benchmark init_list)

    add_executable(list_benchmark collection_test/list_benchmark.cpp)
    target_link_libraries(list_benchmark benchmark::benchmark init_list)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()