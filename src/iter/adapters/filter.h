//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_FILTER_H__
#define __MODERN_STL_FILTER_H__

#include <iter/iter_concepts.h>
#include "iter/termnals/terminals.h"

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
