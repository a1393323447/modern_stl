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
    requires std::invocable<Func, Arg> &&
            (!std::same_as<void, std::invoke_result_t<Func, Arg>>) // 转化后的元素类型不能为 void
    class MapIter {
    public:
        /// 转化后的元素类型
        using Item = std::invoke_result_t<Func, Arg>;

        MapIter(Iter iter, Func func) : iter(iter), func(func) {}

        Option<Item> next() noexcept {
            auto next_item = iter.next();
            if (next_item.is_some()) {
                // FIXME: 现在的 unwrap 语义不明确
                Item item = func(next_item.unwrap_uncheck());
                return Option<Item>::some(item);
            } else {
                return Option<Item>::none();
            }
        }

        MapIter<Iter, Func, Arg>
        into_iter() noexcept { return *this; }

    private:
        Iter iter;
        Func func;
    };

    template<Iterator Iter, typename F>
    MapIter<Iter, F, typename Iter::Item>
    map(Iter iter, F f) noexcept {
        return { iter, f };
    }

    template<Iterator Iter, typename  F>
    using MapFuncType = MapIter<Iter, F, typename Iter::Item>(*)(Iter, F);
    struct Map {
        template<Iterator Iter, typename F>
        static constexpr MapFuncType<Iter, F>
        get_combine_func() noexcept {
            return map<Iter, F>;
        }
    };
}

#endif //__MODERN_STL_MAP_H__
