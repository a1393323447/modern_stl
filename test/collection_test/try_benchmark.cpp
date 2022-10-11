//
// Created by 朕与将军解战袍 on 2022/9/16.
//
#include <benchmark/benchmark.h>
#include <mstl/mstl.h>
#include <iostream>
#include <vector>
#include <random>
#include <string>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

std::vector<Array<std::string, 1000>> vec{};

usize non_vec_1(Array<std::string, 1000>& arr) {
    usize total_len = 0;
    arr.iter() |
    filter([](const auto& str) {
        usize cnt = 0;
        for (auto c: str) {
            if (c >= 'A' + 13) {
                cnt++;
            }
        }
        return cnt >= 10;
    }) |
    map([](const auto& str) {
        return str.size();
    }) |
    for_each([&](usize size) {
        total_len += size;
    });
    return total_len;
}

usize non_vec_2(Array<std::string, 1000>& arr) {
    usize total_len = 0;

    for(usize i = 0; i < arr.size(); i++) {
        usize cnt = 0;
        for (const auto& c: arr[i]) {
            if (c == 'A' + 13) {
                cnt++;
            }
        }

        if (cnt >= 10) {
            total_len += arr[i].size();
        }
    }

    return total_len;
}

void generate_test_data() {
    std::cout << "Start To Gen Test Data\n";

    std::random_device rd{};
    std::mt19937 gen{rd()};

    // 值最可能接近平均
    // 标准差影响生成的值距离平均数的分散
    std::normal_distribution<> d{13,0 };

    for (usize i = 0; i < 10; i++) {
        Array<std::string, 1000> arr{};
        for (usize j = 0; j < 1000; j++) {
            std::string str{};
            for (usize z = 0; z < 50; z++) {
                str.push_back(std::lround(d(gen)) % 26 + 'A');
            }
            arr[j] = str;
        }
        vec.push_back(arr);
    }
    std::cout << "Gen All Test Data\n";
}

void BM_with_iter(benchmark::State& state) {
    volatile usize res = non_vec_1(vec[0]);
    for (auto& arr: vec) {
        res = non_vec_1(arr);
    }

    for (auto _: state) {
        for (auto& arr: vec) {
            res = non_vec_1(arr);
        }
    }
}

BENCHMARK(BM_with_iter)->UseRealTime();

void BM_with_index(benchmark::State& state) {
    volatile usize res = non_vec_2(vec[0]);
    for (auto& arr: vec) {
        res = non_vec_2(arr);
    }

    for (auto _: state) {
        for (auto& arr: vec) {
            res = non_vec_2(arr);
        }
    }
}

BENCHMARK(BM_with_index)->UseRealTime();

int main(int argc, char** argv) {
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;

    generate_test_data();

    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}