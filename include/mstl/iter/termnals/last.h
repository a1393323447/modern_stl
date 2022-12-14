//
// Created by 朕与将军解战袍 on 2022/10/18.
//

#ifndef __MSTL_LAST_H__
#define __MSTL_LAST_H__

#include <mstl/iter/iter_concepts.h>

namespace mstl::iter {

    template<typename Iter>
    concept hasOwnLast = requires (Iter& iter) {
        requires Iterator<Iter>;
        { iter.last() } -> std::same_as<Option<typename Iter::Item>>;
    };

    template<Iterator Iter>
    MSTL_INLINE constexpr
    Option<typename Iter::Item>
    last(Iter& iter) {
        if constexpr (hasOwnLast<Iter>) {
            return iter.last();
        } else {
            auto prev = iter.next();
            auto current = iter.next();

            while (current.is_some()) {
                prev = std::move(current);
                current = iter.next();
            }

            return prev;
        }
    }

    struct LastHolder {
        constexpr LastHolder() { }

        template<typename Iter>
        requires Iterator<std::remove_cvref_t<Iter>>
        constexpr typename std::remove_cvref_t<Iter>::Item
        call(Iter&& iter) {
            return last(std::forward<Iter>(iter));
        }
    };

    MSTL_INLINE
    constexpr LastHolder last() {
        return LastHolder{};
    }

    template<Iterator Iter>
    using LastFuncType = typename Iter::Item(*)(Iter);

    struct Last {
        template<Iterator Iter>
        static consteval LastFuncType<Iter>
        get_terminal_func() {
            return last<Iter>;
        }
    };
}

#endif //__MSTL_LAST_H__
