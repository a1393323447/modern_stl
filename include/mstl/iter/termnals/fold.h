//
// Created by 朕与将军解战袍 on 2022/9/24.
//

#ifndef __MODERN_STL_FOLD_H__
#define __MODERN_STL_FOLD_H__

#include <mstl/ops/callable.h>
#include <mstl/iter/iter_concepts.h>

namespace mstl::iter {

    template<Iterator Iter, typename T, typename Lambda>
    requires ops::Callable<Lambda, T, T, typename Iter::Item>
    constexpr
    T fold(Iter iter, T init, Lambda lambda) {
        auto next = iter.next();
        while (next.is_some()) {
            init = std::move(lambda(
                    std::move(init),
                    next.unwrap_unchecked()
            ));
            next = iter.next();
        }

        return init;
    }

    template<typename T, typename Lambda>
    class FoldHolder {
    public:
        FoldHolder(T init, Lambda lambda):
        init(std::move(init)), lambda(std::move(lambda))
        { }

        template<typename Iter>
        requires Iterator<std::remove_cvref_t<Iter>>
        constexpr T call(Iter&& iter) {
            return fold(std::forward<Iter>(iter), std::move(init), std::move(lambda));
        }
    private:
        T init;
        Lambda lambda;
    };

    template<typename T, typename Lambda>
    MSTL_INLINE constexpr FoldHolder<T, Lambda>
    fold(T init, Lambda lambda) {
        return FoldHolder<T, Lambda>{ std::move(init), std::move(lambda) };
    }

    template<Iterator Iter, typename T, typename Lambda>
    requires ops::Callable<Lambda, T, T, typename Iter::Item>
    using FoldFuncType = T(*)(Iter, T, Lambda);

    struct Fold {
        template<Iterator Iter, typename T, typename Lambda>
        static consteval FoldFuncType<Iter, T, Lambda>
        get_terminal_func() {
            return fold<Iter, T, Lambda>;
        }
    };
}

#endif //__MODERN_STL_FOLD_H__
