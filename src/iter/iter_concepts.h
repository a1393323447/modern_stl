//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_ITER_CONCEPTS_H__
#define __MODERN_STL_ITER_CONCEPTS_H__

#include <concepts>
#include <option/option.h>

namespace mstl::iter {
    /**
     * @tparam Iter 迭代器类型
     */
    template<typename Iter>
    concept Iterator = requires(Iter iter) {
        /**
         * The type of the elements being iterated over
         */
        typename Iter::Item;

        /**
         * <h1>Require</h1>
         * 以 Option<Item>::some(Item) 的形式返回迭代器当前指向的元素, 并将迭代器前移.
         * 如果迭代结束, 则返回 Option<Item>::none().
         *
         * <h1>Example</h1>
         * @code
         * using namespace mstl::collection;
         * using namespace mstl;
         *
         * auto arr  = Array<i32, 3>{1, 2, 3};
         * auto iter = arr.iter();
         *
         * auto value = iter.next();
         * assert(value.is_some());
         * assert(value.unwrap() == 1);
         *
         * value = iter.next();
         * assert(value.is_some());
         * assert(value.unwrap() == 2);
         *
         * value = iter.next();
         * assert(value.is_some());
         * assert(value.unwrap() == 4);
         *
         * value = iter.next();
         * assert(value.is_none());
         * @endcode
         */
        { iter.next() } -> std::same_as<Option<typename Iter::Item>>;
    };

    /**
     * @tparam Into 可以转化为 Iterator 的类型
     */
    template<typename Into>
    concept IntoIterator = requires {
        /**
         * 迭代的元素类型
         */
        typename Into::Item;

        /**
         * <h1>Require</h1>
         * 要求 Into 类型具有成员函数 into_iter
         * 返回一个 Iterator , Iterator::Item 应和 Into::Item 相同
         */
        requires requires(Into into) {
            { into.into_iter() } ->  Iterator;
            requires std::same_as<
                 typename decltype(into.into_iter())::Item,
                 typename Into::Item
            >;
        };
    };

    /**
     * @tparam FromIter 可以从一个迭代器 Iter 构建的类型
     * @tparam Iter 一个迭代器
     */
    template<typename FromIter, typename Iter>
    concept FromIterator = requires {
        requires IntoIterator<FromIter>;
        requires Iterator<Iter>;

        /**
         * <h1>Require</h1>
         * 要求 FromIter 拥有静态函数 from_iter
         * 接受一个迭代器 Iter iter 返回构建得到的 FromIter
         */
        requires requires(Iter iter) {
            { FromIter::from_iter(iter) } -> std::same_as<FromIter>;
        };
    };
}

#endif //__MODERN_STL_ITER_CONCEPTS_H__
