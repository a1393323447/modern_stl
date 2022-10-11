//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_FILTER_H__
#define __MODERN_STL_FILTER_H__

#include <mstl/iter/iter_concepts.h>
#include <mstl/iter/termnals/terminals.h>

namespace mstl::iter {
    namespace _private {
        template<Iterator Iter, typename P, bool ...Predict>
        requires ops::Predicate<P, typename Iter::Item&>
        class FilterIter {
            static_assert(
                (Predict && ... && false),
                "Too many prediction for Filter\n"
            );
        };

        template<Iterator Iter, typename P>
        requires ops::Predicate<P, typename Iter::Item&>
        class FilterIter<Iter, P> {
        public:
            using Item = typename Iter::Item;

            FilterIter(Iter it, P p) noexcept : iter(it), predicate(p) { }

            MSTL_INLINE constexpr
            Option<Item> next() noexcept {
                return find(this->iter, predicate);
            }

            MSTL_INLINE constexpr
            FilterIter<Iter, P>
            into_iter() noexcept { return *this; }

        private:
            Iter iter;
            P predicate;
        };

        template<Iterator Iter, typename P, bool Predict>
        requires ops::Predicate<P, typename Iter::Item&>
        class FilterIter<Iter, P, Predict> {
        public:
            using Item = typename Iter::Item;

            FilterIter(Iter it, P p) noexcept : iter(it), predicate(p) { }

            MSTL_INLINE constexpr
            Option<Item> next() noexcept {
                return find<Iter, P, Predict>(this->iter, predicate);
            }

            MSTL_INLINE constexpr
            FilterIter<Iter, P, Predict>
            into_iter() noexcept { return *this; }

        private:
            Iter iter;
            P predicate;
        };

        template<typename Lambda, bool... Predict>
        class FilterHolder {
            static_assert(
                (Predict && ... && false),
                "Too many prediction for Filter\n"
            );
        };

        template<typename Lambda, bool Predict>
        class FilterHolder<Lambda, Predict> {
        public:
            FilterHolder(Lambda lambda): lambda(lambda) {}

            template<typename Iter>
            MSTL_INLINE constexpr
            FilterIter<Iter, Lambda, Predict>
            to_adapter(Iter iter) {
                return FilterIter<Iter, Lambda, Predict>{ std::move(iter), std::move(lambda) };
            }
        private:
            Lambda lambda;
        };

        template<typename Lambda>
        class FilterHolder<Lambda> {
        public:
            FilterHolder(Lambda lambda): lambda(lambda) {}

            template<typename Iter>
            MSTL_INLINE constexpr
            FilterIter<Iter, Lambda>
            to_adapter(Iter iter) {
                return FilterIter<Iter, Lambda>{ std::move(iter), std::move(lambda) };
            }
        private:
            Lambda lambda;
        };

        template<Iterator Iter, typename P>
        MSTL_INLINE constexpr
        FilterIter<Iter, P>
        filter(Iter iter, P predicate) noexcept {
            return FilterIter<Iter, P>{ iter, predicate };
        }

        template<Iterator Iter, typename P, bool Predict>
        MSTL_INLINE constexpr
        FilterIter<Iter, P, Predict>
        filter(Iter iter, P predicate) noexcept {
            return FilterIter<Iter, P, Predict>{ iter, predicate };
        }
    }

    constexpr bool Likely = true;
    constexpr bool Unlikely = false;


    template<typename Lambda>
    MSTL_INLINE constexpr
    _private::FilterHolder<Lambda>
    filter(Lambda&& lambda) {
        return _private::FilterHolder<Lambda>{ std::forward<Lambda>(lambda) };
    }

    template<bool Predict, typename Lambda>
    MSTL_INLINE constexpr
    _private::FilterHolder<Lambda, Predict>
    filter(Lambda&& lambda) {
        return _private::FilterHolder<Lambda, Predict>{ std::forward<Lambda>(lambda) };
    }

    template<Iterator Iter, typename P>
    using FilterFuncType = _private::FilterIter<Iter, P>(*)(Iter, P);

    template<Iterator Iter, typename P, bool Predict>
    using FilterFuncWithGuessType = _private::FilterIter<Iter, P, Predict>(*)(Iter, P);

    template<bool ...Predict>
    struct Filter {
        static_assert(
            (Predict && ... && false),
            "Too many prediction for Filter\n"
        );
    };

    template<>
    struct Filter<> {
        template<Iterator Iter, typename F>
        static consteval FilterFuncType<Iter, F>
        get_combine_func() noexcept {
            return _private::filter<Iter, F>;
        }
    };

    /**
     * filter 用于过滤迭代器中的元素, 筛选出符合给定条件的元素
     *
     * Filter 是用于与 mstl::iter::combine 函数结合使用, 实现 filter 功能的结构体
     *
     * # Example
     *
     * ## Basic usage
     * ```cpp
     * #include <collection/array.h>
     * #include <iter/iterator.h>
     * using namespace mstl::collection;
     * using namespace mstl::iter;
     *
     * Array<i32, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
     * auto arr_iter = combine(arr.into_iter(),
     *     Filter{}, [](i32 ele) {
     *         return ele % 2 == 0;
     *     }
     * );
     *
     * Option<i32> next = arr_iter.next();
     * while (next.is_some()) {
     *     std::cout << next.unwrap_unchecked();
     * }
     * ```
     *
     * ## Using Predict
     * Filter 结构体提供了 Predict 参数, 可选值为 Likely 和 Unlikely ,
     * 用于实现对 [[likely]] 和 [[unlikely]] 属性的使用。
     *
     * ```cpp
     * #include <collection/array.h>
     * #include <iter/iterator.h>
     * using namespace mstl::collection;
     * using namespace mstl::iter;
     *
     * Array<i32, 10> arr { 1, 3, 5, 7, 9, 11, 13, 15, 17, 20 };
     * auto arr_iter = combine(arr.into_iter(),
     *     Filter<Unlikely>{}, [](i32 ele) {
     *         return ele % 2 == 0;
     *     },
     *     ForEach{}, [](i32 ele) {
     *          ...
     *     }
     * );
     * ```
     *
     * 以上代码等价于
     * ```cpp
     * Array<i32, 10> arr { 1, 3, 5, 7, 9, 11, 13, 15, 17, 20 };
     * auto arr_iter = arr.into_iter();
     *
     * Option<i32> next = arr_iter.next();
     * while (next.is_some()) {
     *     if (lambda()) [[unlikely]] {
     *         ...
     *     }
     * }
     * ```
     */
    template<bool Predict>
    struct Filter<Predict> {
        template<Iterator Iter, typename F>
        static consteval FilterFuncWithGuessType<Iter, F, Predict>
        get_combine_func() noexcept {
            return _private::filter<Iter, F, Predict>;
        }
    };
}

#endif //__MODERN_STL_FILTER_H__
