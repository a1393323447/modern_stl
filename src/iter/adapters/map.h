//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_MAP_H__
#define __MODERN_STL_MAP_H__

#include <iter/iter_concepts.h>
#include <ops/callable.h>

namespace mstl::iter {
    /**
     * @tparam Iter 接受的可以转化为迭代器的类型
     * @tparam Func 转化函数
     * @tparam Arg 转化函数的参数类型
     */
    template<Iterator Iter, typename Func, typename Arg>
    requires std::is_invocable<Func, Arg>::value &&
            (!std::same_as<void, std::invoke_result_t<Func, Arg>>) // 转化后的元素类型不能为 void
    class Map {
    public:
        /// 转化后的元素类型
        using Item = std::invoke_result_t<Func, Arg>;

        Map(Iter iter, Func func) : iter(iter), func(std::move(func)) {}

        Option<Item> next() {
            auto next_item = iter.next();
            if (next_item.is_some()) {
                return func(next_item.unwrap());
            } else {
                return Option<Item>::none();
            }
        }

        Map<Iter, Func, Arg>
        into_iter() { return *this; }

    private:
        Iter iter;
        Func func;
    };

    template<Iterator Iter, typename F>
    Map<Iter, F, typename Iter::Item>
    map(Iter iter, F f) {
        return { iter, f };
    }
}

#endif //__MODERN_STL_MAP_H__
