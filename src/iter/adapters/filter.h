//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_FILTER_H__
#define __MODERN_STL_FILTER_H__

#include <iter/iter_concepts.h>
#include <iter/terminals.h>

namespace mstl::iter {

    constexpr bool Likely = true;
    constexpr bool Unlikely = false;

    template<Iterator Iter, typename P, bool ...args>
    requires ops::Predicate<P, typename Iter::Item&>
    class FilterIter {};

    template<Iterator Iter, typename P>
    requires ops::Predicate<P, typename Iter::Item&>
    class FilterIter<Iter, P> {
    public:
        /// For concept Iterator [Begin]
        using Item = typename Iter::Item;
        /// [End]

        /// For concept Adapter [Begin]
        using UnderlyingIter = Iter;
        using Lambda = P;
        using LambdaTakeItemType = const typename UnderlyingIter::Item&;
        using ProcessResultType = Item;
        static constexpr adapter::InfluenceType
                thisInfluenceType = adapter::InfluenceType::Influence;
        /// [End]

        FilterIter(Iter it, P p) noexcept : iter(it), predicate(p) { }

        /// For concept Iterator [Begin]
        MSTL_INLINE
        Option<Item> next() noexcept {
            return find(this->iter, predicate);
        }
        /// [End]

        /// For concept IntoIter [Begin]
        FilterIter<Iter, P>
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
            return std::move(predicate);
        }

        MSTL_INLINE
        static adapter::ProcessResult<ProcessResultType>
        process(typename UnderlyingIter::Item item, const Lambda& lambda) {
            bool check = lambda(item);

            using adapter::ControlFlow;
            ControlFlow flow = check ? ControlFlow::Continue : ControlFlow::Break;

            return adapter::ProcessResult<ProcessResultType>{ flow ,item };
        }
        /// [End]

    private:
        Iter iter;
        P predicate;
    };

    template<Iterator Iter, typename P, bool Predict>
    requires ops::Predicate<P, typename Iter::Item&>
    class FilterIter<Iter, P, Predict> {
    public:
        /// For concept Iterator [Begin]
        using Item = typename Iter::Item;
        /// [End]

        /// For concept Adapter [Begin]
        using UnderlyingIter = Iter;
        using Lambda = P;
        using LambdaTakeItemType = typename UnderlyingIter::Item&;
        using ProcessResultType = Item;
        static constexpr adapter::InfluenceType
                thisInfluenceType = adapter::InfluenceType::Influence;
        /// [End]

        FilterIter(Iter it, P p) noexcept : iter(it), predicate(p) { }

        /// For concept Iterator [Begin]
        MSTL_INLINE
        Option<Item> next() noexcept {
            return find<Iter, P, Predict>(this->iter, predicate);
        }
        /// [End]

        /// For concept IntoIter [Begin]
        MSTL_INLINE
        FilterIter<Iter, P, Predict>
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
            return std::move(predicate);
        }

        MSTL_INLINE
        static adapter::ProcessResult<ProcessResultType>
        process(typename UnderlyingIter::Item item, const Lambda& lambda) {
            bool check = lambda(item);

            using adapter::ControlFlow;
            ControlFlow flow = check ? ControlFlow::Continue : ControlFlow::Break;

            return adapter::ProcessResult<ProcessResultType>{ flow ,item };
        }
        /// [End]

    private:
        Iter iter;
        P predicate;
    };

    template<Iterator Iter, typename P>
    MSTL_INLINE
    FilterIter<Iter, P>
    filter(Iter iter, P predicate) noexcept {
        return FilterIter<Iter, P>{ std::forward<Iter>(iter), predicate };
    }

    template<Iterator Iter, typename P, bool Predict>
    MSTL_INLINE
    FilterIter<Iter, P, Predict>
    filter(Iter iter, P predicate) noexcept {
        return FilterIter<Iter, P, Predict>{ std::forward<Iter>(iter), predicate };
    }

    template<Iterator Iter, typename P>
    using FilterFuncType = FilterIter<Iter, P>(*)(Iter, P);

    template<Iterator Iter, typename P, bool Predict>
    using FilterFuncWithGuessType = FilterIter<Iter, P, Predict>(*)(Iter, P);


    template<bool ...Predict>
    struct Filter {
        template<typename T>
        struct Protected {
            static const bool value = false;
        };
        static_assert(
             Protected<void>::value,
             "Too many prediction for Filter\n"
        );
    };

    template<>
    struct Filter<> {
        template<Iterator Iter, typename F>
        static consteval FilterFuncType<Iter, F>
        get_combine_func() noexcept {
            return filter<Iter, F>;
        }
    };

    template<bool Predict>
    struct Filter<Predict> {
        template<Iterator Iter, typename F>
        static consteval FilterFuncWithGuessType<Iter, F, Predict>
        get_combine_func() noexcept {
            return filter<Iter, F, Predict>;
        }
    };
}

#endif //__MODERN_STL_FILTER_H__
