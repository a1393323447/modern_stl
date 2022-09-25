//
// Created by 朕与将军解战袍 on 2022/9/24.
//
#include <collection/arrary.h>
#include <iter/iterator.h>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

int main() {
    Array<i32, 10> arr = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    i32 sum = arr.into_iter() |
    fold(0, [&](i32 acc, i32 num) {
        return acc + num;
    });

    return sum;
}