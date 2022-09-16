//
// Created by 朕与将军解战袍 on 2022/9/14.
//

#ifndef __MODERN_STL_TERMINAL_H__
#define __MODERN_STL_TERMINAL_H__

#include <ops/callable.h>
#include <iter/terminal_concepts.h>

namespace mstl::iter {
    template<typename FromIter, typename Iter>
    requires FromIterator<FromIter, Iter>
    FromIter collect(Iter iter) noexcept {
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
        static consteval CollectFunc<Container, Iter>
        get_terminal_func() noexcept {
            return collect<Container, Iter>;
        }
    };

    template<Iterator Iter, typename P>
    requires ops::Predicate<P, typename Iter::Item&>
    Option<typename Iter::Item>
    find(Iter& iter, P predicate) noexcept {
        using Item = typename Iter::Item;
        Option<Item> next_value = iter.next();
        while (next_value.is_some()) {
            // FIXME: 这里不应该使用 unwrap 语义 -> 现在的 unwrap 语义不清晰
            auto&& item = next_value.unwrap_uncheck();
            if (predicate(item)) {
                break;
            }
            next_value = iter.next();
        }
        return next_value;
    }

    template<Iterator Iter, typename P, bool Likely>
    requires ops::Predicate<P, typename Iter::Item&>
    Option<typename Iter::Item>
    find(Iter& iter, P predicate) noexcept {
        using Item = typename Iter::Item;
        Option<Item> next_value = iter.next();
        while (next_value.is_some()) {
            // FIXME: 这里不应该使用 unwrap 语义 -> 现在的 unwrap 语义不清晰
            auto&& item = next_value.unwrap_uncheck();
            if constexpr (Likely) {
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

    template<Iterator Iter, typename F>
    requires ops::Callable<F, void, typename Iter::Item>
    void for_each(Iter iter, F lambda) noexcept {
        Option<typename Iter::Item> next = iter.next();
        while (next.is_some()) {
            lambda(next.unwrap_uncheck());
            next = iter.next();
        }
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

#endif //__MODERN_STL_TERMINAL_H__
