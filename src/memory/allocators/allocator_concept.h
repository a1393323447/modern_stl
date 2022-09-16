//
// Created by Shiroan on 2022/9/15.
//

#ifndef MODERN_STL_ALLOCATOR_CONCEPT_H
#define MODERN_STL_ALLOCATOR_CONCEPT_H

#include <concepts>
#include <memory>

namespace mstl::memory::allocators {

    namespace _private {
        template<typename T>
        concept allocator_stl = requires(T a, usize size) {
            typename T::value_type;

            { a.allocate(size) } -> std::same_as<typename T::value_type *>;
            requires requires(typename T::value_type *p) {
                { a.deallocate(p, size) };
            };
        };

        template<typename T>
        concept allocator_mstl = requires(T a, usize size) {
            typename T::ValueType;

            { a.allocate(size) } -> std::same_as<typename T::ValueType *>;
            requires requires(typename T::ValueType *p) {
                { a.deallocate(p, size) };
            };
        };
    }

    template<typename T>
    concept Allocator = _private::allocator_mstl<T> || _private::allocator_stl<T>;


    static_assert(Allocator<std::allocator<int>>);
}


#endif //MODERN_STL_ALLOCATOR_CONCEPT_H
