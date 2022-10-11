//
// Created by 朕与将军解战袍 on 2022/9/20.
//

#ifndef __MODERN_STL_DEREFERENCE_H__
#define __MODERN_STL_DEREFERENCE_H__

#include <concepts>

namespace mstl::ops {
    template<typename P, typename D>
    concept Deref = requires (P p) {
        { *p } -> std::same_as<D>;
    };
}

#endif //__MODERN_STL_DEREFERENCE_H__
