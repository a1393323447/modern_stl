//
// Created by Shiroan on 2022/9/22.
//

#ifndef MODERN_STL_LAYOUT_H
#define MODERN_STL_LAYOUT_H

#include <mstl/basic_concepts.h>
#include <mstl/result/result.h>
#include <mstl/global.h>
#include <bit>
#include "layout_error.h"

namespace mstl::memory {
    /**
     * 描述一种类型的内存布局.
     */
    struct Layout {
        usize size;   /// <该类型对象的大小
        usize align;  /// <该类型的对齐

        /**
         * @brief 从类型生成Layout对象
         * @tparam T 需要生成Layout的类型
         * @return 描述该类型的Layout
         */
        template<class T>
        requires (!basic::RefType<T>)
        constexpr static Layout from_type() {
            return {sizeof(T), alignof(T)};
        }

        /**
         * @brief 使用参数生成Layout.
         *
         * 用给定的size和align生成Layout.
         *
         * 对align的合法性进行检查. 当且仅当align满足以下条件时合法:
         * - align为非零值.
         * - align为2的幂.
         *
         * @return 若align合法, 则返回`Layout`; 否则, 返回`LayoutError`.
         */
        static constexpr mstl::result::Result<Layout, LayoutError> from_size_align(usize size, usize align) {
            if (align == 0 || std::popcount(align) != 1) {
                return {LayoutError{}};
            } else {
                return {Layout{size, align}};
            }
        }

        /**
         * @brief 使用参数生成Layout. 不进行合法性检查.
         */
        constexpr static Layout from_size_align_unchecked(usize size, usize align) {
            return {size, align};
        }

    private:
        constexpr Layout(usize s, usize a): size(s), align(a) {}
    };
} // memory

std::ostream& operator<<(std::ostream& os, const mstl::memory::Layout& layout) {
    os << "Layout [size: " << layout.size << ", align: " << layout.align << "]";
    return os;
}

#endif //MODERN_STL_LAYOUT_H
