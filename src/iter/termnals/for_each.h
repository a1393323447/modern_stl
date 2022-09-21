//
// Created by 朕与将军解战袍 on 2022/9/21.
//

#ifndef __MODERN_STL_FOR_EACH_H__
#define __MODERN_STL_FOR_EACH_H__

#include "ops/callable.h"
#include "iter/termnals/terminal_concepts.h"

namespace mstl::iter {
    template<Iterator Iter, typename F>
    requires ops::Callable<F, void, typename Iter::Item>
    MSTL_INLINE
    void for_each(Iter iter, F lambda) noexcept {
        Option<typename Iter::Item> next = iter.next();
        while (next.is_some()) {
            lambda(next.unwrap_uncheck());
            next = iter.next();
        }
    }

    template<typename Lambda>
    class ForEachHolder {
    public:
        ForEachHolder(Lambda lambda): lambda(lambda) {}

        template<typename Iter>
        requires Iterator<std::remove_cvref_t<Iter>>
        void call(Iter&& iter) {
            for_each(iter, lambda);
        }
    private:
        Lambda lambda;
    };

    template<typename Lambda>
    MSTL_INLINE
    ForEachHolder<Lambda> for_each(Lambda lambda) noexcept {
        return CollectHolder<Lambda>{ lambda };
    }

    template<Iterator Iter, typename F>
    requires ops::Callable<F, void, typename Iter::Item>
    using ForEachFuncType = void(*)(Iter, F);

    struct ForEach {
        template<Iterator Iter, typename F>
        static consteval ForEachFuncType<Iter, F>
        get_terminal_func() noexcept {
            return for_each<Iter, F>;
        }
    };
}

#endif //__MODERN_STL_FOR_EACH_H__