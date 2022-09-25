//
// Created by 朕与将军解战袍 on 2022/9/21.
//

#ifndef __MODERN_STL_FIND_H__
#define __MODERN_STL_FIND_H__

#include "ops/callable.h"
#include "iter/termnals/terminal_concepts.h"

namespace mstl::iter {
    template<Iterator Iter, typename P>
    requires ops::Predicate<P, typename Iter::Item&>
    MSTL_INLINE constexpr
    Option<typename Iter::Item>
    find(Iter& iter, P predicate) noexcept {
        using Item = typename Iter::Item;
        Option<Item> next_value = iter.next();
        while (next_value.is_some()) {
            auto&& item = next_value.as_ref_uncheck();
            if (predicate(item)) {
                break;
            }
            next_value = iter.next();
        }
        return next_value;
    }

    template<Iterator Iter, typename P, bool Predict>
    requires ops::Predicate<P, typename Iter::Item&>
    MSTL_INLINE constexpr
    Option<typename Iter::Item>
    find(Iter& iter, P predicate) noexcept {
        using Item = typename Iter::Item;
        Option<Item> next_value = iter.next();
        while (next_value.is_some()) {
            auto&& item = next_value.as_ref_uncheck();
            if constexpr (Predict) {
                if (predicate(item)) [[likely]] {
                    break;
                }
            } else {
                if (predicate(item)) [[unlikely]] {
                    break;
                }
            }
            next_value = iter.next();
        }
        return next_value;
    }

    template<typename Lambda>
    class FindHolder {
    public:
        FindHolder(Lambda lambda): lambda(lambda) {}

        template<typename Iter>
        requires Iterator<std::remove_cvref_t<Iter>>
        MSTL_INLINE constexpr
        Option<typename std::remove_cvref_t<Iter>::Item>
        call(Iter&& iter) {
            return find(iter, lambda);
        }

    private:
        Lambda lambda;
    };

    template<typename Lambda>
    MSTL_INLINE constexpr
    FindHolder<Lambda> find(Lambda lambda) {
        return FindHolder<Lambda>{ lambda };
    }

    template<Iterator Iter, typename P>
            requires ops::Predicate<P, typename Iter::Item&>
    using FindFuncType = Option<typename Iter::Item>(*)(Iter&, P);

    struct FindFirst {
        template<Iterator Iter, typename P>
        static consteval FindFuncType<Iter, P>
        get_terminal_func() noexcept {
            return find<Iter, P>;
        }
    };
}

#endif //__MODERN_STL_FIND_H__
