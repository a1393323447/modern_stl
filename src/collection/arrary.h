//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_ARRARY_H__
#define __MODERN_STL_ARRARY_H__

#include <global.h>
#include <iter/iter_concepts.h>
#include <intrinsics.h>

#include <initializer_list>
#include <iostream>

namespace mstl::collection {

    template <typename T, usize N>
    class ArrayIter {
    public:
        using Item = T;
        explicit ArrayIter(Item* p) {
            for (usize i = 0; i < N; i++) {
                ptr[i] = std::move(p[i]);
            }
        }
        ArrayIter(const ArrayIter<T, N>&) = default;
        ArrayIter<T, N>& operator=(const ArrayIter<T, N>&) = default;
        ArrayIter(ArrayIter<T, N>&& other) {
            if (this == &other) {
                return ;
            }
            for (usize i = 0; i < N; i++) {
                ptr[i] = std::move(other.ptr[i]);
            }
            pos = other.pos;
        }
        ArrayIter<T,N>& operator=(ArrayIter<T, N>&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            for (usize i = 0; i < N; i++) {
                ptr[i] = std::move(other.ptr[i]);
            }
            pos = other.pos;
        }

        MSTL_INLINE constexpr
        Option<T> next() {
            if (pos < N) {
                auto n = Option<T>::some(std::move(ptr[pos]));
                pos++;
                return n;
            } else {
                return Option<T>::none();
            }
        }
    private:
        T     ptr[N];
        usize pos = 0;
    };

    template <typename T, usize N>
    requires basic::LValRefType<T>
    class ArrayIter<T, N> {
    public:
        using Item = T;
        using StoreT   = std::remove_cvref_t<T>;
        explicit ArrayIter(StoreT* *p): ptr(p) {};

        MSTL_INLINE constexpr
        Option<T> next() {
            if (pos < N) {
                auto n = Option<T>::some(*ptr[pos]);
                pos++;
                return n;
            } else {
                return Option<T>::none();
            }
        }
    private:
        StoreT* ptr[N];
        usize   pos = 0;
    };

    template <typename T, usize N>
    class ArrayIterRef {
    public:
        using Item   = const T&;
        explicit ArrayIterRef(T *p): ptr(p) {};

        MSTL_INLINE constexpr
        Option<const T&> next() {
            if (pos < N) {
                auto n = Option<const T&>::some(ptr[pos]);
                pos++;
                return n;
            } else {
                return Option<const T&>::none();
            }
        }
    private:
        T *const ptr = nullptr;
        usize    pos = 0;
    };

    template <typename T, usize N>
    requires basic::LValRefType<T>
    class ArrayIterRef<T, N> {
    public:
        using Item = const T&;
        using StoreT = std::remove_cvref_t<T>;
        explicit ArrayIterRef(StoreT* *p): ptr(p) {};

        MSTL_INLINE constexpr
        Option<const T&> next() {
            if (pos < N) {
                auto n = Option<const T&>::some(*ptr[pos]);
                pos++;
                return n;
            } else {
                return Option<const T&>::none();
            }
        }
    private:
        const StoreT* *const ptr = nullptr;
        usize                pos = 0;
    };

    //////////////////////////////// Array ////////////////////////////////////
    template<typename T>
    struct Protected {
        static const bool value = false;
    };
    template <typename T, usize N>
    class Array {
        static_assert(Protected<T>::value,
                      "unexpected type argument: This is a bug !\n"
        );
    };
    template <typename T, usize N>
    requires basic::RValRefType<T>
    class Array<T, N> {
        static_assert(Protected<T>::value,
                      "Should not store a rvalue reference in Array !\n"
        );
    };


    template <typename T, usize N>
    requires basic::LValRefType<T>
    class Array<T, N> {
    public:
        using Item = T;
        using StoreT   = std::remove_cvref_t<T>;
        using IntoIter = ArrayIter<T, N>;
        using IterRef  = ArrayIterRef<T, N>;

        Array(const Array<T, N>& other) = default;
        Array<T, N>& operator=(const Array<T, N>& other) = default;

        Array(Array<T, N>&& other) noexcept {
            if (this == &other) return;

            for (usize i = 0; i < N; i++) {
                values[i] = other[i];
                other[i] = nullptr;
            }
        }

        Array<T, N>& operator=(Array<T, N>&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            for (usize i = 0; i < N; i++) {
                values[i] = other[i];
                other[i] = nullptr;
            }
        }

        MSTL_INLINE
        IntoIter into_iter() {
            auto iter = IntoIter { const_cast<StoreT*>(values) };
            // Move into iter
            for (usize i = 0; i < N; i++) {
                values[i] = nullptr;
            }
            return iter;
        }

        MSTL_INLINE
        IterRef iter() const {
            return IterRef { const_cast<StoreT*>(values) };
        }

        template<iter::Iterator Iter>
        MSTL_INLINE
        static decltype(auto) from_iter(Iter iter) {
            usize pos = 0;
            Array<typename Iter::Item, N> arr{};
            Option<typename Iter::Item> next = iter.next();
            while (next.is_some() && pos < N) {
                auto&& ele = next.unwrap_uncheck();
                auto *ptr = const_cast<StoreT*>(std::addressof(ele));
                arr[pos] = ptr;
                next = iter.next();
                pos++;
            }

            return arr;
        }

        MSTL_INLINE constexpr
        T& operator[](usize pos) {
            if (pos >= N) {
                MSTL_PANIC("Array: index out of boundary.");
            }
            return *values[pos];
        }

        MSTL_INLINE constexpr
        const T& operator[](usize pos) const {
            if (pos >= N) {
                MSTL_PANIC("Array: index out of boundary.");
            }
            return *values[pos];
        }

        MSTL_INLINE
        constexpr static usize size() { return N; }
    private:
        StoreT* values[N];
    };


    template <typename T, usize N>
    requires basic::CopyAble<T> &&
             (!basic::RefType<T>)
    class Array<T, N> {
    public:
        using Item = T;
        using IntoIter = ArrayIter<T, N>;
        using IterRef  = ArrayIterRef<T, N>;

        Array(std::initializer_list<T> list) {
            if (list.size() > N) {
                MSTL_PANIC("Excess elements in array initializer");
            }
            usize pos = 0;
            for (T ele: list) {
                this->values[pos] = ele;
                pos++;
                if (pos >= N) {
                    break;
                }
            }
        }

        Array(const Array<T, N>& other) {
            for (usize pos = 0; pos < N; pos++) {
                values[pos] = other.values[pos];
            }
        }

        Array<T, N>& operator=(const Array<T, N>& other) {
            if  (this == &other) {
                return *this;
            }

            for (usize pos = 0; pos < N; pos++) {
                values[pos] = other.values[pos];
            }

            return *this;
        }

        Array(Array<T, N>&& other) noexcept {
            for (usize pos = 0; pos < N; pos++) {
                values[pos] = std::move(other.values[pos]);
            }
        }

        Array<T, N>& operator=(Array<T, N>&& other) noexcept {
            if  (this == &other) {
                return *this;
            }

            for (usize pos = 0; pos < N; pos++) {
                values[pos] = std::move(other.values[pos]);
            }

            return *this;
        }

        MSTL_INLINE
        IntoIter into_iter() {
            return IntoIter { const_cast<T*>(values) };
        }

        MSTL_INLINE
        IterRef iter() const {
            return IterRef { const_cast<T*>(values) };
        }

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
        Item values[N];
    };


    template <typename T, usize N>
    requires basic::Movable<T> &&
             (!basic::CopyAble<T>) &&
             (!basic::RefType<T>)
    class Array<T, N> {
    public:
        using Item = T;
        using IntoIter = ArrayIter<T, N>;
        using IterRef  = ArrayIterRef<T, N>;

        Array(std::initializer_list<T> list) {
            if (list.size() >= N) {
                MSTL_PANIC("Excess elements in array initializer");
            }
            usize pos = 0;
            for (T ele: list) {
                this->values[pos] = ele;
                pos++;
                if (pos >= N) {
                    break;
                }
            }
        }

        Array(const Array<T, N>& other) = delete;
        Array<T, N>& operator=(const Array<T, N>& other) = delete;

        Array(Array<T, N>&& other) noexcept {
            for (usize pos = 0; pos < N; pos++) {
                values[pos] = std::move(other.values[pos]);
            }
        }

        Array<T, N>& operator=(Array<T, N>&& other) noexcept {
            if  (this == &other) {
                return *this;
            }

            for (usize pos = 0; pos < N; pos++) {
                values[pos] = std::move(other.values[pos]);
            }

            return *this;
        }

        MSTL_INLINE
        IntoIter into_iter() {
            return IntoIter { const_cast<T*>(values) };
        }

        MSTL_INLINE
        IterRef iter() const {
            return IterRef { const_cast<T*>(values) };
        }

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
        Item values[N];
    };

    static_assert(iter::Iterator<ArrayIter<int, 10>>);
    static_assert(std::is_same_v<ArrayIter<int, 10>::Item, int>);

    static_assert(iter::Iterator<ArrayIterRef<int, 10>>);
    static_assert(std::is_same_v<ArrayIterRef<int, 10>::Item, const int&>);

    static_assert(iter::IntoIterator<Array<int, 10>>);
    static_assert(iter::FromIterator<Array<int, 10>, ArrayIter<int, 10>>);
}

#endif //__MODERN_STL_ARRARY_H__
