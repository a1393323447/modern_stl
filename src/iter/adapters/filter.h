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
        using Item = typename Iter::Item;

        FilterIter(Iter it, P p) noexcept : iter(it), predicate(p) { }

        MSTL_INLINE
        Option<Item> next() noexcept {
            return find(this->iter, predicate);
        }

        MSTL_INLINE
        FilterIter<Iter, P>
        into_iter() noexcept { return *this; }

    private:
        Iter iter;
        P predicate;
    };

    template<Iterator Iter, typename P, bool Likely>
    requires ops::Predicate<P, typename Iter::Item&>
    class FilterIter<Iter, P, Likely> {
    public:
        using Item = typename Iter::Item;

        FilterIter(Iter it, P p) noexcept : iter(it), predicate(p) { }

        MSTL_INLINE
        Option<Item> next() noexcept {
            return find<Iter, P, Likely>(this->iter, predicate);
        }

        MSTL_INLINE
        FilterIter<Iter, P, Likely>
        into_iter() noexcept { return *this; }

    private:
        Iter iter;
        P predicate;
    };

    template<Iterator Iter, typename P>
    FilterIter<Iter, P>
    filter(Iter iter, P predicate) noexcept {
        return FilterIter<Iter, P>{ iter, predicate };
    }

    template<Iterator Iter, typename P, bool Likely>
    FilterIter<Iter, P, Likely>
    filter(Iter iter, P predicate) noexcept {
        return FilterIter<Iter, P, Likely>{ iter, predicate };
    }

    template<Iterator Iter, typename P>
    using FilterFuncType = FilterIter<Iter, P>(*)(Iter, P);

    template<Iterator Iter, typename P, bool Likely>
    using FilterFuncWithGuessType = FilterIter<Iter, P, Likely>(*)(Iter, P);


    template<bool ...Likely>
    struct Filter;

    template<>
    struct Filter<> {
        template<Iterator Iter, typename F>
        static consteval FilterFuncType<Iter, F>
        get_combine_func() noexcept {
            return filter<Iter, F>;
        }
    };

    template<bool Likely>
    struct Filter<Likely> {
        template<Iterator Iter, typename F>
        static consteval FilterFuncWithGuessType<Iter, F, Likely>
        get_combine_func() noexcept {
            return filter<Iter, F, Likely>;
        }
    };
}

#endif //__MODERN_STL_FILTER_H__
