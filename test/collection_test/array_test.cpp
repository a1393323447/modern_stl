//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#include <collection/arrary.h>
#include <iter/iterator.h>
#include <iostream>
#include <string>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

template<typename T>
struct Pow {
    T pow;
};

void test_into_iter() {
    Array<i32, 10> array { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    Iterator auto it = array.into_iter();
    auto next = it.next();

    while (next.is_some()) {
        std::cout << next.unwrap() << " ";
        next = it.next();
    }
    std::cout << std::endl;
}

void test_map() {
    Array<i32, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto arr_2 = collect<Array<Pow<i32>, 10>>(map(
            arr.into_iter(), [](i32 ele) {
                return Pow<i32>{ ele * ele };
            }));

    auto iter = arr_2.into_iter();
    auto next = iter.next();

    while (next.is_some()) {
        std::cout << next.unwrap().pow << " ";
        next = iter.next();
    }
    std::cout << std::endl;
}

void test_filter() {
    Array<i32, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto iter = filter(arr.into_iter(), [](i32& ele) {
        return ele % 2 == 0;
    });

    auto next = iter.next();
    while (next.is_some()) {
        std::cout << next.unwrap() << " ";
        next = iter.next();
    }
    std::cout << std::endl;
}

void test_combine() {
    Array<i32, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    auto pow_arr = combine(arr.into_iter(),
        Map{}, [](auto ele) {
            return ele * ele;
        },
        Map{}, [](auto ele) {
            return Pow<usize>{ static_cast<usize>(ele * ele) };
        },
        Filter{}, [](auto& ele) {
            return ele.pow % 2 == 0;
        },
        CollectAs<Array<Pow<usize> , 5>>{}
    );

    auto iter = pow_arr.into_iter();
    auto next = iter.next();
    while (next.is_some()) {
        std::cout << next.unwrap().pow << " ";
        next = iter.next();
    }
    std::cout << std::endl;

    i32 limit = 50;
    auto first = combine(arr.into_iter(),
        Map{}, [](const auto& ele) {
            return ele * ele;
        },
        FindFirst{}, [&](const auto& ele) {
            return ele > limit;
        }
    );

    if (first.is_some()) {
        std::cout << "find " << first.unwrap() << '\n';
    } else {
        std::cout << "Can not find \n";
    }

    combine(arr.into_iter(),
        Filter{}, [](auto ele) {
            return ele % 2 == 1;
        },
        ForEach{}, [](auto ele) {
            std::cout << ele << " ";
        }
    );
    std::cout << '\n';
}

void test_combine_string() {
    using namespace std;
    Array<string, 10> arr = {
            "First", "Second", "Third", "Fourth", "Fifth",
            "Sixth", "Seventh", "Eighth", "Ninth", "Tenth"
    };

    combine(arr.iter(),
        Filter<Likely>{}, [](auto&& str) {
            return str.size() >= 3;
        },
        Map{}, [](auto&& str) {
            return str + "?";
        },
        ForEach{}, [](const auto& str) {
            std::cout << str << '\n';
        }
    );
}

void test_array_ref() {
    using namespace std;
    Array<string, 10> arr = {
            "First", "Second", "Third", "Fourth", "Fifth",
            "Sixth", "Seventh", "Eighth", "Ninth", "Tenth"
    };

    auto usize_arr = combine(arr.into_iter(),
        Map{}, [](auto&& str) {
            return str.size();
        },
        CollectAs<Array<usize, 10>>{}
    );

    combine(usize_arr.iter(),
        ForEach{}, [](const auto& size) {
            std::cout << size << " ";
        }
    );

    std::cout << '\n';
}

struct M {
    M() = default;
    M(int a): m(a) {}

    M(const M&) {
        std::cout << "Copy constructed\n";
    };
    M& operator=(const M&) {
        std::cout << "Copy assigned\n";
        return *this;
    };

    M(M&&) {
        std::cout << "Move constructed\n";
    }
    M& operator=(M&&) {
        std::cout << "Move assigned\n";
        return *this;
    }
    int m;
};


int main() {
    test_into_iter();
    test_map();
    test_filter();
    test_combine();
    test_combine_string();
    test_array_ref();

    return 0;
}