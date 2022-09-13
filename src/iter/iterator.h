//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_ITERATOR_H__
#define __MODERN_STL_ITERATOR_H__

#include "iter_concepts.h"

template<FromIterator T>
T collect(typename T::IntoIter iter) {
  return T::from_iter(iter);
}

#endif //__MODERN_STL_ITERATOR_H__
