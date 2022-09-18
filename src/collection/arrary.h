//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_ARRARY_H__
#define __MODERN_STL_ARRARY_H__

#include <global.h>
#include <basic_concepts.h>
#include <iter/iter_concepts.h>

#include <initializer_list>

namespace mstl::collection::array {

    namespace _iter {
        // TODO: T& and T is store differently in values. Need to discuss.
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
        private:
            T* const ptr = nullptr;
            usize    pos = 0;
        };

        template <typename T, usize N>
        class ArrayIterRef {
        public:
            using Item = const T&;
            explicit ArrayIterRef(T* p): ptr(p) {};
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
            const T* ptr = nullptr;
            usize    pos = 0;
        };
    }

////////////////////////////////////// Array //////////////////////////////////////
    template<typename T, usize N>
    class Array {
        static_assert(
            !std::same_as<T, T>,
            "Unexpected Type: this is bug !\n"
        );
    };

    template <typename T, usize N>
    requires basic::RValRefType<T>
    class Array<T, N> {
        static_assert(
            basic::RValRefType<T>,
            "mstl::Array can not store a rvalue reference type.\n"
        );
    };

    template <typename T, usize N>
    requires basic::LValRefType<T>
    class Array<T, N> {
    public:
        using Item = T;
        using IntoIter  = _iter::ArrayIter<T, N>;
        using IterRef   = _iter::ArrayIterRef<T, N>;
        using StroeType = std::add_pointer_t<std::remove_cvref<T>>; // expected type: T*

        Array() = default;

        Array(std::initializer_list<T> list) {
            // FIXME: panic if list.size() > N
            usize pos = 0;
            for (const T ele: list) {
                auto ptr = const_cast<StroeType>(std::addressof(ele));
                this->values[pos] = ele;
                pos++;
                if (pos >= N) {
                    break;
                }
            }
        }

        IntoIter into_iter() {
            // FIXME: Need to implement `move`
            return IntoIter { values };
        }

        IterRef iter() {
            return IterRef { values };
        }

        template<iter::Iterator Iter>
        static Array<T, N> from_iter(Iter iter) {
            usize pos = 0;
            Array<T, N> arr{};
            Option<T> next = iter.next();
            while (next.is_some()) {
                T ref = next.unwrap_uncheck();
                StroeType ptr = std::addressof(ref);

                arr[pos] = ref;
                pos++;

                next = iter.next();
            }
            return arr;
        }

        T operator[](usize pos) {
            // FIXME: panic if pos >= N
            StroeType ptr = values[pos];
            return *ptr;
        }
        constexpr usize size() { return N; }
    private:
        StroeType values[N];
    };

    template <typename T, usize N>
    requires basic::CopyAble<T> && (!basic::RefType<T>)
    class Array<T, N> {
    public:
        using Item = T;
        using IntoIter = _iter::ArrayIter<T, N>;
        using IterRef  = _iter::ArrayIterRef<T, N>;

        Array() = default;

        Array(std::initializer_list<T> list) {
            // FIXME: panic if list.size() > N
            usize pos = 0;
            for (const T& ele: list) {
                this->values[pos] = ele;
                pos++;
                if (pos >= N) {
                    break;
                }
            }
        }

        IntoIter into_iter() {
            // FIXME: Need to implement `move`
            return IntoIter { values };
        }

        IterRef iter() {
            return IterRef { values };
        }

        template<iter::Iterator Iter>
        static Array<T, N> from_iter(Iter iter) {
            usize pos = 0;
            Array<T, N> arr{};
            Option<T> next = iter.next();
            while (next.is_some()) {
                arr[pos] = next.unwrap_uncheck();
                next = iter.next();
                pos++;
            }

            return arr;
        }

        T& operator[](usize pos) {
            // FIXME: panic if pos >= N
            return values[pos];
        }
        constexpr usize size() { return N; }
    private:
        Item values[N];
    };


    template <typename T, usize N>
    requires basic::Movable<T>    &&
            (!basic::CopyAble<T>) &&
            (!basic::RefType<T>)
    class Array<T, N> {
    public:
        using Item = T;
        using IntoIter = _iter::ArrayIter<T, N>;
        using IterRef  = _iter::ArrayIterRef<T, N>;

        Array() = default;

        Array(std::initializer_list<T> list) {
            // FIXME: panic if list.size() > N
            usize pos = 0;
            for (const T& ele: list) {
                this->values[pos] = std::move(ele);
                pos++;
                if (pos >= N) {
                    break;
                }
            }
        }

        IntoIter into_iter() {
            // FIXME: Need to implement `move`
            return IntoIter { values };
        }

        IterRef iter() {
            return IterRef { values };
        }

        template<iter::Iterator Iter>
        static Array<T, N> from_iter(Iter iter) {
            usize pos = 0;
            Array<T, N> arr{};
            Option<T> next = std::move(iter.next());
            while (next.is_some()) {
                arr[pos] = std::move(next.unwrap_uncheck());
                next = std::move(iter.next());
                pos++;
            }

            return arr;
        }

        T& operator[](usize pos) {
            // FIXME: panic if pos >= N
            return values[pos];
        }
        constexpr usize size() { return N; }
    private:
        Item values[N];
    };

    static_assert(iter::Iterator<_iter::ArrayIter<int, 10>>);
    static_assert(std::is_same_v<_iter::ArrayIter<int, 10>::Item, int>);

    static_assert(iter::Iterator<_iter::ArrayIterRef<int, 10>>);
    static_assert(std::is_same_v<_iter::ArrayIterRef<int, 10>::Item, const int&>);

    static_assert(iter::IntoIterator<Array<int, 10>>);
    static_assert(iter::FromIterator<Array<int, 10>, _iter::ArrayIter<int, 10>>);
}

namespace mstl::collection {
    /**    Export    **/
    using array::Array;
}

#endif //__MODERN_STL_ARRARY_H__
