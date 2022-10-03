//
// Created by Shiroan on 2022/9/15.
//

#ifndef MODERN_STL_ALLOCATOR_CONCEPT_H
#define MODERN_STL_ALLOCATOR_CONCEPT_H

#include <concepts>
#include "allocator.h"

namespace mstl::memory::concepts {

    template<typename T>
    concept Allocator = basic::CopyAble<T> && requires(T a, Layout layout, usize length, void *ptr){
        { a.allocate(layout, length) } -> std::same_as<void *>;
        { a.deallocate(ptr, layout, length) } -> std::same_as<void>;
    };

    static_assert(Allocator<mstl::memory::allocator::Allocator>);
}


#endif //MODERN_STL_ALLOCATOR_CONCEPT_H
