//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_ARRARY_H__
#define __MODERN_STL_ARRARY_H__

#include <global.h>
#include <iter/iter_concepts.h>

#include <initializer_list>

namespace mstl::collection {

    template <typename T, usize N>
    class ArrayIter {
    public:
        using Item = T;
        explicit ArrayIter(Item* p): ptr(p) {};
        Option<T> next() {
            if (pos < N) {
                auto n = Option<T>::some(ptr[pos]);
                pos++;
                return n;
            } else {
                return Option<T>::none();
            }
        }
        usize size_hint() {
            return N;
        }
    private:
        T*    ptr = nullptr;
        usize pos = 0;
    };

    template <typename T, usize N>
    class Array {
    public:
        using Item = T;
        using IntoIter = ArrayIter<T, N>;

        Array(std::initializer_list<T> list) {
            // FIXME: panic if list.size() > N
            usize pos = 0;
            for (T ele: list) {
                this->values[pos] = ele;
                pos++;
                if (pos >= N) [[unlikely]] {
                    break;
                }
            }
        }

        IntoIter into_iter() {
            return IntoIter { values };
        }

        template<iter::Iterator Iter>
        static decltype(auto) from_iter(Iter iter) {
            usize pos = 0;
            Array<typename Iter::Item, N> arr{};
            Option<typename Iter::Item> next = iter.next();
            while (next.is_some()) {
                arr[pos] = next.unwrap();
                next = iter.next();
                pos++;
            }

            return arr;
        }

        T& operator[](usize pos) {
            // FIXME: panic if pos >= N
            return values[pos];
        }
        usize size() { return N; }
    private:
        Item values[N];
    };

    static_assert(iter::Iterator<ArrayIter<int, 10>>);
    static_assert(iter::IntoIterator<Array<int, 10>>);
    static_assert(iter::FromIterator<Array<int, 10>, ArrayIter<int, 10>>);
}

#endif //__MODERN_STL_ARRARY_H__
