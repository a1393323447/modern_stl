//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_MAP_H__
#define __MODERN_STL_MAP_H__

#include <iter/iter_concepts.h>
#include <ops/callable.h>
#include <intrinsics.h>

namespace mstl::iter {
    namespace _private {
        /**
        * @tparam Iter 接受的可以转化为迭代器的类型
        * @tparam Func 转化函数
        * @tparam Arg 转化函数的参数类型
        */
        template<Iterator Iter, typename Func>
        requires std::invocable<Func, typename Iter::Item> &&
                 (!std::same_as<void, std::invoke_result_t<Func, typename Iter::Item>>) // 转化后的元素类型不能为 void
        class MapIter {
        public:
            /// 转化后的元素类型
            using Item = std::invoke_result_t<Func, typename Iter::Item>;

            MapIter(Iter iter, Func func): iter(iter), func(func) {}

            MSTL_INLINE constexpr
            Option<Item> next() noexcept {
                auto next_item = iter.next();
                if (next_item.is_some()) {
                    Item item = func(next_item.unwrap_unchecked());
                    return Option<Item>::some(item);
                } else {
                    return Option<Item>::none();
                }
            }

            MSTL_INLINE constexpr
            MapIter<Iter, Func>
            into_iter() noexcept { return *this; }

        private:
            Iter iter;
            Func func;
        };

        template<Iterator Iter, typename F>
        MSTL_INLINE constexpr MapIter<Iter, F>
        map(Iter iter, F f) noexcept {
            return { iter, f };
        }

        template<typename Lambda>
        class MapHolder {
        public:
            MapHolder(Lambda lambda): lambda(lambda) {}

            template<typename Iter>
            MSTL_INLINE constexpr
            MapIter<Iter, Lambda>
            to_adapter(Iter iter) {
                return MapIter<Iter, Lambda>{ std::move(iter), lambda };
            }
        private:
            Lambda lambda;
        };
    }

    template<typename Lambda>
    MSTL_INLINE constexpr
    _private::MapHolder<Lambda>
    map(Lambda&& lambda) {
        return _private::MapHolder<Lambda>{ std::forward<Lambda>(lambda) };
    }

    template<Iterator Iter, typename  F>
    using MapFuncType = _private::MapIter<Iter, F>(*)(Iter, F);
    struct Map {
        template<Iterator Iter, typename F>
        static consteval MapFuncType<Iter, F>
        get_combine_func() noexcept {
            return _private::map<Iter, F>;
        }
    };
}

#endif //__MODERN_STL_MAP_H__
