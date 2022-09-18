//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_COMBINATOR_CONCEPTS_H__
#define __MODERN_STL_COMBINATOR_CONCEPTS_H__

#include <iter/adapters/adapter_concepts.h>

namespace mstl::iter::combinator {
    /**
     * Combinator 是一个不带模板参数的包装类, 它拥有一个模板函数, 用于获取真正的 Combinator 函数。
     * 这个类的作用主要是帮助编译器推导真正的 Combinator 函数的类型
     * @tparam C Combinator
     * @tparam Iter 迭代器是 Combinator 函数的第一个参数, 是一个迭代器
     * @tparam Lambda lambda 函数是 Combinator 函数的第二个参数, 是一个函数
     */
    template<typename C, typename Iter, typename Lambda>
    concept Combinator = requires {
        /**
         * 检查 C 是否拥有模板函数 get_combine_func
         * 注意:
         *    在做这个检查时, 就会根据 Combinator 函数中的约束
         *    对 Iter 和 Lambda 进行检查
         *    所以不需要再对 Iter 和 Lambda 进行约束检查
         */
        C::template get_combine_func<Iter, Lambda>();

        /**
         * 获取到的 Combinator 需要满足:
         *  1. 能够以 (Iter, Lambda) 为函数参数调用
         *  2. Combinator 的返回值也应该是一个迭代器
         */
        requires std::invocable<decltype(C::template get_combine_func<Iter, Lambda>()), Iter, Lambda>; // 1
        requires Iterator<
            std::invoke_result_t<
                decltype(C::template get_combine_func<Iter, Lambda>()),
                Iter,
                Lambda
            >
        >; // 2
    };
}

#endif //__MODERN_STL_COMBINATOR_CONCEPTS_H__
