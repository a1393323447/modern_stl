//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_ITER_CONCEPTS_H__
#define __MODERN_STL_ITER_CONCEPTS_H__

#include <concepts>
#include "option/option.h"

namespace mstl::iter {
    /**
     * 对迭代器类型的约束
     * @tparam Iter 迭代器类型
     */
    template<typename Iter>
    concept Iterator = requires(Iter iter) {
        /// 迭代器元素类型
        typename Iter::Item;

        { iter.next() } -> std::same_as<Option<typename Iter::Item>>;
    };

    /**
     * 对容器类型的约束
     * 满足该约束的容器都可以转化为迭代器
     * @tparam Container
     */
    template<typename Container>
    concept IntoIterator = requires {
        /// 容器的元素类型
        typename Container::Item;
        /// 转化的迭代器类型
        typename Container::IntoIter;
        /// 转化的迭代器类型也要满足迭代器约束
        Iterator<typename Container::IntoIter>;
        /// 转化的迭代器的元素类型应该与容器的元素类型相同
        std::same_as<typename Container::Item, typename Container::IntoIter::Item>;

        requires requires(Container container) {
            { container.into_iter() } -> std::same_as<typename Container::IntoIter>;
        };
    };

    /**
     *
     * @tparam Container
     */
    template<typename Container>
    concept FromIterator = requires {
        ///
        IntoIterator<Container>;

        requires requires(typename Container::IntoIter iter) {
            { Container::from_iter(iter) } -> std::same_as<Container>;
        };
    };
}

#endif //__MODERN_STL_ITER_CONCEPTS_H__
