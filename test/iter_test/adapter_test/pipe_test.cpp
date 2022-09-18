//
// Created by 朕与将军解战袍 on 2022/9/18.
//
#include <iostream>
#include <collection/arrary.h>
#include <iter/iterator.h>
#include <iter/adapters/adapter_concepts.h>
#include <iter/adapters/flattened_adapter.h>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;
using namespace mstl::iter::adapter::_private;

using ArrayIter = array::_iter::ArrayIter<i32, 10>;
using MapAdapter = MapIter<ArrayIter, decltype([](int)->int{}), int>;

static_assert(adapter::Adapter<MapAdapter>);

using SP1 = Pipe<MapAdapter>;
static_assert(StackPipe<SP1>);
static_assert(std::same_as<
        Option<i32>,
        typename SP1::ThisPipeReturnType
>);

using SP2 = Pipe<MapAdapter, SP1>;
static_assert(StackPipe<SP2>);
static_assert(std::same_as<
        Option<i32>,
        typename SP2::ThisPipeReturnType
>);

using SP3 = Pipe<MapAdapter, SP2>;
static_assert(StackPipe<SP3>);
static_assert(std::same_as<
        Option<i32>,
        typename SP3::ThisPipeReturnType
>);

static_assert(Iterator<adapter::FlattenedAdapter<ArrayIter, SP3>>);

struct Pow {
    i32 pow;
};

int main() {
    Array<i32, 5> arr = { 1, 2, 3, 4, 5 };

    combine(arr.into_iter(),
        Map{}, [](i32 num) {
            return num + 1;
        },
        Map{}, [](i32 num) {
            return Pow { num * num };
        },
        Filter{}, [](const auto& p) {
            return p.pow > 5;
        },
        ForEach{}, [](auto num) {
            std::cout << num.pow << " ";
        }
    );

//    combine(f,
//        ForEach{}, [](auto num) {
//            std::cout << num.pow << " ";
//        }
//    );
}