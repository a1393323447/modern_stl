//
// Created by 朕与将军解战袍 on 2022/9/27.
//

#ifndef __MODERN_STL_NEW_ARRAY_H__
#define __MODERN_STL_NEW_ARRAY_H__

#include <global.h>
#include <ops/range.h>
#include <intrinsics.h>
#include <basic_concepts.h>
#include <iter/iter_concepts.h>
// std
#include <initializer_list>

namespace mstl::collection {
    template<typename T, usize N>
    class ArrayIter {
    public:
        using Item = T;
        constexpr ArrayIter(Item* arr_data) {
            for (usize i = 0; i < N; i++) {
                data[i] = std::move(arr_data[i]);
            }
        }
        /// impl copy
        constexpr ArrayIter(const ArrayIter&) requires basic::CopyAble<T> = default;
        constexpr ArrayIter& operator=(const ArrayIter& other)
        requires basic::CopyAble<T> {
            if (this == &other) {
                return *this;
            }

            for (usize pos = 0; pos < N; pos++) {
                data[pos] = other.data[pos];
            }
            return *this;
        }
        ArrayIter(const ArrayIter&) requires (!basic::CopyAble<T>) = delete;
        ArrayIter& operator=(const ArrayIter& other)
        requires (!basic::CopyAble<T>) = delete;
        /// impl move
        constexpr ArrayIter(ArrayIter&& other) noexcept {
            if (this == &other) {
                return ;
            }
            for (usize i = 0; i < N; i++) {
                data[i] = std::move(other.data[i]);
            }
            range = other.range;
        }
        constexpr ArrayIter& operator=(ArrayIter&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            for (usize i = 0; i < N; i++) {
                data[i] = std::move(other.data[i]);
            }
            range = other.range;
        }
        ArrayIter(ArrayIter&& other) noexcept
        requires (!basic::Movable<T>) = delete;
        ArrayIter& operator=(ArrayIter&& other) noexcept
        requires (!basic::Movable<T>) = delete;
        /// impl Iterator
        MSTL_INLINE constexpr
        Option<T> next() {
            return range.next().map([&](usize pos){
                Item item = std::move(data[pos]);
                return item;
            });
        }
        /// impl DoubleEndedIterator
        MSTL_INLINE constexpr
        Option<T> prev() {
            return range.prev().map([&](usize pos){
                Item item = std::move(data[pos]);
                return item;
            });
        }
    private:
        Item data[N];
        ops::Range<usize> range = {0, N};
    };

    template <typename T, usize N>
    class ArrayIterRef {
    public:
        using Item   = const T&;
        constexpr ArrayIterRef(T *ptr): start(ptr), end(ptr + N) {};

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
        T *start = nullptr;
        T *end = nullptr;
    };



    template<typename T, usize N>
    class Array {
        static_assert(
            !basic::RefType<T>,
            "Can not store a reference in an array.\n"
        );
    };

    template<typename T, usize N>
    requires (!basic::RefType<T>)
    class Array<T, N> {
        static_assert(N > 0, "Can not have a zero size Array.\n");
    public:
        using Item = T;
        using IntoIter = ArrayIter<T, N>;
        using IterRef  = ArrayIterRef<T, N>;
        Array() = default;
        constexpr Array(std::initializer_list<T> list) requires basic::Movable<T> {
            if (list.size() > N) {
                // FIXME: May be it should change to a warning
                MSTL_PANIC("Excess elements in array initializer");
            }
            usize pos = 0;
            for (T ele: list) {
                this->values[pos] = std::move(ele);
                pos++;
                if (pos >= N) {
                    break;
                }
            }
        }
        constexpr Array(const T (& raw)[N]) {
            for (usize i = 0; i < N; i++) {
                values[i] = raw[i];
            }
        }
        /// impl Copy for Array
        constexpr Array(const Array&) requires basic::CopyAble<T> = default;
        constexpr Array& operator=(const Array& other)
        requires basic::CopyAble<T> {
            if  (this == &other) {
                return *this;
            }

            for (usize pos = 0; pos < N; pos++) {
                values[pos] = other.values[pos];
            }
            return *this;
        }
        Array(const Array&) requires (!basic::CopyAble<T>) = delete;
        Array& operator=(const Array&)
        requires (!basic::CopyAble<T>) = delete;
        /// impl move for Array
        constexpr Array(Array&& other) noexcept
        requires basic::Movable<T> {
            for (usize pos = 0; pos < N; pos++) {
                values[pos] = std::move(other.values[pos]);
            }
        }
        constexpr Array& operator=(Array&& other) noexcept
        requires basic::Movable<T> {
            if  (this == &other) {
                return *this;
            }

            for (usize pos = 0; pos < N; pos++) {
                values[pos] = std::move(other.values[pos]);
            }

            return *this;
        }
        Array(Array&& other) requires (!basic::Movable<T>) = delete;
        Array& operator=(Array&& other)
        requires (!basic::Movable<T>) = delete;
        /// impl IntoIter
        MSTL_INLINE constexpr
        IntoIter into_iter() {
            return IntoIter { const_cast<T*>(values) };
        }
        MSTL_INLINE constexpr
        IterRef iter() const {
            return IterRef { const_cast<T*>(values) };
        }
        /// impl FromIter
        template<iter::Iterator Iter>
        MSTL_INLINE
        static decltype(auto) from_iter(Iter iter) {
            usize pos = 0;
            Array<typename Iter::Item, N> arr{};
            Option<typename Iter::Item> next = iter.next();
            while (next.is_some() && pos < N) {
                arr[pos] = next.unwrap_uncheck();
                next = iter.next();
                pos++;
            }

            return arr;
        }
        /// impl index
        MSTL_INLINE constexpr
        T& operator[](usize pos) {
            if (pos >= N) {
                MSTL_PANIC("Array: index out of boundary.");
            }
            return values[pos];
        }
        MSTL_INLINE constexpr
        const T& operator[](usize pos) const {
            if (pos >= N) {
                MSTL_PANIC("Array: index out of boundary.");
            }
            return values[pos];
        }

        MSTL_INLINE constexpr
        static usize size() { return N; }

    private:
        T values[N];
    };
}

#endif //__MODERN_STL_NEW_ARRAY_H__
