//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_UNTILITY_H__
#define __MODERN_STL_UNTILITY_H__

#include <mstl/intrinsics.h>
#include <mstl/ops/callable.h>
#include "iter_concepts.h"
#include "termnals/terminal_concepts.h"
#include "adapters/adapter_concepts.h"
#include "adapters/combinator_concepts.h"

namespace mstl::iter {
    /**
     * combine 递归的基线函数 1: combine 返回组合后的迭代器
     */
    template<Iterator Iter, typename Com, typename Lambda>
    requires combinator::Combinator<Com, Iter, Lambda>
    MSTL_INLINE constexpr
    decltype(auto) combine(Iter iter, Com, Lambda lambda) noexcept {
        constexpr auto combinatorFunc = Com::template get_combine_func<Iter, Lambda>();

        // 不再递归调用 combine
        return combinatorFunc(iter, lambda);
    }

    /**
     * combine 递归的基线函数 2: 以 Terminal 结尾
     */
    template<Iterator Iter, typename Ter, typename... Args>
    requires terminal::Terminal<Ter, Iter, Args...>
    MSTL_INLINE constexpr
    decltype(auto) combine(Iter iter, Ter, Args... args) noexcept {
        constexpr auto terminalFunc = Ter::template get_terminal_func<Iter, Args...>();

        // 不再递归调用 combine
        return terminalFunc(iter, args...);
    }

    ///
    /// 用于将 Combinator 进行组合, 并可选地以一个 Terminal 结尾
    ///
    /// @param iter 迭代器
    /// @param Com 由 Combinator 生成的辅助结构
    /// @param lambda 对应的 lambda 函数
    ///
    /// # Example
    /// ```cpp
    /// #include <collection/array.h>
    /// #include <iter/iterator.h>
    ///
    /// using namespace mstl::collection;
    /// using namespace mstl::iter;
    ///
    /// template<typename T>
    /// struct Pow {
    ///     T pow;
    /// };
    ///
    /// Array<i32, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    ///
    /// Array<Pow<usize> , 5> pow_arr = combine(arr.into_iter(),
    ///     Map{}, [](auto ele) {
    ///         return ele * ele;
    ///     },
    ///     Map{}, [](auto ele) {
    ///         return Pow<usize>{ static_cast<usize>(ele * ele) };
    ///     },
    ///     Filter{}, [](auto& ele) {
    ///         return ele.pow % 2 == 0;
    ///     },
    ///     CollectAs<Array<Pow<usize> , 5>>{}
    /// );
    ///
    /// combine(pow_arr.iter(),
    ///     ForEach{}, [](const auto& ele) {
    ///         std::cout << ele.pow << " ";
    ///     }
    /// );
    /// ```
    ///
    template<Iterator Iter, typename Com, typename Lambda, typename... Args>
    requires combinator::Combinator<Com, Iter, Lambda>
    MSTL_INLINE constexpr
    decltype(auto)  combine(Iter iter, Com, Lambda lambda, Args... args) noexcept {
        // 通过实现了 Combinator 包装类 Com 获取真正的 combinator 函数
        constexpr auto combinatorFunc = Com::template get_combine_func<Iter, Lambda>();

        // 递归调用 combine 模拟链式调用
        return combine(combinatorFunc(iter, lambda), args...);
    }

    /// 提供 `|` 运算符, 是迭代器处理方式的抽象, 用于对 filter, map 等操作的组合
    ///
    /// # Example
    /// ```cpp
    /// #include <collection/array.h>
    /// #include <iter/iterator.h>
    ///
    /// using namespace mstl::collection;
    /// using namespace mstl::iter;
    ///
    /// template<typename T>
    /// struct Pow {
    ///     T pow;
    /// };
    /// Array<usize, 1000> arr = { ... };
    /// Array<Pow<usize>, 100> pow_arr =
    ///       arr.iter() |
    ///       map([](const auto& ele) {
    ///           return ele * ele;
    ///       }) |
    ///       map([](const auto& ele) {
    ///           return Pow<usize>{ static_cast<usize>(ele * ele) };
    ///       }) |
    ///       filter([](const auto& ele) {
    ///           return ele.pow % 2 == 0;
    ///       }) |
    ///       collect<Array<Pow<usize>, 100>>();
    /// ```
    ///
    template<Iterator Iter, typename AdapterHolder>
    requires _private::AdapterHolder<AdapterHolder, Iter>
    MSTL_INLINE constexpr
    decltype(auto) operator|(Iter iter, AdapterHolder holder) {
        return holder.to_adapter(std::move(iter));
    }

    template<Iterator Iter, typename TerminalHolder>
    requires terminal::_private::TerminalHolder<TerminalHolder, Iter>
    MSTL_INLINE constexpr
    decltype(auto) operator|(Iter iter, TerminalHolder holder) {
        return holder.call(std::move(iter));
    }
}

#endif //__MODERN_STL_UNTILITY_H__
