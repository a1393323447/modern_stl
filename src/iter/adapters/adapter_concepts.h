//
// Created by 朕与将军解战袍 on 2022/9/21.
//

#ifndef __MODERN_STL_ADAPTER_HOLDER_H__
#define __MODERN_STL_ADAPTER_HOLDER_H__

#include <iter/iter_concepts.h>

namespace mstl::iter::_private {
    template<typename Holder, typename Iter>
    concept AdapterHolder = requires {
        requires Iterator<Iter>;
        requires requires(Holder holder, Iter iter) {
            { holder.to_adapter(iter) } -> Iterator;
        };
    };
}

#endif //__MODERN_STL_ADAPTER_HOLDER_H__
