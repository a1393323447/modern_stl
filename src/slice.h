//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_SLICE_H__
#define __MODERN_STL_SLICE_H__

#include <global.h>
#include <intrinsics.h>
#include <basic_concepts.h>
#include <ops/range.h>
#include <option/option.h>

namespace mstl {
    template<typename T, typename I>
    class SliceRefIter {
    public:
        using Item = I;
        constexpr SliceRefIter(T* ptr, usize len): start(ptr), end(ptr + len) {}

        MSTL_INLINE constexpr
        Option<Item> next() {
            if (start >= end) {
                return Option<Item>::none();
            } else {
                auto op = Option<Item>::some(*start);
                start++;
                return op;
            }
        }

        MSTL_INLINE constexpr
        Option<Item> prev() {
            if (start >= end) {
                return Option<Item>::none();
            } else {
                end--;
                auto op = Option<Item>::some(*end);
                return op;
            }
        }

    private:
        T* start = nullptr;
        T* end = nullptr;
    };

    /// Slice 并不拥有数据的所有权
    template<typename T>
    requires (!basic::RefType<T>)
    class Slice {
    public:
        using Item = T;
        using IntoIter      = SliceRefIter<T, T&>;
        using ConstRefIter  = SliceRefIter<T, const T&>;
        using MutRefIter    = SliceRefIter<T, T&>;

        constexpr Slice(T *ptr, usize size): ptr(ptr), size(size) {}

        constexpr Slice(const Slice&) = default;
        constexpr Slice& operator=(const Slice& other) = default;

        MSTL_INLINE constexpr
        static Slice<T> from_raw(T* ptr, usize size) {
            return { ptr, size };
        }

        MSTL_INLINE constexpr
        IntoIter into_iter() {
            auto iter = IntoIter { const_cast<T*>(ptr), size };
            ptr  = nullptr;
            size = 0;
            return iter;
        }

        MSTL_INLINE constexpr
        ConstRefIter iter() const {
            return ConstRefIter { const_cast<T*>(ptr), size };
        }

        MSTL_INLINE constexpr
        MutRefIter iter_mut() {
            return MutRefIter { ptr, size };
        }

    private:
        T* ptr = nullptr;
        usize size = 0;
    };
}

#endif //__MODERN_STL_SLICE_H__
