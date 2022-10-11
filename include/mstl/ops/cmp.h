//
// Created by 朕与将军解战袍 on 2022/9/20.
//

#ifndef __MODERN_STL_CMP_H__
#define __MODERN_STL_CMP_H__

#include <concepts>
#include <compare>

namespace mstl::ops {
    template<typename T, typename U>
    concept StrongOrd = requires (T lhs, U rhs) {
        { lhs <=> rhs } -> std::convertible_to<std::strong_ordering>;
    };

    template<typename T, typename U>
    concept WeakOrd = requires(T lhs, U rhs) {
        { lhs <=> rhs } -> std::convertible_to<std::weak_ordering>;
    };

    template<typename T, typename U>
    concept PartialOrd = requires(T lhs, U rhs) {
        { lhs <=> rhs } -> std::convertible_to<std::partial_ordering>;
    };

    template<typename T, typename U>
    concept Eq = requires(T lhs, U rhs) {
        { lhs == rhs } -> std::convertible_to<bool>;
    };
}

#endif //__MODERN_STL_CMP_H__
