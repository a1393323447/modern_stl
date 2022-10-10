//
// Created by Shiroan on 2022/10/10.
//

#ifndef MODERN_STL_TYPE_LIST_H
#define MODERN_STL_TYPE_LIST_H

#include "type_traits.h"

#define return_t return mstl::utility::type_list::TypeConstInstance
#define return_v return mstl::utility::type_list::ValueConstInstance

namespace mstl::utility::type_list {

    template<typename T>
    struct TypeConst {
        using type = T;

        template<class U>
        consteval bool operator==(const TypeConst<U>&) const {
            return std::same_as<T, U>;
        }
    };

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

    template <typename ...Types>
    struct TypeListImpl {
        struct IsTypeList{};
        using type = TypeListImpl;

        consteval static usize size() { return sizeof...(Types); }

        template <usize pos>
        using At = ArgAtT<pos, Types...>;

        template<class ...NTypes>
        using Append = TypeListImpl<Types..., NTypes...>;

        template<class ...NTypes>
        using Prepend = TypeListImpl<NTypes..., Types...>;

        template<usize pos>
        consteval auto at() const {
            return At<pos>{};
        }

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

    template <template<class ...>typename T, typename ...Types>
    consteval auto extract_from(const T<Types...>&) {
        return TypeList<Types...>;
    }

    template <typename F, typename ...Ts>
    constexpr auto map_impl (TypeListImpl<Ts...>, F) -> TypeListImpl<std::invoke_result_t<F, Ts>...> {
        return {};
    };

    template <typename F>
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
