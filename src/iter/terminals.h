//
// Created by 朕与将军解战袍 on 2022/9/14.
//

#ifndef __MODERN_STL_TERMINAL_H__
#define __MODERN_STL_TERMINAL_H__

#include <iter/terminal_concepts.h>

namespace mstl::iter {
    template<typename FromIter, typename Iter>
    requires FromIterator<FromIter, Iter>
    FromIter collect(Iter iter) {
        return FromIter::from_iter(iter);
    }

    /**
     * 用于帮助编译器推断, 提供更好的错误信息
     */
    template<typename FromIter, typename Iter>
            requires FromIterator<FromIter, Iter>
    using CollectFunc = FromIter(*)(Iter);
    template<typename Container>
    struct CollectAs {
        template<typename Iter>
        static CollectFunc<Container, Iter>
        get_terminal_func() {
            return collect<Container, Iter>;
        }
    };

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

    template<Iterator Iter, typename P>
    requires ops::Predicate<P, typename Iter::Item&>
    using FindFuncType = Option<typename Iter::Item>(*)(Iter&, P);
    struct FindFirst {
        template<Iterator Iter, typename P>
        static FindFuncType<Iter, P>
        get_terminal_func() {
            return find<Iter, P>;
        }
    };

}

#endif //__MODERN_STL_TERMINAL_H__
