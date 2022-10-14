//
// Created by Shiroan on 2022/10/10.
//

#ifndef MODERN_STL_TYPE_LIST_H
#define MODERN_STL_TYPE_LIST_H

#include <concepts>
#include "type_traits.h"

#define return_t return mstl::utility::type_list::TypeConstInstance
#define return_v return mstl::utility::type_list::ValueConstInstance

/**
 * 类型运算工具, 提供编译期类型运算能力.
 *
 * 可以实现类型的提取, 转换, 过滤等功能.
 *
 * ## Example
 * @code
 *          constexpr auto tuple = make_tuple(1, 2.0, 3l);
 *          constexpr auto ls1 = extract_from(tuple);       // 从类模板中提取模板参数
 *          static_assert(ls1 == TypeList<int, double, long>);
 *
 *          constexpr auto ls2 = TypeList<int, double>;
 *          constexpr auto t1 = ls2 | apply_to<Tuple>();    // 把类型列表应用到类模板中
 *          static_assert(t1 == TypeConstInstance<Tuple<int, double>>);
 *
 *          constexpr auto ls3 = ls2 | append<char, float>(); // 在类型列表后添加元素
 *          static_assert(ls3 == TypeList<int, double, char, float>);
 *
 *          constexpr auto ls4 = ls3 | prepend<double*, const char*>();  // 在类型列表前添加元素
 *          static_assert(ls4 == TypeList<double*, const char*, int, double, char, float>);
 *
 *          static_assert(ls4.at<1>() == TypeConstInstance<const char*>); // 按索引取出元素
 *
 *          constexpr auto ls5 = ls4
 *                             | filter([]<typename T>(TypeConst<T>) {  // 过滤类型元素, 如同普通元素那样
 *                                 return_v<(sizeof(T) < 8)>;           // 使用return_v返回一个编译期常量值
 *                             })
 *                             | map([]<typename T>(TypeConst<T>) {     // 转换类型元素, 如同普通元素那样
 *                                 return_t<T*>;                        // 使用return_t返回一个类型
 *                             });
 *          static_assert(ls5 == TypeList<int*, char*, float*>);
 * @endcode
 */
namespace mstl::utility::type_list {

    /**
     * 类型包装类.
     * @tparam T 欲包装的类型
     */
    template<typename T>
    struct TypeConst {
        using type = T;

        /**
         * 判断两类型是否相等.
         * @return 当且仅当T和U为同种类型时, 返回true.
         *
         * ## Example
         * @code
         *      static_assert(TypeConst<int>{} == TypeConst<i32>{});
         *      static_assert(TypeConst<int>{} != TypeConst<i64>{});
         * @endcode
         */
        template<class U>
        consteval bool operator==(const TypeConst<U>&) const {
            return std::same_as<T, U>;
        }
    };

    /**
     * 编译期常量值的包装类. 等价于`std::integral_constant<decltype(V), V>`.
     */
    template<auto V>
    using ValueConst = std::integral_constant<decltype(V), V>;

    template<typename T>
    constexpr inline auto TypeConstInstance = TypeConst<T>{};

    template<auto V>
    constexpr inline auto ValueConstInstance = ValueConst<V>{};

    template <typename T>
    concept IsTypeList = requires {
        typename T::IsTypeList;
        typename T::type;
    };

    /**
     * 类型列表. 储存一系列类型, 并承载一系列类型计算操作.
     */
    template <typename ...Types>
    struct TypeListImpl {
        struct IsTypeList{};
        using type = TypeListImpl;

        /**
         * @return 当前类型列表的元素个数
         */
        consteval static usize size() { return sizeof...(Types); }

        /**
         * 取出元素
         */
        template <usize pos>
        using At = ArgAtT<pos, Types...>;

        /**
         * 后方添加元素
         */
        template<class ...NTypes>
        using Append = TypeListImpl<Types..., NTypes...>;

        /**
         * 前方添加元素
         */
        template<class ...NTypes>
        using Prepend = TypeListImpl<NTypes..., Types...>;

        template<usize pos>
        static consteval auto at() {
            return At<pos>{};
        }

        template<class Type>
        consteval static usize count() {
            return ((usize)std::same_as<Type, typename Types::type> + ...);
        }

        /**
         * 判断两个类型列表是否相等
         */
        template<class ...OTypes>
        consteval bool operator==(const TypeListImpl<OTypes...>&) const {
            if constexpr (sizeof...(Types) != sizeof...(OTypes)) {
                return false;
            }
            else {
                return ((Types{} == OTypes{}) && ...);
            }
        }
    };

    /**
     * 创建类型列表实例. 类型将由TypeConst包装.
     * @tparam Types
     */
    template<typename ...Types>
    constexpr inline TypeListImpl<TypeConst<Types>...> TypeList{};

    template <typename ...>
    struct Append {};

    template <typename ...Types>
    consteval Append<Types...> append() {
        return {};
    }

    template<IsTypeList TL, typename ...Types>
    consteval auto operator|(const TL& , const Append<Types...>& ) -> typename TL::template Append<TypeConst<Types>...> {
        return {};
    }

    template <typename ...>
    struct Prepend{};

    template <typename ...Types>
    consteval Prepend<Types...> prepend() {
        return {};
    }

    template<IsTypeList TL, typename ...Types>
    consteval auto operator|(const TL& , const Prepend<Types...>& ) -> typename TL::template Prepend<TypeConst<Types>...> {
        return {};
    }

    template <template<class ...>typename>
    struct ApplyTo {};

    template <template<class ...>typename T>
    consteval ApplyTo<T> apply_to() {
        return {};
    }

    template<template<class ...>typename T, typename ...Types>
    consteval auto operator|(const TypeListImpl<Types...>& , const ApplyTo<T>& ) {
        return_t<T<typename Types::type...>>;
    }

    /**
     * 从类模板提取类型模板参数.
     */
    template <template<class ...>typename T, typename ...Types>
    consteval auto extract_from(const T<Types...>&) {
        return TypeList<Types...>;
    }

    /**
     * 对类型列表中的每一个元素执行变换.
     */
    template <typename F, typename ...Ts>
    consteval auto map_impl (TypeListImpl<Ts...>, F) -> TypeListImpl<std::invoke_result_t<F, Ts>...> {
        return {};
    }

    template <typename>
    struct MapHolder {};

    template<typename F>
    consteval MapHolder<F> map(F) {
        return {};
    }

    template<typename F, typename ...Types>
    consteval auto operator|(const TypeListImpl<Types...>& ls, MapHolder<F>) {
        return map_impl(ls, F{});
    }

    template<IsTypeList In, typename P, IsTypeList Out=TypeListImpl<>>
    struct Filter: Out {};

    template<typename P, IsTypeList Out, typename H, typename ...Ts>
    struct Filter<TypeListImpl<H, Ts...>, P, Out>:
    std::conditional_t<P::template value<H>,
                    Filter<TypeListImpl<Ts...>, P, typename Out::template Append<H>>,
                    Filter<TypeListImpl<Ts...>, P, Out>> {};

    template<typename F>
    struct FilterWrapper {
        template<class H>
        constexpr static bool value = std::invoke_result_t<F, H>::value;
    };

    /**
     * 滤去不满足谓词的元素
     * @tparam TL 类型列表
     * @tparam F  谓词
     */
    template<IsTypeList TL, typename F>
    consteval auto filter_impl(TL, F) {
        return Filter<TL, FilterWrapper<F>>{};
    }

    template <typename F>
    struct FilterHolder {};

    template<typename F>
    consteval FilterHolder<F> filter(F) {
        return {};
    }

    template<typename F, typename ...Types>
    consteval auto operator|(const TypeListImpl<Types...>& ls, FilterHolder<F>) {
        return filter_impl(ls, F{});
    }
}

namespace mstl::utility {
    using type_list::TypeConst;
    using type_list::TypeConstInstance;
    using type_list::ValueConst;
    using type_list::ValueConstInstance;
    using type_list::TypeList;
    using type_list::TypeListImpl;
    using type_list::operator|;
    using type_list::append;
    using type_list::prepend;
    using type_list::apply_to;
    using type_list::extract_from;
    using type_list::filter;
    using type_list::map;
    using type_list::filter_impl;
    using type_list::map_impl;
}

#endif //MODERN_STL_TYPE_LIST_H
