//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_ITERATOR_H__
#define __MODERN_STL_ITERATOR_H__

#include <ops/callable.h>
#include <iter/utility.h>
#include <iter/adapters/map.h>
#include <iter/iter_concepts.h>
#include <iter/adapters/filter.h>
#include <iter/adapters/combinator_concepts.h>

namespace mstl::iter {
    /**
     * combine 递归的基线函数
     */
    template<Iterator Iter, typename Com, typename Lambda>
    requires combinator::Combinator<Com, Iter, Lambda>
    decltype(auto) combine(Iter iter, Com, Lambda lambda) {
        auto combinatorFunc = Com::template get_combine_func<Iter, Lambda>();

        // 不再递归调用 combine
        return combinatorFunc(iter, lambda);
    }

    /**
     * 用于将 iter::adapters 进行组合, 返回组合后的迭代器
     *
     * <h1>Example</h1>
     * @code
     * Array<i32, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
     * auto iter = combine(arr.into_iter(),
     *     Map, [](auto ele) {
     *         return ele * ele;
     *     },
     *     Map, [](auto ele) {
     *         return Pow<usize>{ static_cast<usize>(ele * ele) };
     *     }
     * );
     * auto next = iter.next();
     * while (next.is_some()) {
     *     std::cout << next.unwrap().pow << " ";
     *     next = iter.next();
     * }
     * @endcode
     */
    template<Iterator Iter, typename Com, typename Lambda, typename... Args>
    requires combinator::Combinator<Com, Iter, Lambda>
    decltype(auto) combine(Iter iter, Com, Lambda lambda, Args... args) {
        // 通过实现了 Combinator 包装类 Com 获取真正的 combinator 函数
        auto combinatorFunc = Com::template get_combine_func<Iter, Lambda>();

        // 递归调用 combine 模拟链式调用
        return combine(combinatorFunc(iter, lambda), args...);
    }
}
#endif //__MODERN_STL_ITERATOR_H__
