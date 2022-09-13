//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#include <collection/arrary.h>
#include <iter/iterator.h>
#include <iostream>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

template<typename T>
struct Pow {
    T pow;
};

void test_into_iter() {
    Array<i32, 10> array { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto it = array.into_iter();
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
    Array<int, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
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

int main() {
    test_into_iter();
    test_map();
    test_filter();
    return 0;
}