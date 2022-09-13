//
// Created by Shiroan on 2022/9/13.
//

#ifndef MODERN_STL_TUPLE_H
#define MODERN_STL_TUPLE_H

#include <type_traits>
#include <iostream>
#include "global.h"
#include "type_traits.h"

namespace mstl::utility {
    template<typename ...Ts>
    constexpr usize sum_of_size_of_types() {
        if constexpr (sizeof...(Ts) == 0) {
            return 0;
        } else {
            return (ConditionV<std::is_reference_v<Ts>, sizeof(usize), sizeof(Ts)> + ...);
        }
    }

    namespace _private {
        template<usize N, typename T, typename ...Ts>
        struct sum_of_size_of_front_n_types {
            static constexpr usize value = ConditionV<std::is_reference_v<T>, sizeof(usize), sizeof(T)>
                                           + sum_of_size_of_front_n_types<N - 1, Ts...>::value;
        };

        template<typename T, typename ...Ts>
        struct sum_of_size_of_front_n_types<0, T, Ts...> {
            static constexpr usize value = 0;
        };

        template<usize N, typename T, typename ...Ts>
        constexpr usize sum_of_size_of_front_n_types_v = sum_of_size_of_front_n_types<N, T, Ts...>::value;

        template<typename ...Ts>
        struct get_protector {
            constexpr static bool value = true;
        };

        template<>
        struct get_protector<> {
            constexpr static bool value = false;
        };
    }

    template<typename ...Ts>
    class Tuple {
        template<usize pos, typename Arg, typename ...Args>
        friend ArgAtT<pos, Arg, Args...> &
        get(Tuple<Arg, Args...> &tuple);

        template<usize pos, typename Arg, typename ...Args>
        friend const ArgAtT<pos, Arg, Args...> &
        get(const Tuple<Arg, Args...> &tuple);

    private:
        char inner[sum_of_size_of_types<Ts...>()];

    private:
        template<usize I, typename Arg, typename ...Args>
        requires requires {
            sizeof...(Ts) == sizeof...(Args) + 1;
            std::is_assignable_v<ArgAtT<I, Ts...> &, const ArgAtT<I, Arg, Args...> &>;
        }
        void copy_impl(const Tuple<Arg, Args...> &t) {
            get<I>(*this) = get<I>(t);

            if constexpr (I != 0) {
                copy_impl<I - 1>(t);
            }
        }

        template<usize I>
        void copy_impl(const Tuple &t) {
            get<I>(*this) = get<I>(t);

            if constexpr (I != 0) {
                copy_impl<I - 1>(t);
            }
        }

        template<typename Arg, typename ...Args>
        void emplace(usize pos, Arg v, Args ...vs) {
            usize newPos = pos;
            if constexpr (std::is_reference_v<Arg>) {               // when Arg is reference(assume Arg = T &)
                using nT = std::remove_reference_t<Arg>;            // nT = T, Arg's reference is removed
                using pT = std::add_pointer_t<nT>;                  // pT = T*, Arg is considered as a pointer to T now.s
                auto start = reinterpret_cast<pT *>(inner +
                                                    pos);    // start is T**, which means that a T* will be stored into 'inner'.
                new(start) pT{&v};                                // construct &v at start(T**)
                newPos += sizeof(usize);
            } else {  // non-reference
                auto start = reinterpret_cast<Arg *>(inner + pos);
                new(start) Arg{v};
                newPos += sizeof(Arg);
            }
            emplace<Args...>(newPos, vs...);
        }

        template<typename Arg>
        void emplace(usize pos, Arg v) {
            if constexpr (std::is_reference_v<Arg>) {
                using nT = std::remove_reference_t<Arg>;
                using pT = std::add_pointer_t<nT>;
                auto start = reinterpret_cast<pT *>(inner + pos);
                new(start) pT{&v};
            } else {  // non-reference
                auto start = reinterpret_cast<Arg *>(inner + pos);
                new(start) Arg{v};
            }
        }

        template<typename Arg, typename ...Args>
        void destroy(usize pos = 0) {
            if constexpr (std::is_class_v<Arg>) {
                reinterpret_cast<Arg *>(inner + pos)->~Arg();
            }

            if constexpr (sizeof...(Args) != 0) {
                destroy < Args...>(sizeof(Arg) + pos);
            }
        }

    public:
        Tuple(Ts ...vs) : inner{0} {
            emplace<Ts...>(0, vs...);
        }

        ~Tuple() {
            destroy<Ts...>();
        }

        static constexpr usize size() {
            return sizeof...(Ts);
        }

        constexpr Tuple &operator=(const Tuple &other) {
            if (&other == this)
                return *this;

            copy_impl<size() - 1>(other);
            return *this;
        }

        template<class UType, class... UTypes>
        constexpr Tuple &operator=(const Tuple<UType, UTypes...> &other) {
            copy_impl<size() - 1>(other);
            return *this;
        }
    };

    template<>
    class Tuple<> {  // Unit type
    public:
        Tuple() = default;

        static constexpr usize size() {
            return 0;
        }

        constexpr Tuple &operator=(const Tuple &other) = default;

        constexpr bool operator==(const Tuple &other) const {
            return true;
        }
    };

    using Unit = Tuple<>;
    constexpr Unit unit = {};

    template<usize I, typename T>
    struct TupleElement;

    template<usize I, typename Arg, typename ...Args>
    struct TupleElement<I, Tuple<Arg, Args...>> {
        using type = ArgAtT<I, Arg, Args...>;
    };

    template<usize I, typename Arg, typename ...Args>
    ArgAtT<I, Arg, Args...> &get(Tuple<Arg, Args...> &tuple) {
        using T = ArgAtT<I, Arg, Args...>;
        if constexpr (std::is_reference_v<T>) {  // when The Pointed Type T is a reference
            using pT = std::remove_reference_t<T>*;
            auto start = reinterpret_cast<pT*> (
                    tuple.inner
                    + _private::sum_of_size_of_front_n_types_v<I, Arg, Args...>
            );
            return **start;
        } else {
            auto start = reinterpret_cast<T *>(
                    tuple.inner
                    + _private::sum_of_size_of_front_n_types_v<I, Arg, Args...>
            );
            return *start;
        }
    }

    template<usize I, typename Arg, typename ...Args>
    const ArgAtT<I, Arg, Args...> &get(const Tuple<Arg, Args...> &tuple) {
        using T = ArgAtT<I, Arg, Args...>;
        if constexpr (std::is_reference_v<T>) {  // when The Pointed Type T is a reference
            using pT = std::remove_reference_t<T>*;
            auto start = reinterpret_cast<const pT*> (
                    tuple.inner
                    + _private::sum_of_size_of_front_n_types_v<I, Arg, Args...>
            );
            return **start;
        } else {
            auto start = reinterpret_cast<const T *>(
                    tuple.inner
                    + _private::sum_of_size_of_front_n_types_v<I, Arg, Args...>
            );
            return *start;
        }
    }

    template<typename T, typename ...Ts>
    Tuple<T, Ts...>
    make_tuple(T v, Ts ...vs) {
        return Tuple<T, Ts...>{v, vs...};
    }

    constexpr Unit make_tuple() {
        return unit;
    }

    template<typename T, typename ...Ts>
    Tuple<T&, Ts&...>
    tie(T& v, Ts& ...vs) {
        return Tuple<T&, Ts&...>{v, vs...};
    }
} // utility

#endif //MODERN_STL_TUPLE_H
