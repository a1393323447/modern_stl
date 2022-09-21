//
// Created by 朕与将军解战袍 on 2022/9/20.
//

#ifndef __MODERN_STL_ARITHMETICAL_H__
#define __MODERN_STL_ARITHMETICAL_H__

namespace mstl::ops {
    template<typename I, typename Out = I>
    concept Inc = requires (I inc) {
        { inc++ } -> std::same_as<Out>;
        { ++inc } -> std::same_as<Out&>;
    };

    template<typename D, typename Out = D>
    concept Dec = requires (D dec) {
        { dec-- } -> std::same_as<Out>;
        { --dec } -> std::same_as<Out&>;
    };

    template<typename L, typename R = L, typename Out = L>
    concept Add = requires (L lhs, R rhs) {
        { lhs + rhs } -> std::same_as<Out>;
    };

    template<typename L, typename R = L, typename Out = L>
    concept Sub = requires (L lhs, R rhs) {
        { lhs - rhs } -> std::same_as<Out>;
    };
}

#endif //__MODERN_STL_ARITHMETICAL_H__
