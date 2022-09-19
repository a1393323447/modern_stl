//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_ARRARY_H__
#define __MODERN_STL_ARRARY_H__

#include <global.h>
#include <iter/iter_concepts.h>
#include <intrinsics.h>

#include <initializer_list>

namespace mstl::collection {

    template <typename T, usize N>
    class ArrayIter {
    public:
        using Item = T;
        explicit ArrayIter(Item* p): ptr(p) {};
        MSTL_INLINE Option<T> next() {
            if (pos < N) {
                auto n = Option<T>::some(ptr[pos]);
                pos++;
                return n;
            } else {
                return Option<T>::none();
            }
        }
    private:
        T* const ptr = nullptr;
        usize    pos = 0;
    };

    template <typename T, usize N>
    class ArrayIterRef {
    public:
        using Item = const T&;
        explicit ArrayIterRef(T* p): ptr(p) {};
        MSTL_INLINE Option<const T&> next() {
            if (pos < N) {
                auto n = Option<const T&>::some(ptr[pos]);
                pos++;
                return n;
            } else {
                return Option<const T&>::none();
            }
        }
    private:
        const T* ptr = nullptr;
        usize    pos = 0;
    };

    template <typename T, usize N>
    class Array {
    public:
        using Item = T;
        using IntoIter = ArrayIter<T, N>;
        using IterRef  = ArrayIterRef<T, N>;

        Array(std::initializer_list<T> list) {
            // FIXME: panic if list.size() > N
            usize pos = 0;
            for (T ele: list) {
                this->values[pos] = ele;
                pos++;
                if (pos >= N) {
                    break;
                }
            }
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
            while (next.is_some()) {
                arr[pos] = next.unwrap_uncheck();
                next = iter.next();
                pos++;
            }

            return arr;
        }

        MSTL_INLINE
        T& operator[](usize pos) {
            // FIXME: panic if pos >= N
            return values[pos];
        }

        MSTL_INLINE
        constexpr usize size() const { return N; }
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