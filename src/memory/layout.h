//
// Created by Shiroan on 2022/9/22.
//

#ifndef MODERN_STL_LAYOUT_H
#define MODERN_STL_LAYOUT_H

#include <basic_concepts.h>
#include <result/result.h>
#include <global.h>
#include <bit>
#include "layout_error.h"

namespace mstl::memory {
    struct Layout {
        usize size;
        usize align;

        template<class T>
        requires (!basic::RefType<T>)
        constexpr static Layout from_type() {
            return {sizeof(T), alignof(T)};
        }

        static mstl::result::Result<Layout, LayoutError> from_size_align(usize size, usize align) {
            if (align == 0 || std::popcount(align) != 1) {
                return {LayoutError{}};
            } else {
                return {Layout{size, align}};
            }
        }

        constexpr static Layout from_size_align_unchecked(usize size, usize align) {
            return {size, align};
        }

    private:
        constexpr Layout(usize s, usize a): size(s), align(a) {}
    };
} // memory

#endif //MODERN_STL_LAYOUT_H
