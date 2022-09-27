//
// Created by 朕与将军解战袍 on 2022/9/25.
//
#include <benchmark/benchmark.h>
#include <collection/array.h>
#include <iter/iterator.h>
#include <iostream>
#include <vector>
#include <random>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

std::vector<Array<usize, 1000>> vec{};

usize vec_1(Array<usize, 1000>& arr) {
    usize sum = arr.iter() |
    filter([](const auto& num) {
        return num % 2 == 0;
    }) |
    fold(0ull, [](usize acc, usize size) {
        return acc + size;
    });
    return sum;
}

usize vec_2(Array<usize, 1000>& arr) {
    usize sum = 0;

    for(usize i = 0; i < arr.size(); i++) {
        if (arr[i] % 2 == 0) {
            sum += arr[i];
        }
    }

    return sum;
}

void generate_test_data() {
    std::cout << "Start To Gen Test Data\n";

    std::random_device rd{};
    std::mt19937 gen{rd()};

    // 值最可能接近平均
    // 标准差影响生成的值距离平均数的分散
    std::normal_distribution<> d{10000,0 };

    for (usize i = 0; i < 10; i++) {
        Array<usize, 1000> arr{};
        for (usize j = 0; j < 1000; j++) {
            arr[j] = std::lround(d(gen));
        }
        vec.push_back(arr);
    }
    std::cout << "Gen All Test Data\n";
}

void BM_with_iter(benchmark::State& state) {
    volatile usize res = vec_1(vec[0]);
    for (auto& arr: vec) {
        res = vec_1(arr);
    }

    for (auto _: state) {
        for (auto& arr: vec) {
            res = vec_1(arr);
        }
    }
}

BENCHMARK(BM_with_iter)->UseRealTime();

void BM_with_index(benchmark::State& state) {
    volatile usize res = vec_2(vec[0]);
    for (auto& arr: vec) {
        res = vec_2(arr);
    }

    for (auto _: state) {
        for (auto& arr: vec) {
            res = vec_2(arr);
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