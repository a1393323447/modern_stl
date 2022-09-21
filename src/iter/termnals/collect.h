//
// Created by 朕与将军解战袍 on 2022/9/21.
//

#ifndef __MODERN_STL_COLLECT_H__
#define __MODERN_STL_COLLECT_H__

#include "ops/callable.h"
#include "iter/termnals/terminal_concepts.h"

namespace mstl::iter {
    template<typename FromIter, typename Iter>
    requires FromIterator<FromIter, Iter>
    MSTL_INLINE
    FromIter collect(Iter iter) noexcept {
        return FromIter::from_iter(std::forward<Iter>(iter));
    }

    template<typename FromIter>
    struct CollectHolder {
    public:
        CollectHolder() = default;

        template<typename Iter>
        requires Iterator<std::remove_cvref_t<Iter>>
        MSTL_INLINE
        FromIter call(Iter&& iter) {
            return collect<FromIter, std::remove_cvref_t<Iter>>(iter);
        }
    };

    template<typename FromIter>
    MSTL_INLINE
    CollectHolder<FromIter> collect() noexcept {
        return CollectHolder<FromIter>{};
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
        static consteval CollectFunc<Container, Iter>
        get_terminal_func() noexcept {
            return collect<Container, Iter>;
        }
    };
}

#endif //__MODERN_STL_COLLECT_H__
