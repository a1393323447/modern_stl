//
// Created by Shiroan on 2022/9/18.
//

#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <initializer_list>

#include <benchmark/benchmark.h>
#include <mstl.h>

extern std::initializer_list<std::string> list;

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

using Strings = std::vector<std::string>;

Strings generate_test_data() {
    Strings vec;
    vec.reserve(1000000);
    std::random_device rd{};
    std::mt19937 gen{rd()};

    // 采用均匀分布, 值分布于8位有符号数取值范围内
    std::uniform_int_distribution<> d{-256, +255};

    for (usize i = 0; i < 1000000; i++) {
        std::string str{};
        for (usize z = 0; z < 20; z++) {
            str.push_back((i8) d(gen));
        }

        vec.push_back(str);
    }
    return vec;
}

template<typename Vec>
void default_construct() {
    Vec strs(1000000, "foo");
}

template<typename Vec>
void BM_default_construct(benchmark::State& state) {
    for (auto _ : state) {
        default_construct<Vec>();
    }
}

BENCHMARK_TEMPLATE(BM_default_construct, std::vector<std::string>);
BENCHMARK_TEMPLATE(BM_default_construct, Vector<std::string>);

template<typename Vec>
void range_based(const Strings& vec) {
    Vec strs(vec.begin(), vec.end());
}

template<typename Vec>
void BM_range_based(benchmark::State& state) {
    auto vec = generate_test_data();
    for (auto _ : state) {
        range_based<Vec>(vec);
    }
}

BENCHMARK_TEMPLATE(BM_range_based, std::vector<std::string>);
BENCHMARK_TEMPLATE(BM_range_based, Vector<std::string>);

template <typename Vec>
void ilist() {
    Vec strs(list);
}

template<typename Vec>
void BM_ilist(benchmark::State& state) {
    for (auto _ : state) {
        ilist<Vec>();
    }
}

BENCHMARK_TEMPLATE(BM_ilist, std::vector<std::string>);
BENCHMARK_TEMPLATE(BM_ilist, Vector<std::string>);

template <typename Vec>
void push_back(const Strings& vec) {
    Vec strs;

    for (const auto& i : vec) {
        strs.push_back(i);
    }

}

template<typename Vec>
void BM_push_back(benchmark::State& state) {
    Strings vec = generate_test_data();

    for (auto _ : state) {
        push_back<Vec>(vec);
    }
}

BENCHMARK_TEMPLATE(BM_push_back, std::vector<std::string>);
BENCHMARK_TEMPLATE(BM_push_back, Vector<std::string>);

template <typename Vec>
void insert(const Strings& vec) {
    Vec str;
    std::string a = "foo";
    str.insert(str.begin(), a);
    str.insert(str.begin(), std::move(a));
    str.insert(str.begin(), 2, std::string("foo"));
    str.insert(str.begin(), {"foo", "bar"});
    str.insert(str.begin(), vec.begin(), vec.end());
}

template<typename Vec>
void BM_insert(benchmark::State& state) {
    Strings vec = generate_test_data();

    for (auto _ : state) {
        insert<Vec>(vec);
    }
}

BENCHMARK_TEMPLATE(BM_insert, std::vector<std::string>);
BENCHMARK_TEMPLATE(BM_insert, Vector<std::string>);

template<typename Vec>
void resize() {
    Vec strs;
    strs.resize(1000000, "foo");

    strs.resize(10);
}

template<typename Vec>
void BM_resize(benchmark::State& state) {
    for (auto _ : state) {
        resize<Vec>();
    }
}
BENCHMARK_TEMPLATE(BM_resize, std::vector<std::string>);
BENCHMARK_TEMPLATE(BM_resize, Vector<std::string>);

template<typename Vec>
void reserve() {
    Vec strs;
    strs.reserve(1000000);
}

template<typename Vec>
void BM_reserve(benchmark::State& state) {
    for (auto _ : state) {
        reserve<Vec>();
    }
}
BENCHMARK_TEMPLATE(BM_reserve, std::vector<std::string>);
BENCHMARK_TEMPLATE(BM_reserve, Vector<std::string>);

template<typename Vec>
void erase(const Strings& vec) {
    Vec strs;

    for (const auto& i : vec) {
        strs.push_back(i);
    }

    strs.erase(strs.begin(), strs.begin() + 100000);
}

template<typename Vec>
void BM_erase(benchmark::State& state) {
    auto vec = generate_test_data();
    for (auto _ : state) {
        erase<Vec>(vec);
    }
}
BENCHMARK_TEMPLATE(BM_erase, std::vector<std::string>);
BENCHMARK_TEMPLATE(BM_erase, Vector<std::string>);

BENCHMARK_MAIN();