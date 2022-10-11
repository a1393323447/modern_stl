//
// Created by 朕与将军解战袍 on 2022/9/15.
//

#ifndef __MODERN_STL_BASIC_CONCEPTS_H__
#define __MODERN_STL_BASIC_CONCEPTS_H__

#include <concepts>
#include <ostream>

namespace mstl::basic {
    template<typename T>
    concept Movable = std::movable<T>;

    template<typename T>
    concept CopyAble = std::copyable<T>;

    template<typename T>
    concept RefType = std::is_reference_v<T>;

    template<typename T>
    concept LValRefType = std::is_lvalue_reference_v<T>;

    template<typename T>
    concept RValRefType = std::is_rvalue_reference_v<T>;

    template<typename T>
    concept Printable = requires (std::ostream& out, const T& val){
        out << val;
    };

    template<typename T>
    concept Error = Movable<T> && CopyAble<T> && Printable<T> && !std::is_reference_v<T>;

    // True, if and only if In is one of T and Ts...
    template<typename In, typename T, typename ...Ts>
    concept OneOf = std::same_as<In, T> || (std::same_as<In, Ts> || ...);

    template<typename P>
    concept PtrType = std::is_pointer_v<P>;
}

#endif //__MODERN_STL_BASIC_CONCEPTS_H__
