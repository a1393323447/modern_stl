//
// Created by Shiroan on 2022/9/13.
//

#ifndef MODERN_STL_TUPLE_H
#define MODERN_STL_TUPLE_H

#include <type_traits>
#include <concepts>
#include <utility>
#include "global.h"
#include "type_traits.h"

namespace mstl::utility {

    namespace _private {
        /**
         * 计算<T, Ts...>中前N个类型的大小总和.
         * 特别地, 引用类型被视为指针处理, 因此它的大小是一个指针的长度, 而非基类型的长度.
         *
         * @tparam N 需要累加类型大小的数量
         * */
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

        /**
             *  计算模板参数包中各类型参数的大小总和.
             *  特别地, 引用类型被视为指针处理, 因此它的大小是一个指针的长度, 而非基类型的长度.
             *
             *  @tparam Ts 参数包, 需要计算累加大小的类型
             *
             *  <h3>Example</h3>
             *  @code
             *  static_assert(sum_of_size_of_types<int, int>() == sizeof(int) + sizeof(int));
             *  static_assert(sum_of_size_of_types<int, int&>() == sizeof(int) + sizeof(usize));
             *  static_assert(sum_of_size_of_types<>() == 0);
             *  @endcode
             * */
        template<typename ...Ts>
        constexpr usize sum_of_size_of_types() {
            if constexpr (sizeof...(Ts) == 0) {
                return 0;
            } else {
                return (ConditionV<std::is_reference_v<Ts>, sizeof(usize), sizeof(Ts)> + ...);
            }
        }
    }


    /**
     * 元组. 储存固定大小的异类值.
     *
     * @tparam Ts 参数包, 需要储存的类型列表
     * */
    template<typename ...Ts>
    class Tuple {
        template<usize pos, typename Arg, typename ...Args>
        friend ArgAtT<pos, Arg, Args...> &
        get(Tuple<Arg, Args...> &tuple);

        template<usize pos, typename Arg, typename ...Args>
        friend const ArgAtT<pos, Arg, Args...> &
        get(const Tuple<Arg, Args...> &tuple);

        template<usize pos>
        using Type = ArgAtT<pos, Ts...>;

    private:
        char inner[_private::sum_of_size_of_types<Ts...>()];

    private:
        // 对复制的实现, 兼容可隐式转换的值
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

        // 在同类型Tuple之间复制
        template<usize I>
        void copy_impl(const Tuple &t) {
            get<I>(*this) = get<I>(t);  // 调用此函数是否产生歧义? 目前没有复制构造函数.

            if constexpr (I != 0) {
                copy_impl<I - 1>(t);
            }
        }

        // 原地构建值, 仅在初始化时使用
        template<typename Arg, typename ...Args>
        void emplace(usize pos, Arg v, Args ...vs) {
            usize newPos = pos;
            if constexpr (std::is_reference_v<Arg>) {               // when Arg is reference(assume Arg = T &)
                using nT = std::remove_reference_t<Arg>;            // nT = T, Arg's reference is removed
                using pT = std::add_pointer_t<nT>;                  // pT = T*, Arg is considered as a pointer to T now.s
                auto start = reinterpret_cast<pT *>(inner +
                                                    pos);           // start is T**, which means that a T* will be stored into 'inner'.
                new(start) pT{std::addressof(v)};                                // construct &v at start(T**)
                newPos += sizeof(usize);
            } else {  // non-reference
                auto start = reinterpret_cast<Arg *>(inner + pos);
                new(start) Arg{v};
                newPos += sizeof(Arg);
            }
            emplace < Args...>(newPos, vs...);
        }

        template<typename Arg>
        void emplace(usize pos, Arg v) {
            if constexpr (std::is_reference_v<Arg>) {
                using nT = std::remove_reference_t<Arg>;
                using pT = std::add_pointer_t<nT>;
                auto start = reinterpret_cast<pT *>(inner + pos);
                new(start) pT{std::addressof(v)};
            } else {  // non-reference
                auto start = reinterpret_cast<Arg *>(inner + pos);
                new(start) Arg{v};
            }
        }

        // 复制构造函数的内部实现
        template<usize I>
        void emplace(const Tuple& t) requires (I < sizeof...(Ts)) {
            usize pos = _private::sum_of_size_of_front_n_types_v<I, Ts...>;
            using T = ArgAtT<I, Ts...>;

            if constexpr (std::is_reference_v<T>) {
                using nT = std::remove_reference_t<T>*;
                auto start = reinterpret_cast<nT*>(inner + pos);
                new (start) nT{*reinterpret_cast<nT*>(t.inner + pos)};
            } else {
                auto start = reinterpret_cast<T*>(inner + pos);
                new (start) T{get<I>(t)};
            }

            if constexpr (I < size() - 1) {
                emplace<I + 1>(t);
            }
        }

        // 销毁, 对所有类类型对象逐一调用析构函数
        template<typename Arg, typename ...Args>
        void destroy(usize pos = 0) {
            if constexpr (std::is_class_v<Arg>) {
                reinterpret_cast<Arg *>(inner + pos)->~Arg();
            }

            if constexpr (sizeof...(Args) != 0) {
                destroy < Args...>(
                        ConditionV<std::is_reference_v<Arg>,
                                sizeof(usize),
                                sizeof(Arg)>
                        + pos);
            }
        }

        template<usize I, typename ...Args>
        bool equal_impl(const Tuple<Args...> &other) const requires requires(ArgAtT<I, Ts...> a, ArgAtT<I, Args...> b){
            sizeof...(Ts) == sizeof...(Args) + 1;
            a == b;
        } {
            bool res = get<I>(other) == get<I>(*this);
            if (!res) {
                return false;
            } else {
                if constexpr (I != 0) {
                    return equal_impl < I - 1 > (other);
                } else {
                    return true;
                }
            }
        }

    public:
        Tuple(Ts ...vs) : inner{0} {
            emplace<Ts...>(0, vs...);  // std::forward<Ts>(vs)...无效, 为什么?
        }

        ~Tuple() {
            destroy<Ts...>();
        }

        Tuple(const Tuple& t): inner{0} {
            emplace<0>(t);
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

        template<typename Arg, typename ...Args>
        bool operator==(const Tuple<Arg, Args...> &other) const {
            return equal_impl<size() - 1>(other);
        }

        bool operator==(const Tuple &other) const {
            return equal_impl<size() - 1>(other);
        }

        auto& first() requires (sizeof...(Ts) == 2) {
            return get<0>(*this);
        }

        const auto& first() const requires (sizeof...(Ts) == 2) {
            return get<0>(*this);
        }

        auto& second() requires (sizeof...(Ts) == 2) {
            return get<1>(*this);
        }

        const auto& second() const requires (sizeof...(Ts) == 2) {
            return get<1>(*this);
        }
    };

    /**
     * 空元组, 或称"单位类型".
     * */
    template<>
    class Tuple<> {  // Unit type
    public:
        Tuple() = default;
        Tuple(const Tuple&) = default;

        static constexpr usize size() {
            return 0;
        }

        constexpr Tuple &operator=(const Tuple &) = default;

        constexpr bool operator==(const Tuple &) const {
            return true;
        }
    };

    using Unit = Tuple<>;
    constexpr Unit unit = {};

    /**
     * 元函数<br>
     * 返回元组T的第I个元素的类型<br>
     * 传入空元组或非元组, 则程序非良构
     *
     * @tparam I 索引
     * @tparam T 一个元组类型
     *<h3>Example</h3>
     * @code
     * using T = Tuple<int, double>;
     * using V = typename TupleElenemt<1, T>::type;
     * static_assert(std::same_as<T, V>);
     * @endcode
     * */
    template<usize I, typename T>
    struct TupleElement;

    template<usize I, typename Arg, typename ...Args>
    struct TupleElement<I, Tuple<Arg, Args...>> {
        using type = ArgAtT<I, Arg, Args...>;
    };

    /**
     * 获取元组中储存的值.
     *
     * @tparam I 索引
     * @param tuple 需要取值的元组
     * @return 元组tuple在第I个位置储存的值
     *<h3>Example</h3>
     * @code
     * auto t = Tuple<int, double>{1, 2.0};
     * assert(get<1>(t) == 2.0);
     * @endcode
     * */
    template<usize I, typename Arg, typename ...Args>
    ArgAtT<I, Arg, Args...> &get(Tuple<Arg, Args...> &tuple) {
        using T = ArgAtT<I, Arg, Args...>;
        if constexpr (std::is_reference_v<T>) {         // when The Pointed Type T is a reference (assume it's A&)
            using pT = std::remove_reference_t<T> *;     // pT = A*
            auto start = reinterpret_cast<pT *> (        // start => A**, which means the start position of pointer A*
                    tuple.inner
                    + _private::sum_of_size_of_front_n_types_v<I, Arg, Args...>
            );
            return **start;                             // return A&
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
        if constexpr (std::is_reference_v<T>) {         // when The Pointed Type T is a reference (assume it's A&)
            using pT = std::remove_reference_t<T> *;     // pT = A*
            auto start = reinterpret_cast<const pT *> (  // start => const A**
                    tuple.inner
                    + _private::sum_of_size_of_front_n_types_v<I, Arg, Args...>
            );
            return **start;                             // return const A&
        } else {
            auto start = reinterpret_cast<const T *>(
                    tuple.inner
                    + _private::sum_of_size_of_front_n_types_v<I, Arg, Args...>
            );
            return *start;
        }
    }

    /**
     * 用给定参数构建一个元组
     *
     *<h3>Example</h3>
     * @code
     * auto t = make_tuple(1, 2.0, 'c');
     * static_assert(std::same_as<decltype(t), Tuple<int, double, char>);
     * assert(get<2>(t) == 'c');
     * @endcode
     * */
    template<typename T, typename ...Ts>
    Tuple<T, Ts...>
    make_tuple(T v, Ts ...vs) {
        return Tuple<T, Ts...>{v, vs...};
    }

    template<std::default_initializable T, std::default_initializable ...Ts>
    Tuple<T, Ts...>
    make_tuple() {
        return Tuple<T, Ts...>{T{}, Ts{}...};
    }

    /**
     * 返回一个空元组的单例
     * */
    constexpr Unit make_tuple() {
        return unit;
    }

    /**
     * 对给定的实参(v, vs...), 返回由它们的引用构成的元组.
     *<h3>Example</h3>
     * @code
     * auto a = make_tuple(1, 2);
     * int j, k;
     * tie(j, k) = a;
     * assert(j == 1);
     * assert(k == 2);
     * @endcode
     * */
    template<typename T, typename ...Ts>
    Tuple<T &, Ts &...>
    tie(T &v, Ts &...vs) {
        return Tuple<T &, Ts &...>{v, vs...};
    }

    // Pair<T, U>
    template<typename T, typename U>
    using Pair = Tuple<T, U>;

    template<typename T, typename U>
    Pair<std::remove_reference_t<T>, std::remove_reference_t<U>> make_pair(T&& t, U&& u) {
        return make_tuple<std::remove_reference_t<T>,
                        std::remove_reference_t<U>>(std::forward<T>(t), std::forward<U>(u));
    }
} // utility

#endif //MODERN_STL_TUPLE_H
