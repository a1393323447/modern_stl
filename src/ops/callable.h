//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_FUNCTION_H__
#define __MODERN_STL_FUNCTION_H__

#include <concepts>
#include <type_traits>

namespace mstl::ops {
    template<typename F, typename R, typename... Args>
    concept Callable = requires {
        requires std::invocable<F, Args...>;
        requires std::same_as<R, typename std::invoke_result<F, Args...>::type>;
    };
}

#endif //__MODERN_STL_FUNCTION_H__
