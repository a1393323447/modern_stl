//
// Created by Shiroan on 2022/9/28.
//

#include <benchmark/benchmark.h>

#include <random>
#include <string>
#include <initializer_list>

#include <list>
#include <forward_list>

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

template<typename L>
void assign_constrct() {
    L strs(1000000, "foo");
}

template<typename L>
void BM_assign_construct(benchmark::State& state) {
    for (auto _ : state) {
        assign_constrct<L>();
    }
}
BENCHMARK_TEMPLATE(BM_assign_construct, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_assign_construct, List<std::string>);

BENCHMARK_TEMPLATE(BM_assign_construct, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_assign_construct, ForwardList<std::string>);

template<typename L>
void range_based(const Strings& vec) {
    L strs(vec.begin(), vec.end());
}

template<typename L>
void BM_range_based(benchmark::State& state) {
    auto vec = generate_test_data();
    for (auto _ : state) {
        range_based<L>(vec);
    }
}
BENCHMARK_TEMPLATE(BM_range_based, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_range_based, List<std::string>);

BENCHMARK_TEMPLATE(BM_range_based, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_range_based, ForwardList<std::string>);

template <typename L>
void ilist() {
    L strs(list);
}

template<typename L>
void BM_ilist(benchmark::State& state) {
    for (auto _ : state) {
        ilist<L>();
    }
}

BENCHMARK_TEMPLATE(BM_ilist, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_ilist, List<std::string>);

BENCHMARK_TEMPLATE(BM_ilist, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_ilist, ForwardList<std::string>);

template <typename L>
void push_back(const Strings& vec) {
    L strs;

    for (const auto& i : vec) {
        strs.push_back(i);
    }

}

template<typename L>
void BM_push_back(benchmark::State& state) {
    Strings vec = generate_test_data();

    for (auto _ : state) {
        push_back<L>(vec);
    }
}
BENCHMARK_TEMPLATE(BM_push_back, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_push_back, List<std::string>);

template <typename L>
void push_front(const Strings& vec) {
    L strs;

    for (const auto& i : vec) {
        strs.push_front(i);
    }

}

template<typename L>
void BM_push_front(benchmark::State& state) {
    Strings vec = generate_test_data();

    for (auto _ : state) {
        push_front<L>(vec);
    }
}

BENCHMARK_TEMPLATE(BM_push_front, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_push_front, ForwardList<std::string>);

template <typename L>
void insert(const Strings& vec) {
    L str;
    std::string a = "foo";
    str.insert(str.begin(), a);
    str.insert(str.begin(), std::move(a));
    str.insert(str.begin(), 2, std::string("foo"));
    str.insert(str.begin(), {"foo", "bar"});
    str.insert(str.begin(), vec.begin(), vec.end());
}

template<typename L>
void BM_insert(benchmark::State& state) {
    Strings vec = generate_test_data();

    for (auto _ : state) {
        insert<L>(vec);
    }
}

BENCHMARK_TEMPLATE(BM_insert, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_insert, List<std::string>);

template <typename L>
void insert_after(const Strings& vec) {
    L str;
    std::string a = "foo";
    str.insert_after(str.before_begin(), a);
    str.insert_after(str.before_begin(), std::move(a));
    str.insert_after(str.before_begin(), 2, std::string("foo"));
    str.insert_after(str.before_begin(), {"foo", "bar"});
    str.insert_after(str.before_begin(), vec.begin(), vec.end());
}

template<typename L>
void BM_insert_after(benchmark::State& state) {
    Strings vec = generate_test_data();

    for (auto _ : state) {
        insert_after<L>(vec);
    }
}

BENCHMARK_TEMPLATE(BM_insert_after, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_insert_after, ForwardList<std::string>);

template<typename L>
void resize() {
    L strs;
    strs.resize(1000000, "foo");

    strs.resize(10);
}

template<typename L>
void BM_resize(benchmark::State& state) {
    for (auto _ : state) {
        resize<L>();
    }
}

BENCHMARK_TEMPLATE(BM_resize, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_resize, List<std::string>);

BENCHMARK_TEMPLATE(BM_resize, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_resize, ForwardList<std::string>);

template<typename L>
void erase(L& strs) {
    strs.erase(strs.begin(), std::next(strs.begin(), 100000));
}

template<typename L>
void BM_erase(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = generate_test_data();
        L l(vec.begin(), vec.end());
        state.ResumeTiming();
        erase<L>(l);
    }
}

BENCHMARK_TEMPLATE(BM_erase, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_erase, List<std::string>);

template<typename L>
void erase_after(L& strs) {
    strs.erase_after(strs.begin(), std::next(strs.begin(), 100000));
}

template<typename L>
void BM_erase_after(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = generate_test_data();
        L l(vec.begin(), vec.end());
        state.ResumeTiming();
        erase_after<L>(l);
    }
}
BENCHMARK_TEMPLATE(BM_erase_after, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_erase_after, ForwardList<std::string>);

template<typename L>
void sort(L& l) {
    l.sort();
}

template<typename L>
void BM_sort(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = generate_test_data();
        L l(vec.begin(), vec.end());
        state.ResumeTiming();
        sort(l);
    }
}

BENCHMARK_TEMPLATE(BM_sort, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_sort, List<std::string>);

BENCHMARK_TEMPLATE(BM_sort, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_sort, ForwardList<std::string>);

template<typename L>
void merge(L& l1, L& l2) {
    l1.merge(l2);
}

template<typename L>
void BM_merge(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = generate_test_data();
        L l1(vec.begin(), vec.begin() + 500000);
        L l2(vec.begin() + 500000, vec.end());
        l1.sort();
        l2.sort();
        state.ResumeTiming();
        merge(l1, l2);
    }
}

BENCHMARK_TEMPLATE(BM_merge, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_merge, List<std::string>);

BENCHMARK_TEMPLATE(BM_merge, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_merge, ForwardList<std::string>);

template<typename L>
void splice(L& l1, L& l2) {
    l1.splice(l1.begin(), l2);
}

template<typename L>
void BM_splice(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = generate_test_data();
        L l1(vec.begin(), vec.begin() + 500000);
        L l2(vec.begin() + 500000, vec.end());
        state.ResumeTiming();
        splice(l1, l2);
    }
}
BENCHMARK_TEMPLATE(BM_splice, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_splice, List<std::string>);

template<typename L>
void splice_after(L& l1, L& l2) {
    l1.splice_after(l1.begin(), l2);
}

template<typename L>
void BM_splice_after(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = generate_test_data();
        L l1(vec.begin(), vec.begin() + 500000);
        L l2(vec.begin() + 500000, vec.end());
        state.ResumeTiming();
        splice_after(l1, l2);
    }
}
BENCHMARK_TEMPLATE(BM_splice_after, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_splice_after, ForwardList<std::string>);

template<typename L>
void reverse(L& l1) {
    l1.reverse();
}

template<typename L>
void BM_reverse(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = generate_test_data();
        L l1(vec.begin(), vec.end());
        state.ResumeTiming();
        reverse(l1);
    }
}
BENCHMARK_TEMPLATE(BM_reverse, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_reverse, List<std::string>);

BENCHMARK_TEMPLATE(BM_reverse, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_reverse, ForwardList<std::string>);

template<typename L>
void unique(L& l1) {
    l1.unique();
}

template<typename L>
void BM_unique(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = generate_test_data();
        L l1(vec.begin(), vec.end());
        state.ResumeTiming();
        unique(l1);
    }
}
BENCHMARK_TEMPLATE(BM_unique, std::list<std::string>);
BENCHMARK_TEMPLATE(BM_unique, List<std::string>);

BENCHMARK_TEMPLATE(BM_unique, std::forward_list<std::string>);
BENCHMARK_TEMPLATE(BM_unique, ForwardList<std::string>);

BENCHMARK_MAIN();