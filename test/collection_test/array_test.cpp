//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#include <collection/arrary.h>
#include <iter/adapters/map.h>
#include <iter/iterator.h>
#include <iostream>

int main() {
    using namespace mstl::iter;
    using namespace mstl::collection;

    Array<int, 10> array{};

    for (mstl::usize i = 0; i < array.size(); i++) {
        array[i] = i;
    }

    auto it = array.into_iter();
    auto next = it.next();

    while (next.is_some()) {
        std::cout << next.unwrap() << " ";
        next = it.next();
    }
    std::cout << std::endl;

    auto arr_2 = collect<Array<long, 10>>(map(array.into_iter(), [](int ele) -> long {
        return ele * ele;
    }));

    auto it2 = arr_2.into_iter();
    auto next2 = it2.next();

    while (next2.is_some()) {
        std::cout << next2.unwrap() << " ";
        next2 = it2.next();
    }

    return 0;
}