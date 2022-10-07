//
// Created by Shiroan on 2022/10/6.
//

#ifndef MODERN_STL_MATCH_H
#define MODERN_STL_MATCH_H

#include <ops/ops.h>
#include <collection/linked_list.h>
#include <option/option.h>

namespace mstl::utility {
    template <typename T, typename R>
    class MatchMid;
    /**
     * @brief 值匹配工具.
     *
     * 对需匹配的值t, 依次判断是否满足条件. 若满足, 则返回分支函数的值.
     *
     * t满足某一条件后, 便不再检查其是否满足后续条件.
     *
     * ## Example
     * @code
     *      i32 x = 7;
     *      auto y = match(x)
     *              .when(1, [](){return 5;})                   // 失配
     *              .when(ops::Range(2, 10), [](){return 10;})  // 匹配
     *              .rest([](){return 20;})                     // 不再检查
     *              .finale();
     *
     *      assert(y == 10);
     *
     *      auto z = match(x)
     *             | when(1, [](){return 5;})
     *             | when(ops::Range(2, 10), [](){return 10;})
     *             | rest([](){return 20;})
     *             | finale();
     *
     *      assert(z == 10);
     * @endcode
     *
     * @tparam T 需要匹配的元素类型
     */
    template <typename T>
    class Match {
        const T& target;

        template<class T1>
        constexpr friend Match<T1> match(const T1& t);

    public:
        /**
         * @brief 等值匹配.
         *
         * 当值等于u时, 执行分支函数.
         *
         * ## 约束
         * 对F的实例f, 必须满足表达式f().
         *
         * U必须可与T判等.
         *
         * @tparam U 需要判等的值的类型.
         * @tparam F 分支函数类型.
         * @param u 与匹配对象判等的值.
         * @param fun 分支函数.
         */
        template<typename U, typename F>
        constexpr auto when(const U& u, F fun) -> MatchMid<T, std::invoke_result_t<F>>
        requires ops::Eq<T, U> && std::invocable<F> {
            using R = std::invoke_result_t<F>;
            if (u == target) {
                return MatchMid<T, R>{target, fun};
            }
            return MatchMid<T, R>(target);
        }

        /**
         * @brief 范围匹配.
         *
         * 当值处于范围内时, 执行分支函数.
         *
         * ## 约束
         * U与T必须满足*可偏序比较*.
         *
         * 对F的实例f, 必须满足表达式f().
         *
         * @tparam U 范围所迭代的类型
         * @tparam F 分支函数类型.
         * @param range 范围
         * @param fun 分支函数
         */
        template<typename U, typename F>
        constexpr auto when(const ops::Range<U>& range, F fun) -> MatchMid<T, std::invoke_result_t<F>>
        requires ops::PartialOrd<T, U> && std::invocable<F> {
            using R = std::invoke_result_t<F>;
            if (range.template contains(target)) {
                return MatchMid<T, R>{target, fun};
            }

            return MatchMid<T, R>(target);
        }

        /**
         * @brief 余项匹配.
         *
         * 当此前定义的条件均不满足需匹配的值时, 执行分支函数fun.
         */
        template<typename F>
        constexpr auto rest(F fun) -> MatchMid<T, std::invoke_result_t<F>>
        requires std::invocable<F>{
            return {target, fun()};
        }

    private:
        constexpr Match(const T &target) : target(target) {}
    };

    template <typename T, typename R>
    class MatchMid {
        const T& target;
        Option<R> res = Option<R>::none();

        friend class Match<T>;

    public:
        /**
         * @brief 等值匹配.
         *
         * 当值等于u时, 执行分支函数.
         *
         * ## 约束
         * 对F的实例f, 必须满足表达式f().
         *
         * U必须可与T判等.
         *
         * @tparam U 需要判等的值的类型.
         * @tparam F 分支函数类型.
         * @param u 与匹配对象判等的值.
         * @param fun 分支函数.
         */
        template<typename U, typename F>
        constexpr MatchMid& when(const U& u, F fun)
        requires ops::Eq<T, U> && ops::Callable<F, R> {
            if (res.is_none() && u == target) {
                res = Option<R>::some(fun());
            }
            return *this;
        }

        /**
         * @brief 范围匹配.
         *
         * 当值处于范围内时, 执行分支函数.
         *
         * ## 约束
         * U与T必须满足*可偏序比较*.
         *
         * 对F的实例f, 必须满足表达式f().
         *
         * @tparam U 范围所迭代的类型
         * @tparam F 分支函数类型.
         * @param range 范围
         * @param fun 分支函数
         */
        template<typename U, typename F>
        constexpr MatchMid& when(const ops::Range<U>& range, F fun)
        requires ops::PartialOrd<T, U> && ops::Callable<F, R> {
            if (res.is_none() && range.template contains(target)) {
                res = Option<R>::some(fun());
            }

            return *this;
        }

        /**
         * @brief 余项匹配.
         *
         * 当此前定义的条件均不满足需匹配的值时, 执行分支函数fun.
         */
        template<typename F>
        constexpr MatchMid& rest(F fun) {
            if (res.is_none()) {
                res = Option<R>::some(fun());
            }

            return *this;
        }

        /**
         * 返回结果值. 若目标未能触及分支, 则引发panic; 若该函数在常量求值语境下调用, 则编译失败.
         */
        constexpr R finale() {
            return res.unwrap();
        }

    private:
        template<class F>
        constexpr MatchMid(const T& t, F fun) requires ops::Callable<F, R>
        : target(t), res(Option<R>::some(fun())) {}

        constexpr MatchMid(const T& t): target(t) {}
    };

    template<typename T>
    class MatchMid<T, void> {
        const T& target;
        bool reach = false;

    public:
        template<typename U, typename F>
        constexpr MatchMid& when(const U& u, F fun)
        requires ops::Eq<T, U> && ops::Callable<F, void> {
            if (!reach && u == target) {
                reach = true;
                fun();
            }
            return *this;
        }

        template<typename U, typename F>
        constexpr MatchMid& when(const ops::Range<U>& range, F fun)
        requires ops::PartialOrd<T, U> && ops::Callable<F, void> {
            if (!reach && range.template contains(target)) {
                fun();
            }

            return *this;
        }

        template<typename F>
        constexpr MatchMid& rest(F fun) {
            if (!reach) {
                fun();
            }

            return *this;
        }

        constexpr void finale() {
            if (!reach) {
                MSTL_PANIC("Failed to cover all branches.");
            }
        }

    private:
        template<class F>
        constexpr MatchMid(const T& t, F fun) requires ops::Callable<F, void>: target(t), reach(true) {
            fun();
        }

        constexpr MatchMid(const T& t): target(t) {}
    };

    /**
     * 生成Match对象.
     * @param t 需匹配的值
     */
    template <typename T>
    constexpr Match<T> match(const T& t) {
        return Match<T>{t};
    }

    template <typename U, typename F>
    requires std::invocable<F>
    struct When {
        using R = std::invoke_result_t<F>;

        F fun;
        const U& u;
    };

    /**
     * 生成When对象. 为管道风格调用时使用.
     */
    template<typename U, typename F>
    constexpr When<U, F> when(const U& u, F fun) {
        return {fun, u};
    }

    template <typename F>
    requires std::invocable<F>
    struct Rest {
        using R = std::invoke_result_t<F>;

        F fun;
    };

    /**
     * 生成Rest对象. 为管道风格调用时使用.
     */
    template<typename F>
    constexpr Rest<F> rest(F fun) {
        return {fun};
    }

    struct Finale {};

    /**
     * 生成Finale对象. 为管道风格调用时使用.
     */
    constexpr Finale finale() {
        return {};
    }

    template<typename U, typename F>
    constexpr decltype(auto) operator|(auto&& match, const When<U, F>& w) {
            return match.template when(w.u, w.fun);
    }

    template<typename F>
    constexpr decltype(auto) operator|(auto&& match, const Rest<F>& r) {
        return match.template rest(r.fun);
    }

    template<typename T, typename R>
    constexpr R operator|(MatchMid<T, R>& match, Finale) {
        return match.finale();
    }
}

#endif //MODERN_STL_MATCH_H
