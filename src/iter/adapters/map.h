//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_MAP_H__
#define __MODERN_STL_MAP_H__

#include <iter/iter_concepts.h>
#include <functional>
#include "ops/callable.h"

template<Iterator Iter, typename Func, typename ReturnType>
requires Callable<Func, ReturnType, typename Iter::Item>
class Map {
public:
    using Item = ReturnType;

    Map(Iter iter, Func func): iter(iter), func(std::move(func)) {}

    Option<Item> next() {
        auto next_item = iter.next();
        if (next_item.is_some()) {
            return func(next_item.unwrap());
        } else {
            return { None };
        }
    }

private:
    Iter iter;
    Func func;
};

template<Iterator Iter, typename R, typename ...Args>
Map<Iter, std::function<R(Args...)>, R>
map(Iter iter, std::function<R(Args...)> f) {
    return { iter, f };
}

#endif //__MODERN_STL_MAP_H__
