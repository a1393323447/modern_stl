//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_ITERATOR_H__
#define __MODERN_STL_ITERATOR_H__

#include "iter_concepts.h"

namespace mstl::iter {
    template<typename FromIter, typename Iter>
    requires FromIterator<FromIter, Iter>
    FromIter collect(Iter iter) {
        return FromIter::from_iter(iter);
    }
}

#endif //__MODERN_STL_ITERATOR_H__
