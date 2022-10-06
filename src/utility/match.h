//
// Created by Shiroan on 2022/10/6.
//

#ifndef MODERN_STL_MATCH_H
#define MODERN_STL_MATCH_H

#include <ops/ops.h>
#include <collection/linked_list.h>
#include <option/option.h>

namespace mstl::utility {
    /**
     *
     * ## Example
     * @code
     *      i32 x = 15;
     *      auto y = match<i32>(x)
     *              .when(1, [](){return 5;})
     *              .when(ops::Range(2, 10), [](){return 10;})
     *              .rest([](){return 20;})
     *              .go();
     *
     *      assert(y == 20);
     * @endcode
     *
     * @tparam T 需要匹配的元素类型
     * @tparam R 返回值类型
     * @tparam Complete 标记是否已经涉及所有分支
     */
    template <typename T, typename R, bool Complete=false>
    class Match {
        // todo operator|
        const T& target;
        Option<R> res = Option<R>::none();

        template<class T1, class T2>
        friend Match<T2, T1> match(const T2& t);

    public:
        /**
         *
         * @tparam U
         * @tparam F
         * @param u
         * @param fun
         * @return
         */
        template<typename U, typename F>
        constexpr Match& when(const U& u, F fun)
        requires ops::Eq<T, U> && ops::Callable<F, R> {
            if (res.is_none() && u == target) {
                res = Option<R>::some(fun());
            }
            return *this;
        }

        template<typename U, typename F>
        constexpr Match& when(const ops::Range<U>& range, F fun)
        requires ops::PartialOrd<T, U> && ops::Callable<F, R> {
            if (res.is_none() && range.template contains(target)) {
                res = Option<R>::some(fun());
            }

            return *this;
        }

        template<typename F>
        constexpr Match<T, R, true> rest(F fun) {
            if (res.is_none()) {
                res = Option<R>::some(fun());
            }

            return {std::move(res)};
        }

    private:
        Match(const T &target) : target(target) {}
    };

    template<typename T, typename R>
    class Match<T, R, true> {
        friend class Match<T, R, false>;

        Option<R> res = Option<R>::none();

    public:
        R go() {
            return res.unwrap_uncheck();
        }

    private:
        Match(Option <R> &&res) : res(res) {}
    };

    template <typename R, typename T>
    Match<T, R> match(const T& t) {
        return Match<T, R>{t};
    }
}

#endif //MODERN_STL_MATCH_H
