//
// Created by 朕与将军解战袍 on 2022/9/27.
//

#ifndef __MODERN_STL_REDUCE_H__
#define __MODERN_STL_REDUCE_H__

#include "../../ops/callable.h"
#include "../iter_concepts.h"
#include "fold.h"

namespace mstl::iter {

    template<Iterator Iter, typename Lambda>
    requires ops::Callable<Lambda, typename Iter::Item, typename Iter::Item, typename Iter::Item>
    constexpr
    Option<typename Iter::Item>
    reduce(Iter iter, Lambda lambda) {
        using ReturnType = Option<typename Iter::Item>;
        auto first = iter.next();
        if (first.is_some()) [[likely]] {
            return ReturnType::some(fold(iter, first.unwrap_unchecked(), lambda));
        } else {
            return ReturnType::none();
        }
    }

    template<typename Lambda>
    class ReduceHolder {
    public:
         ReduceHolder(Lambda lambda): lambda(std::move(lambda)) {};

        template<typename Iter>
        requires Iterator<std::remove_cvref_t<Iter>>
        constexpr
        Option<typename std::remove_cvref_t<Iter>::Item>
        call(Iter&& iter) {
            return reduce(std::forward<Iter>(iter),  std::move(lambda));
        }
    private:
         Lambda lambda;
    };

    template<typename Lambda>
    MSTL_INLINE constexpr ReduceHolder<Lambda>
    reduce(Lambda lambda) {
        return ReduceHolder<Lambda>{ std::move(lambda) };
    }

    template<Iterator Iter, typename Lambda>
    requires ops::Callable<Lambda, typename Iter::Item, typename Iter::Item, typename Iter::Item>
    using ReduceFuncType = typename Iter::Item(*)(Iter, Lambda);

    struct Reduce {
        template<Iterator Iter, typename Lambda>
        static consteval ReduceFuncType<Iter, Lambda>
        get_terminal_func() {
            return fold<Iter, Lambda>;
        }
    };
}

#endif //__MODERN_STL_REDUCE_H__
