//
// Created by Shiroan on 2022/9/13.
//

#ifndef MODERN_STL_TYPE_TRAITS_H
#define MODERN_STL_TYPE_TRAITS_H

#include "global.h"

namespace mstl::utility {
    template <usize pos, typename ...Ts>
    struct ArgAt {
        static_assert(sizeof...(Ts) != 0, "Parameter pack 'Ts' is empty.");
    };

    template <usize pos, typename T, typename ...Ts>
    struct ArgAt<pos, T, Ts...> {
        using type = typename ArgAt<pos - 1, Ts...>::type;
    };

    template <typename T, typename ...Ts>
    struct ArgAt<0, T, Ts...> {
        using type = T;
    };

    template <usize pos, typename ...Ts>
    using ArgAtT = typename ArgAt<pos, Ts...>::type;

    template <bool C, usize True, usize False>
    struct Condition;

    template<usize True, usize False>
    struct Condition<true, True, False> {
        static constexpr usize value = True;
    };

    template<usize True, usize False>
    struct Condition<false, True, False> {
        static constexpr usize value = False;
    };

    template<bool C, usize True, usize False>
    constexpr usize ConditionV = Condition<C, True, False>::value;
}
#endif //MODERN_STL_TYPE_TRAITS_H
