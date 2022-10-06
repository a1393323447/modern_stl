//
// Created by Shiroan on 2022/9/13.
//

#ifndef MODERN_STL_TUPLE_H
#define MODERN_STL_TUPLE_H

#include <type_traits>
#include <concepts>
#include <utility>

#include <ops/ops.h>
#include "global.h"
#include "type_traits.h"

namespace mstl::utility {
    /**
     * @brief 元组.
     *
     * 储存固定大小的异类值.
     *
     * @tparam Ts 参数包, 需要储存的类型列表
     * */
    template<typename ...Ts>
    class Tuple {
    };


    template<typename First, typename ...Rest>
    class Tuple<First, Rest...> {
        First f;
        Tuple<Rest...> r;

    public:
        template<class F, class ...R>
        constexpr Tuple(F &&f, R &&...r) noexcept
        requires (std::convertible_to<F, First> && (std::convertible_to<R, Rest> && ...))
                : f(std::forward<F>(f)), r(std::forward<R>(r)...) {}

        constexpr Tuple(const Tuple &other) noexcept requires basic::CopyAble<First>
                : f(other.f), r(other.r) {}

        constexpr Tuple(Tuple &&other)  noexcept requires basic::Movable<First>
                : f(std::move(other.f)), r(std::move(other.r)) {}



        constexpr Tuple& operator=(const Tuple& other) noexcept requires basic::CopyAble<First> {
            if (&other == this) {
                return *this;
            }

            f = other.f;
            r = other.r;
            return *this;
        }

        constexpr Tuple& operator=(Tuple&& other) noexcept requires basic::Movable<First> {
            if (&other == this) {
                return *this;
            }

            f = std::move(other.f);
            r = std::move(other.r);
            return *this;
        }

        template<class UType, class... UTypes>
        constexpr Tuple& operator=(const Tuple<UType, UTypes...> &other)
        requires ops::Assign<First, UType> && (ops::Assign<Rest, UTypes> && ...) {
            f = other.get_first();
            r = other.get_rest();
            return *this;
        }

        template<class F, class ...R>
        constexpr bool operator==(const Tuple<F, R...>& rhs) const
        requires ops::Eq<First, F> && (ops::Eq<Rest, R> && ...) {
            return f == rhs.get_first() && r == rhs.get_rest();
        }

    public:
        static constexpr usize size() { return 1 + sizeof...(Rest); }

        constexpr First& get_first() {
            return f;
        }

        constexpr const First& get_first() const {
            return f;
        }

        constexpr auto& get_rest() {
            return r;
        }

        constexpr const auto& get_rest() const {
            return r;
        }

        template <usize pos>
        constexpr auto& get() noexcept
        requires (pos < size()) {
            if constexpr (pos == 0) {
                return f;
            } else {
                return r.template get<pos - 1>();
            }
        }

        template <usize pos>
        constexpr auto& get() const noexcept
        requires (pos < size()) {
            if constexpr (pos == 0) {
                return f;
            } else {
                return r.template get<pos - 1>();
            }
        }

        constexpr auto& first() noexcept requires (size() == 2) {
            return f;
        }

        constexpr const auto& first() const noexcept requires (size() == 2) {
            return f;
        }

        constexpr auto& second() noexcept requires (size() == 2) {
            return r.get_first();
        }

        constexpr const auto& second() const noexcept requires (size() == 2) {
            return r.get_first();
        }
    };

    template<class F, class ...R>
    Tuple(F&&, R&&...) -> Tuple<F, R...>;

    /**
     * 空元组, 或称"单位类型".
     * */
    template<>
    class Tuple<> {  // Unit type
    public:
        constexpr Tuple() = default;

        constexpr Tuple(const Tuple &) = default;

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
    constexpr ArgAtT<I, Arg, Args...> &get(Tuple<Arg, Args...> &tuple) {
        return tuple.template get<I>();
    }

    template<usize I, typename Arg, typename ...Args>
    constexpr const ArgAtT<I, Arg, Args...> &get(const Tuple<Arg, Args...> &tuple) {
        return tuple.template get<I>();
    }

    template<usize I, typename Arg, typename ...Args>
    constexpr ArgAtT<I, Arg, Args...> &&get(Tuple<Arg, Args...> &&tuple) {
        return std::move(tuple.template get<I>());
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
    constexpr Tuple<std::decay_t<T>, std::decay_t<Ts>...>
    make_tuple(T&& v, Ts&& ...vs) {
        return Tuple<std::decay_t<T>, std::decay_t<Ts>...>{v, vs...};
    }

    template<std::default_initializable T, std::default_initializable ...Ts>
    constexpr Tuple<T, Ts...>
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
    constexpr Tuple<T &, Ts &...>
    tie(T &v, Ts &...vs) {
        return Tuple<T &, Ts &...>{std::ref(v), std::ref(vs)...};
    }

    // Pair<T, U>
    template<typename T, typename U>
    using Pair = Tuple<T, U>;

    template<typename T, typename U>
    constexpr Pair<std::remove_reference_t<T>, std::remove_reference_t<U>> make_pair(T &&t, U &&u) {
        return make_tuple<std::decay_t<T>,
                std::decay_t<U>>(std::forward<T>(t), std::forward<U>(u));
    }
} // utility

#endif //MODERN_STL_TUPLE_H
