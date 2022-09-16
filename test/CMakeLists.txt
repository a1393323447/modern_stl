add_executable(array_test collection_test/array_test.cpp)
add_executable(array_benchmark collection_test/array_benchmark.cpp)


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
endif()

find_package(benchmark)
if (benchmark_FOUND)
    add_executable(try_benchmark collection_test/try_benchmark.cpp)
    target_link_libraries(try_benchmark benchmark::benchmark)
endif()