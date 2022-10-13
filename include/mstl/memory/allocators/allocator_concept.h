//
// Created by Shiroan on 2022/9/15.
//

#ifndef MODERN_STL_ALLOCATOR_CONCEPT_H
#define MODERN_STL_ALLOCATOR_CONCEPT_H

#include <concepts>
#include "allocator.h"

namespace mstl::memory::concepts {

    /**
     * 分配器
     * # 成员函数要求
     * - allocate(Layout layout, usize len)
     *      - 返回值要求
     *
     *          返回值类型为 void*.
     *
     *      - 功能描述
     *
     *          以layout中描述的参数, 分配一片至少能容纳len个layout所描述的类型的对象的空间. 返回所分配的空间.
     *
     * - deallocate(void* ptr, Layout layout, usize len)
     *      - 返回值要求
     *
     *          要求无返回值.
     *
     *      - 功能描述
     *
     *          以layout和len中描述的参数, 解分配ptr所指向的空间.
     */
    template<typename T, typename N=int>
    concept Allocator = basic::CopyAble<T> && ops::Eq<T, T> && requires(T a, Layout layout, usize length, void *ptr, N* ptr_){
        { a.allocate(layout, length) } -> std::same_as<void *>;
        { a.deallocate(ptr, layout, length) } -> std::same_as<void>;
        { a.template allocate<N>(length) } -> std::same_as<N*>;
        { a.template deallocate(ptr_, length) } -> std::same_as<void>;
    };

    static_assert(Allocator<mstl::memory::allocator::Allocator>);
}


#endif //MODERN_STL_ALLOCATOR_CONCEPT_H
