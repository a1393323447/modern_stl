//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_MAP_H__
#define __MODERN_STL_MAP_H__

#include <ops/callable.h>
#include <iter/iter_concepts.h>
#include <iter/adapters/adapter_concepts.h>

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
        /// For concept Iterator [Begin]
        /// 转化后的元素类型
        using Item = std::invoke_result_t<Func, Arg>;
        /// [End]

        /// For concept Adapter [Begin]
        using UnderlyingIter = Iter;
        using Lambda = Func;
        using LambdaTakeItemType = typename UnderlyingIter::Item;
        using ProcessResultType = Item;
        static constexpr adapter::InfluenceType
        thisInfluenceType = adapter::InfluenceType::NoEffect;
        /// [End]

        MapIter(Iter iter, Func func) : iter(std::forward<Iter>(iter)), func(func) {}

        /// For concept Iterator [Begin]
        MSTL_INLINE
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
        /// [End]

        /// For concept IntoIter [Begin]
        MSTL_INLINE
        MapIter<Iter, Func, Arg>
        into_iter() noexcept { return *this; }
        /// [End]

        /// For concept Adapter [Begin]
        MSTL_INLINE
        constexpr
        UnderlyingIter get_underlying_iter() {
            return std::move(iter);
        }

        MSTL_INLINE
        constexpr
        Lambda get_lambda() {
            return std::move(func);
        }

        MSTL_INLINE
        static adapter::ProcessResult<ProcessResultType>
        process(typename UnderlyingIter::Item item, const Lambda& lambda) {
            ProcessResultType ret = lambda(item);
            return adapter::ProcessResult<ProcessResultType>{
                adapter::ControlFlow::Continue,
                ret
            };
        }
        /// [End]

    private:
        Iter iter;
        Func func;
    };

    template<Iterator Iter, typename F>
    MSTL_INLINE
    MapIter<Iter, F, typename Iter::Item>
    map(Iter iter, F f) noexcept {
        return {std::forward<Iter>(iter), f };
    }

    template<Iterator Iter, typename  F>
    using MapFuncType = MapIter<Iter, F, typename Iter::Item>(*)(Iter, F);
    struct Map {
        template<Iterator Iter, typename F>
        static consteval MapFuncType<Iter, F>
        get_combine_func() noexcept {
            return map<Iter, F>;
        }
    };

    static_assert(adapter::Adapter<MapIter<_private::TestIter<int>, decltype([](int) -> int { return 0; }), int>>);
}

#endif //__MODERN_STL_MAP_H__
