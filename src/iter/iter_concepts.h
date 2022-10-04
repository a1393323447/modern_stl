//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_ITER_CONCEPTS_H__
#define __MODERN_STL_ITER_CONCEPTS_H__

#include <concepts>
#include <option/option.h>

namespace mstl::iter {
    /**
     * Rust 风格的迭代器
     * # typename 标注要求
     * - Item: 迭代器中需要使用 Item 标注迭代器所迭代的元素
     *      ```cpp
     *      class FooIter {
     *      public:
     *          using Item = Foo;
     *          ...
     *      };
     *      ```
     * # 成员函数要求
     * - next()
     *      - 返回值要求
     *
     *          返回值类型为 Option<Item>
     *
     *      - 功能描述
     *
     *          返回以 some(Item) 的形式返回下一个元素, 如果没有元素, 则返回 none()
     *
     *      示例
     *
     *      ```cpp
     *      mstl::collection::Array<i32, 3> arr = { 1, 2, 3 };
     *      Iterator auto iter = arr.iter();
     *      Option<const i32&> one =  iter.next();
     *      auto two = iter.next();
     *      auto three = iter.next();
     *      auto none = iter.next();
     *      ```
     */
    template<typename Iter>
    concept Iterator = requires(Iter iter) {
        typename Iter::Item;
        { iter.next() } -> std::same_as<Option<typename Iter::Item>>;
    };

    /**
     * 双向迭代器
     * # 前提
     * DoubleEndedIterator 需要实现 Iterator
     * # 成员函数要求
     * - prev()
     *      - 返回值要求
     *
     *          返回值类型为 Option<Item>
     *
     *      - 功能描述
     *
     *          返回以 some(Item) 的形式返回逆向迭代的下一个元素, 如果没有元素, 则返回 none()
     *
     *      示例
     *
     *      ```cpp
     *      mstl::collection::Array<i32, 3> arr = { 1, 2, 3 };
     *      Iterator auto iter = arr.iter();
     *      Option<const i32&> three =  iter.prev();
     *      auto two = iter.prev();
     *      auto one = iter.prev();
     *      auto none = iter.prev();
     *      ```
     */
    template<typename Iter>
    concept DoubleEndedIterator = requires {
        requires Iterator<Iter>;
        requires requires(Iter iter) {
            { iter.prev() } -> std::same_as<Option<typename Iter::Item>>;
        };
    };

    /**
     * 可以获得元素数量的迭代器
     * # 前提
     * DoubleEndedIterator 需要实现 Iterator
     * # 成员函数要求
     * - len()
     *      - 返回值要求
     *
     *          返回值类型为 usize
     *
     *      - 功能描述
     *
     *          返回迭代器当前剩余的元素数量
     *
     *      示例
     *
     *      ```cpp
     *      mstl::collection::Array<i32, 3> arr = { 1, 2, 3 };
     *      Iterator auto iter = arr.iter();
     *      usize three =  iter.len();
     *      ```
     * - is_empty()
     *      - 返回值要求
     *
     *          返回值类型为 bool
     *
     *      - 功能描述
     *
     *          返回迭代器是否为空
     *
     *      示例
     *
     *      ```cpp
     *      mstl::collection::Array<i32, 3> arr = { 1, 2, 3 };
     *      Iterator auto iter = arr.iter();
     *      bool False =  iter.is_empty();
     *      ```
     */
    template<typename Iter>
    concept ExactSizeIterator = requires {
        requires Iterator<Iter>;
        requires requires(Iter iter) {
            { iter.len() } -> std::same_as<usize>;
            { iter.is_empty() } -> std::same_as<bool>;
        };
    };

    /**
     * 迭代元素存储在连续空间的迭代器
     * # 前提
     * DoubleEndedIterator 需要实现 Iterator
     * # 成员函数要求
     * - start_addr()
     *      - 返回值要求
     *
     *          返回值类型为 const T* [T = std::remove_reference_t<Item>]
     *
     *      - 功能描述
     *
     *          返回迭代器下一个元素所在的地址
     *
     *      示例
     *
     *      ```cpp
     *      mstl::collection::Array<i32, 3> arr = { 1, 2, 3 };
     *      Iterator auto iter = arr.iter();
     *
     *      const usize *ptr_1 =  iter.start_addr();
     *      assert(ptr_1 == &arr[0]);
     *
     *      iter.next();
     *
     *      const usize *ptr_2 = iter.start_addr();
     *      assert(ptr_2 == &arr[1]);
     *      ```
     */
    template<typename Iter>
    concept ContinuousIterator = requires {
        requires ExactSizeIterator<Iter>;
        requires requires(Iter iter) {
            { iter.start_addr() } -> std::same_as<const std::remove_reference_t<typename Iter::Item> *>;
        };
    };

    /**
     * IntoIterator 描述了:
     * - 一个类型(T)如何转换为迭代器
     * - 转换为哪种迭代器
     *
     * # typename 标注要求
     * - Item
     *
     *      类型 T 中持有的元素类型
     *
     * # 成员函数要求
     * - into_iter()
     *      - 返回值要求
     *
     *          返回值类型为一个迭代器, 并且迭代元素类型为 Item
     *
     *      - 功能描述
     *
     *          将类型 T 转换为迭代器
     *
     *      示例
     *
     *      ```cpp
     *      mstl::collection::Array<i32, 3> arr = { 1, 2, 3 };
     *      Iterator auto iter = arr.iter();
     *
     *      Option<i32> one = iter.next();
     *      ```
     */
    template<typename Into>
    concept IntoIterator = requires {
        typename Into::Item;
        requires requires(Into into) {
            { into.into_iter() } ->  Iterator;
            requires std::same_as<
                 typename decltype(into.into_iter())::Item,
                 typename Into::Item
            >;
        };
    };

    /**
     * 实现了 FromIterator 的类型 T (一般是一个容器), 可以将一个符合要求 Iterator 转化 T
     *
     * # 前提
     * - 类型 T 需要实现 IntoIterator
     * - 被转化的 Iterator 所迭代的元素类型和 T 的元素类型相同
     *
     * # 静态函数要求
     * - from_iter(Iterator auto iter)
     *      - 函数参数要求
     *          - iter: 要求实现 Iterator [Item = T::Item]
     *
     *      - 返回值要求
     *
     *          返回值类型为 T
     *
     *      - 功能描述
     *
     *          将迭代器转换为类型 T , 得到的 T 中包含迭代器所持有的元素
     *
     *      示例
     *
     *      基础用法
     *      ```cpp
     *      mstl::collection::Array<i32, 3> arr = { 1, 2, 3 };
     *      Iterator auto iter = arr.into_iter();
     *
     *      auto arr_2 = mstl::collection::Array<i32, 3>::from_iter(iter);
     *      ```
     *
     *      关联用法
     *      ```cpp
     *      mstl::collection::Array<i32, 3> arr = { 1, 2, 3 };
     *
     *      // 1. 使用 mstl::iter::combine
     *      auto arr_2 = mstl::iter::combine(arr.into_iter(),
     *          mstl::iter::CollectAs<mstl::collection::Array<i32, 3>>{}
     *      );
     *
     *      // 2. 使用 mstl::iter::operator|
     *      using namespace mstl::iter;
     *      auto arr_2 = arr.into_iter() |
     *          collect<mstl::collection::Array<i32, 3>>();
     *
     *      // 3. 直接使用 mstl::iter::collect
     *      auto arr_2 = mstl::iter::collect(arr.into_iter());
     *      ```
     */
    template<typename FromIter, typename Iter>
    concept FromIterator = requires {
        requires IntoIterator<FromIter>;
        requires Iterator<Iter>;
        requires std::same_as<
            typename FromIter::Item,
            std::remove_reference_t<typename Iter::Item>
        >;

        /**
         * <h1>Require</h1>
         * 要求 FromIter 拥有静态函数 from_iter
         * 接受一个迭代器 Iter iter 返回构建得到的 FromIter
         */
        requires requires(Iter iter) {
            { FromIter::from_iter(std::forward<Iter>(iter)) } -> std::same_as<FromIter>;
        };
    };

    /**
     * 同C++标准之具名要求"老式输入迭代器 (LegacyInputIterator)".
     * <br>
     * 要求类型可解引用、前缀自增、后缀自增，且可相等比较.
     * */
    template<typename It>
    concept LegacyInputIterator = requires(It i, It j) {
        {i == j} -> std::same_as<bool>;
        *i;
        ++i;
        i++;
    };
}

#endif //__MODERN_STL_ITER_CONCEPTS_H__
