//
// Created by 朕与将军解战袍 on 2022/9/15.
//
#include <iter/iterator.h>
#include <collection/array.h>

#include <string>
#include <iostream>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

using OwnIter = ArrayIter<i32, 10>;
using MapWithOwnIter = _private::MapIter<OwnIter, decltype([](i32) -> u32 { return 0; }), OwnIter::Item>;
static_assert(std::is_same_v<MapWithOwnIter::Item, u32>);

using RefIter = ArrayIterRef<i32, 10>;
using MapWithRefIter = MapIter<RefIter, decltype([](const i32&) -> u32 { return 0; }), RefIter::Item>;
static_assert(std::is_same_v<MapWithRefIter::Item, u32>);

using OwnStrIter = ArrayIter<std::string, 10>;
using MapWithOwnStrIter      = MapIter<OwnStrIter, decltype([](std::string) -> u32 { return 0; }), OwnStrIter::Item>;
using MapWithOwnStrIterByCR  = MapIter<OwnStrIter, decltype([](const std::string&) -> u32 { return 0; }), OwnStrIter::Item>;
using MapWithOwnIterByAuto   = MapIter<OwnStrIter, decltype([](auto s) -> u32 { return 0; }), OwnStrIter::Item>;
using MapWithOwnIterByARR    = MapIter<OwnStrIter, decltype([](auto&& s) -> u32 { return 0; }), OwnStrIter::Item>;
using MapWithOwnIterByCAR    = MapIter<OwnStrIter, decltype([](const auto&s) -> u32 { return 0; }), OwnStrIter::Item>;
using MapWithOwnIterByCARR   = MapIter<OwnStrIter, decltype([](const auto&&s) -> u32 { return 0; }), OwnStrIter::Item>;

using RefStrIter = ArrayIterRef<std::string, 10>;
using MapWithRefStrIter      = MapIter<RefStrIter, decltype([](std::string) -> u32 { return 0; }), RefStrIter::Item>;
using MapWithRefIterByCR     = MapIter<RefStrIter, decltype([](const std::string&) -> u32 { return 0; }), RefStrIter::Item>;
using MapWithRefIterByAuto   = MapIter<RefStrIter, decltype([](auto s) -> u32 { return 0; }), RefStrIter::Item>;
using MapWithRefIterByARR    = MapIter<RefStrIter, decltype([](auto&& s) -> u32 { return 0; }), RefStrIter::Item>;
using MapWithRefIterByCAR    = MapIter<RefStrIter, decltype([](const auto&s) -> u32 { return 0; }), RefStrIter::Item>;

struct Clone {
    Clone() = default;
    Clone(const Clone&) {
        std::cout << "Clone\n";
    }
    Clone& operator=(const Clone&) {
        std::cout << "Clone\n";
        return *this;
    }
};

int main() {
    Array<Clone, 10> arr{};

    std::cout << "iter ..\n";
    combine(arr.iter(),
        Map{}, [](auto&& c) -> const Clone& {
            return c;
        },
        ForEach{}, [](auto& c) {}
    );

    return 0;
}