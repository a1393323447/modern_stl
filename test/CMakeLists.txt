add_executable(array_test collection_test/array_test.cpp)
target_link_libraries(array_test PRIVATE mstl)
add_executable(array_benchmark collection_test/array_benchmark.cpp)
target_link_libraries(array_benchmark PRIVATE mstl)
add_executable(fold_test iter_test/fold_test.cpp)
target_link_libraries(fold_test PRIVATE mstl)


find_package(Boost COMPONENTS unit_test_framework)

add_executable(type_list_test utility_test/type_list_test.cpp)
target_link_libraries(type_list_test PRIVATE mstl)

if (Boost_FOUND)
    # For Unit Test Frame of Boost

    add_executable(tuple_test utility_test/tuple_test.cpp)
    target_link_libraries(tuple_test PRIVATE mstl PRIVATE Boost::unit_test_framework)
    add_test(
            NAME tuple_test
            COMMAND tuple_test
    )

    add_executable(option_test option_test/option_test.cpp)
    target_link_libraries(option_test PRIVATE mstl PRIVATE Boost::unit_test_framework)
    add_test(
            NAME option_test
            COMMAND option_test
    )

    add_executable(vector_test collection_test/vector_test.cpp)
    target_link_libraries(vector_test PRIVATE mstl PRIVATE Boost::unit_test_framework)
    add_test(
            NAME vector_test
            COMMAND vector_test
    )

    add_executable(range_test range_test.cpp)
    target_link_libraries(range_test PRIVATE mstl PRIVATE Boost::unit_test_framework)
    add_test(
            NAME range_test
            COMMAND range_test
    )

    add_executable(result_test collection_test/result_test.cpp)
    target_link_libraries(result_test PRIVATE mstl PRIVATE Boost::unit_test_framework)
    add_test(
            NAME result_test
            COMMAND result_test
    )

    add_executable(list_test collection_test/list_test.cpp)
    target_link_libraries(list_test PRIVATE mstl PRIVATE Boost::unit_test_framework)
    add_test(
            NAME list_test
            COMMAND list_test
    )

    add_executable(string_test collection_test/string_test.cpp)
    target_link_libraries(string_test PRIVATE mstl PRIVATE Boost::unit_test_framework)
    add_test(
            NAME string_test
            COMMAND list_test
    )

    add_executable(match_test utility_test/match_test.cpp)
    target_link_libraries(match_test PRIVATE mstl PRIVATE Boost::unit_test_framework)
    add_test(
            NAME match_test
            COMMAND match_test
    )
endif()

find_package(benchmark)
if (benchmark_FOUND)
    link_directories(lib)

    add_executable(try_benchmark collection_test/try_benchmark.cpp)
    target_link_libraries(try_benchmark PRIVATE mstl PRIVATE benchmark::benchmark)

    add_executable(array_benchmark_2 collection_test/array_benchmark_2.cpp)
    target_link_libraries(array_benchmark_2 PRIVATE mstl PRIVATE benchmark::benchmark)

    add_executable(vector_benchmark collection_test/vector_benchmark.cpp)
    target_link_libraries(vector_benchmark PRIVATE mstl PRIVATE benchmark::benchmark init_list)

    add_executable(list_benchmark collection_test/list_benchmark.cpp)
    target_link_libraries(list_benchmark PRIVATE mstl PRIVATE benchmark::benchmark init_list)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
endif()