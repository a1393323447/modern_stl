//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_FILTER_H__
#define __MODERN_STL_FILTER_H__

#include <iter/iter_concepts.h>
#include <iter/utility.h>

namespace mstl::iter {
    template<Iterator Iter, typename P>
    requires ops::Predicate<P, typename Iter::Item&>
    class FilterIter {
    public:
        using Item = typename Iter::Item;

        FilterIter(Iter it, P p): iter(it), predicate(p) { }

        Option<Item> next() {
            return find(this->iter, predicate);
        }

        FilterIter<Iter, P>
        into_iter() { return *this; }

    private:
        Iter iter;
        P predicate;
    };

    template<Iterator Iter, typename P>
    FilterIter<Iter, P>
    filter(Iter iter, P predicate) {
        return { iter, predicate };
    }

    struct FilterCombinator {
        template<Iterator Iter, typename F>
        static auto
        get_combine_func() {
            return filter<Iter, F>;
        }
    } Filter;
}

#endif //__MODERN_STL_FILTER_H__
