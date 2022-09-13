//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_UNTILITY_H__
#define __MODERN_STL_UNTILITY_H__

#include <ops/callable.h>
#include <iter/iter_concepts.h>

namespace mstl::iter {
    template<typename FromIter, typename Iter>
    requires FromIterator<FromIter, Iter>
    FromIter collect(Iter iter) {
        return FromIter::from_iter(iter);
    }

    template<Iterator Iter, typename P>
    requires ops::Predicate<P, typename Iter::Item&>
    Option<typename Iter::Item>
    find(Iter& iter, P predicate) {
        using Item = typename Iter::Item;
        Option<Item> next_value = iter.next();
        while (next_value.is_some()) {
            // FIXME: 这里不应该使用 unwrap 语义 -> 现在的 unwrap 语义不清晰
            Item item = next_value.unwrap();
            if (predicate(item)) {
                break;
            }
            next_value = iter.next();
        }
        return next_value;
    }
}

#endif //__MODERN_STL_UNTILITY_H__
