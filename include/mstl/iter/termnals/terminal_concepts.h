//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_TERMINAL_CONCEPTS_H__
#define __MODERN_STL_TERMINAL_CONCEPTS_H__

#include "../iter_concepts.h"

namespace mstl::iter::terminal {

    namespace _private {
        template<typename TerHolder, typename Iter>
        concept TerminalHolder = requires {
            requires Iterator<Iter>;
            requires requires (TerHolder holder, Iter iter) {
                { holder.call(iter) };
            };
        };
    }

    template<typename Ter, typename Iter, typename... Args>
    concept Terminal = requires {
        requires Iterator<Iter>;

        Ter::template get_terminal_func<Iter, Args...>();
        requires std::is_invocable_v<decltype(Ter::template get_terminal_func<Iter, Args...>()), Iter, Args...>  ||
                 std::is_invocable_v<decltype(Ter::template get_terminal_func<Iter, Args...>()), Iter&, Args...> ||
                 std::is_invocable_v<decltype(Ter::template get_terminal_func<Iter, Args...>()), Iter&&, Args...>;
    };
}

#endif //__MODERN_STL_TERMINAL_CONCEPTS_H__
