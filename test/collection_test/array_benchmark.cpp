//
// Created by 朕与将军解战袍 on 2022/9/14.
//
#include <collection/arrary.h>
#include <iter/iterator.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <string>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

template<typename T>
struct Pow {
    T pow;
};

Array<Pow<usize>, 100> combine_for_ben_vec_1(Array<i32, 1000> arr) {
    auto pow_arr = combine(arr.iter(),
        Map{}, [](auto&& ele) {
            return ele * ele;
        },
        Map{}, [](const auto&& ele) {
            return Pow<usize>{ static_cast<usize>(ele * ele) };
        },
        Filter{}, [](auto&& ele) {
            return ele.pow % 2 == 0;
        },
        CollectAs<Array<Pow<usize>, 100>>{}
    );

    return pow_arr;
}

Array<Pow<usize>, 100> combine_for_ben_vec_2(Array<i32, 1000> arr) {
    for (usize i = 0; i < arr.size(); i++) {
        arr[i] = arr[i] * arr[i];
    }

    Array<Pow<usize>, arr.size()> arr2{};
    for (usize i = 0; i < arr2.size(); i++) {
        arr2[i] = Pow<usize>{ static_cast<usize>(arr[i] * arr[i]) };
    }

    Array<Pow<usize>, 100> arr3{};
    usize pos = 0;
    for (usize i = 0; i < arr3.size(); i++) {
        if (arr2[i].pow % 2 == 0) {
            arr3[pos] = arr2[i];
            pos++;
        }
    }

    return arr3;
}

void test_vec_1(auto&& vec) {
    std::cout << "1. warming...\n";
    Array<Pow<usize>, 100> res{};
    for (auto&& arr: vec) {
        res = combine_for_ben_vec_1(arr);
    }
    std::cout << res[0].pow << " Done\n";


    auto now = std::chrono::steady_clock::now();
    for (auto arr: vec) {
        res = combine_for_ben_vec_1(arr);
    }
    auto after = std::chrono::steady_clock::now();
    auto time = after - now;
    std::cout << "1. Benchmark done with result " << res[2].pow << '\n';
    std::cout << "1. time: " << time << "\n";
}

void test_vec_2(auto&& vec) {
    std::cout << "2. warming...\n";
    auto res = combine_for_ben_vec_2(vec[0]);
    for (auto arr: vec) {
        res = combine_for_ben_vec_2(arr);
    }
    std::cout << res[0].pow << " Done\n";


    auto now = std::chrono::steady_clock::now();
    for (auto arr: vec) {
        res = combine_for_ben_vec_2(arr);
    }
    auto after = std::chrono::steady_clock::now();
    auto time = after - now;
    std::cout << "2. Benchmark done with result " << res[2].pow << '\n';
    std::cout << "2. time: " << time << "\n";
}

void test_vectorize() {
    std::cout << "Testing Vectorize ...\n";
    std::random_device rd{};
    std::mt19937 gen{rd()};

    // 值最可能接近平均
    // 标准差影响生成的值距离平均数的分散
    std::normal_distribution<> d{20,5 };

    // 生成数据
    std::vector<Array<i32, 1000>> vec{};
    for (usize i = 0; i < 10000; i++) {
        auto array = Array<i32, 1000>{};
        for (usize j = 0; j < 1000; j++) {
            array[j] = std::lround(d(gen));
        }
        vec.push_back(array);
    }

    test_vec_1(vec);

    test_vec_2(vec);

    std::cout << "Testing Vectorize Done ...\n\n";
}

usize non_vec_1(Array<std::string, 1000> arr) {
    usize total_len = 0;
    combine(arr.iter(),
        Filter{}, [](auto&& str) {
            usize cnt = 0;
            for (auto c: str) {
                if (c == 'K') {
                    cnt++;
                }
            }
            return cnt > 3;
        },
        Map{}, [](auto&& str) {
            return str.size();
        },
        ForEach{}, [&](usize size) {
            total_len += size;
        }
    );

    return total_len;
}

usize non_vec_2(Array<std::string, 1000> arr) {
    usize total_len = 0;

    for(usize i = 0; i < arr.size(); i++) {
        usize cnt = 0;
        for (auto c: arr[i]) {
            if (c == 'K') {
                cnt++;
            }
        }

        if (cnt > 3) {
            total_len += arr[i].size();
        }
    }

    return total_len;
}

void test_non_vec_1(std::vector<Array<std::string, 1000>>& vec) {
    std::cout << "1. warming...\n";
    auto res = non_vec_1(vec[0]);
    for (const auto& arr: vec) {
        res = non_vec_1(arr);
    }
    std::cout << res << " Done\n";


    auto now = std::chrono::steady_clock::now();
    for (const auto& arr: vec) {
        res = non_vec_1(arr);
    }
    auto after = std::chrono::steady_clock::now();
    auto time = after - now;
    std::cout << "1. Benchmark done with result " << res << '\n';
    std::cout << "1. time: " << time << "\n";
}

void test_non_vec_2(std::vector<Array<std::string, 1000>>& vec) {
    std::cout << "2. warming...\n";
    auto res = non_vec_2(vec[0]);
    for (const auto& arr: vec) {
        res = non_vec_2(arr);
    }
    std::cout << res << " Done\n";


    auto now = std::chrono::steady_clock::now();
    for (const auto& arr: vec) {
        res = non_vec_2(arr);
    }
    auto after = std::chrono::steady_clock::now();
    auto time = after - now;
    std::cout << "2. Benchmark done with result " << res << '\n';
    std::cout << "2. time: " << time << "\n";
}

void test_non_vectorize() {
    std::cout << "Testing Non-Vectorize ...\n";
    std::random_device rd{};
    std::mt19937 gen{rd()};

    // 值最可能接近平均
    // 标准差影响生成的值距离平均数的分散
    std::normal_distribution<> d{5,5 };

    std::vector<Array<std::string, 1000>> vec{};
    for (usize i = 0; i < 1000; i++) {
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

    test_non_vec_1(vec);
    test_non_vec_2(vec);

    std::cout << "Testing Non-Vectorize Done\n";
}

int main() {
    test_vectorize();

    test_non_vectorize();

    return 0;
}