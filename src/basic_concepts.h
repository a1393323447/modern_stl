//
// Created by 朕与将军解战袍 on 2022/9/15.
//

#ifndef __MODERN_STL_BASIC_CONCEPTS_H__
#define __MODERN_STL_BASIC_CONCEPTS_H__

#include <concepts>

namespace mstl::basic {
    template<typename T>
    concept Movable = std::is_move_constructible_v<T> &&
                      std::is_move_assignable_v<T>;

    template<typename T>
    concept CopyAble = std::is_nothrow_copy_constructible_v<T> &&
                       std::is_copy_assignable_v<T>;

    template<typename T>
    concept RefType = std::is_reference_v<T>;

    template<typename T>
    concept LValRefType = std::is_lvalue_reference_v<T>;

    template<typename T>
    concept RValRefType = std::is_rvalue_reference_v<T>;
}

#endif //__MODERN_STL_BASIC_CONCEPTS_H__
