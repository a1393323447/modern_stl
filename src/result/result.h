//
// Created by Shiroan on 2022/9/20.
//

#ifndef MODERN_STL_RESULT_H
#define MODERN_STL_RESULT_H

#include <basic_concepts.h>
#include <global.h>
#include "option/option.h"

namespace mstl::result {

    namespace _private {

        /**
         * @brief `Result`的基类.
         *
         * @tparam T 表示成功的返回值类型
         * @tparam E 表示失败的返回值类型
         * @sa ResultRef
         */
        template<typename T, mstl::basic::Error E> requires (!std::same_as<T, E> &&
                                                             !mstl::basic::RefType<T> &&
                                                             !mstl::basic::RefType<E>)
        class ResultBase {
        public:
            /// 从t复制构造一个`Ok`型`Result`
            constexpr ResultBase(const T &t) requires basic::CopyAble<T>
                    : type(Ok), t(t) {
            }

            /// 从t移动构造一个`Ok`型`Result`
            constexpr ResultBase(T &&t) requires basic::Movable<T>
                    : type(Ok), t(std::forward<T>(t)) {
            }

            /// 从e复制构造一个`Err`型`Result`
            constexpr ResultBase(const E &e) : type(Err), e(e) {
            }

            /// 从e移动构造一个`Err`型`Result`
            constexpr ResultBase(E &&e) : type(Err), e(std::forward<E>(e)) {
            }

            constexpr ResultBase(const ResultBase &r) noexcept requires basic::CopyAble<T>: type(r.type) {
                copy_impl(r);
            }

            constexpr ResultBase(ResultBase &&r) noexcept requires basic::Movable<T>
                    : type(r.type) {
                move_impl(std::forward<ResultBase<T, E>>(r));
            }

            constexpr ~ResultBase() {
                if (type == Ok) {
                    destroy_t();
                } else {
                    destroy_e();
                }
            }

            constexpr ResultBase &operator=(ResultBase &&rhs) noexcept requires basic::Movable<T> {
                if (this == &rhs) {
                    return *this;
                }
                if (this->type != rhs.type) {
                    if (this->is_ok()) {
                        destroy_t();
                    } else {
                        destroy_e();
                    }
                    move_impl(std::forward<ResultBase<T, E>>(rhs));
                } else {
                    if (this->is_ok()) {
                        this->ok_ref_unchecked() = rhs.unwrap();
                    } else {
                        this->err_ref_unchecked() = rhs.unwrap_err();
                    }
                }

                return *this;
            }

            constexpr ResultBase &operator=(const ResultBase &rhs) noexcept requires basic::CopyAble<T> {
                if (this == &rhs) {
                    return *this;
                }
                if (this->type != rhs.type) {  // destroy holding object of self
                    if (this->is_ok()) {
                        destroy_t();
                    } else {
                        destroy_e();
                    }
                    copy_impl(rhs);
                } else {
                    if (this->is_ok()) {
                        this->ok_ref_unchecked() = rhs.ok_ref_unchecked();
                    } else {
                        this->err_ref_unchecked() = rhs.err_ref_unchecked();
                    }
                }
                return *this;
            }

            constexpr bool operator==(const ResultBase &rhs) const {
                if (type != rhs.type) {
                    return false;
                } else {
                    if (type == Ok) {
                        return ok_ref_unchecked() == rhs.ok_ref_unchecked();
                    } else {
                        return err_ref_unchecked() == rhs.err_ref_unchecked();
                    }
                }
            }

            /**
             * 从`args`构造`Ok`型`Result`.
             *
             * ## Example
             * @code
             *      auto res_ok = Result<int, std::string>::ok(1);
             *      assert(res_ok.is_ok());
             *      assert(res_ok.ok_unchecked() == 1);
             * @endcode
             */
            template<class ...Args>
            constexpr static ResultBase ok(Args &&...args)requires (sizeof...(Args) != 0) {
                static_assert(std::constructible_from<T, Args...>, "Ok-type T cannot be construct from the arguments.");
                return ResultBase(T{std::forward<Args>(args)...});
            }

            /**
             * 从`args`构造`Err`型`Result`.
             *
             * ## Example
             * @code
             *      auto res_err = Result<int, std::string>::err("Error");
             *      assert(res_err.is_err());
             *      assert(res_err.err_unchecked() == "Error");
             * @endcode
             */
            template<class ...Args>
            constexpr static ResultBase err(Args &&...args)requires (sizeof...(Args) != 0) {
                static_assert(std::constructible_from<E, Args...>,
                              "Err-type E cannot be construct from the arguments.");
                return ResultBase(E{std::forward<Args>(args)...});
            }

        public:
            /**
             * 检查当前`Result`是否为`Ok`型.
             * @return 若当前`Result`是`Ok`型, 则返回`true`; 否则, 返回`false`.
             *
             * ## Example
             * @code
             *      Result<int, double> res1(1), res2(2.0);
             *
             *      assert( res1.is_ok());
             *      assert(!res2.is_ok());
             * @endcode
             */
            constexpr inline bool is_ok() const {
                return type == Ok;
            }

            /**
             * 检查当前`Result`是否为`Err`型.
             * @return 若当前`Result`是`Err`型, 则返回`true`; 否则, 返回`false`.
             *
             * ## Example
             * @code
             *      Result<int, double> res1(1), res2(2.0);
             *      assert(!res1.is_err());
             *      assert( res2.is_err());
             * @endcode
             */
            constexpr inline bool is_err() const {
                return type == Err;
            }

            /**
             * @brief 解包并返回表示成功的值.
             *
             * 若该`Result`为`Ok`型, 则消耗该`Result`对象, 并返回其中表示成功的值; 否则, 程序将引发`panic`.
             *
             * @note 调用该函数后, 该`Result`将被消耗(或视为已移动), 因此继续使用该`Result`应视为未定义行为.
             * @return 表示成功的值
             * @sa unwrap_unchecked(), unwrap_err(), unwrap_err_unchecked()
             *
             * ## Example
             * @code
             *      Result<std::string, int> res1("foo"), res2(2);
             *      assert(res1.unwrap() == "foo");
             *      // res1.unwrap();  // res1 is comsumed.
             *      // res2.unwrap();  // res2 is an err, so that it will raise a panic invoking this method.
             * @endcode
             */
            constexpr T &&unwrap() {
                if (type == Ok) {
                    return std::move(t);
                } else {
                    MSTL_PANIC(err_ref_unchecked());
                }
            }

            /**
             * @brief 解包并返回表示成功的值, 且不检查对象的合法性.
             *
             * 若该`Result`为`Ok`型, 则消耗该`Result`对象, 并返回其中表示成功的值; 否则, 行为未定义.
             *
             * @note 调用该函数后, 该`Result`将被消耗(或视为已移动), 因此继续使用该`Result`应视为未定义行为.
             * @return 表示成功的值
             * @sa unwrap(), unwrap_err(), unwrap_err_unchecked()
             *
             * ## Example
             * @code
             *      Result<std::string, int> res1("foo"), res2(2);
             *      assert(res1.unwrap_unchecked() == "foo");
             *      // res1.unwrap_unchecked();  // res1 is comsumed.
             *      // res2.unwrap_unchecked();  // res2 is an err, so that it is a UB to invoke this method.
             * @endcode
             */
            constexpr T &&unwrap_unchecked() {
                return std::move(t);
            }

            /**
             * @brief 解包并返回表示失败的值.
             *
             * 若该`Result`为`Err`型, 则消耗该`Result`对象, 并返回其中表示错误的值; 否则, 程序将引发`panic`.
             * @return 表示错误的值
             * @sa unwrap(), unwrap_unchecked(), unwrap_err_unchecked()
             * ## Example
             * @code
             *      Result<std::string, int> res1("foo"), res2(2);
             *      assert(res2.unwrap_err() == 2);
             *      // res2.unwrap_err();  // res1 is comsumed.
             *      // res1.unwrap_err();  // res1 is an ok, so that it will raise a panic invoking this method.
             * @endcode
             */
            constexpr E &&unwrap_err() {
                if (type == Err) {
                    return std::move(e);
                } else {
                    MSTL_PANIC("Result is \"Ok\".");
                }
            }

            /**
             * @brief 解包并返回表示失败的值, 且不检查对象的合法性.
             *
             * 若该`Result`为`Err`型, 则消耗该`Result`对象, 并返回其中表示错误的值; 否则, 行为未定义.
             * @return 表示错误的值
             * @sa unwrap(), unwrap_err(), unwrap_unchecked()
             * ## Example
             * @code
             *      Result<std::string, int> res1("foo"), res2(2);
             *      assert(res2.unwrap_err() == 2);
             *      // res2.unwrap_err();  // res1 is comsumed.
             *      // res1.unwrap_err();  // res1 is an ok, so that it will raise a panic invoking this method.
             * @endcode
             */
            constexpr E &&unwrap_err_unchecked() {
                return std::move(e);
            }

            /**
             * @brief 返回可能的表示正确的值的副本.
             *
             * @return 若该`Result`为`Ok`型, 则返回`Some(t)`; 否则, 返回`None`.
             * @sa ok_unchecked(), ok_ref(), ok_ref_unchecked()
             *
             * ## Example
             * @code
             *      Result<std::string, int> res1("foo"), res2(2);
             *      assert(res1.ok().unwrap() == "foo");
             *      assert(res2.ok().is_none());
             * @endcode
             */
            constexpr Option<T> ok() requires basic::CopyAble<T> {
                if (type == Ok) {
                    return Option<T>::some(t);
                } else {
                    return Option<T>::none();
                }
            }

            /**
             * @brief 返回表示正确的值的副本.
             *
             * @return 若该`Result`为`Ok`型, 则返回`t`; 否则, 行为未定义.
             * @sa ok(), ok_ref(), ok_ref_unchecked()
             */
            constexpr T ok_unchecked() requires basic::CopyAble<T> {
                return t;
            }

            /**
             * @brief 返回可能的表示正确的值的引用.
             *
             * @return 若该`Result`为`Ok`型, 则返回`Some(*&t)`; 否则, 返回`None`.
             * @sa ok(), ok_unchecked(), ok_ref_unchecked()
             */
            constexpr Option<T &> ok_ref() {
                if (type == Ok) {
                    return Option<T &>::some(t);
                } else {
                    return Option<T &>::none();
                }
            }

            constexpr Option<const T &> ok_ref() const {
                if (type == Ok) {
                    return Option<const T &>::some(t);
                } else {
                    return Option<T &>::none();
                }
            }

            /**
             * @brief 返回可能的表示错误的值的副本.
             *
             * @return 若该`Result`为`Err`型, 则返回`Some(e)`; 否则, 返回`None`.
             * @sa err_unchecked(), err_ref(), err_ref_unchecked()
             */
            constexpr Option<E> err() const {
                if (type == Err) {
                    return Option<E>::some(e);
                } else {
                    return Option<E>::none();
                }
            }

            /**
             * @brief 返回表示错误的值的副本.
             *
             * @return 若该`Result`为`Err`型, 则返回`e`; 否则, 行为未定义.
             * @sa err(), err_ref(), err_ref_unchecked()
             */
            constexpr E err_unchecked() const {
                return e;
            }

            /**
             * @brief 返回可能的表示错误的值的引用.
             *
             * @return 若该`Result`为`Err`型, 则返回`Some(*&e)`; 否则, 返回`None`.
             * @sa err(), err_unchecked(), err_ref_unchecked()
             */
            constexpr Option<E &> err_ref() {
                if (type == Err) {
                    return Option<E &>::some(e);
                } else {
                    return Option<E &>::none();
                }
            }

            constexpr Option<const E &> err_ref() const {
                if (type == Err) {
                    return Option<const E &>::some(e);
                } else {
                    return Option<const E &>::none();
                }
            }

            /**
             * @brief 返回表示正确的值的引用.
             *
             * @return 若该`Result`为`Ok`型, 则返回`*&t`; 否则, 行为未定义.
             * @sa ok(), ok_unchecked(), ok_ref()
             */
            constexpr T &ok_ref_unchecked() {
                return t;
            }

            constexpr const T &ok_ref_unchecked() const {
                return t;
            }

            /**
             * @brief 返回表示错误的值的引用.
             *
             * @return 若该`Result`为`Err`型, 则返回`*&e`; 否则, 行为未定义.
             * @sa err_unchecked(), err_ref(), err_ref_unchecked()
             */
            constexpr E &err_ref_unchecked() {
                return e;
            }

            constexpr const E &err_ref_unchecked() const {
                return e;
            }

        protected:
            enum Type {
                Ok,
                Err
            } type;

            union {
                T t;
                E e;
            };

            constexpr void destroy_t() {
                if constexpr (std::is_class_v<T>) {
                    t.~T();
                }
            }

            constexpr void destroy_e() {
                if constexpr (std::is_class_v<E>) {
                    e.~E();
                }
            }

            constexpr ResultBase() = default;

        private:

            constexpr void move_impl(ResultBase &&r) requires basic::Movable<T> {
                if (r.type == Ok) {
                    std::construct_at(&t, std::move(r.t));
                } else {
                    std::construct_at(&e, std::move(r.e));
                }
            }

            constexpr void copy_impl(const ResultBase &r) requires basic::CopyAble<T> {
                if (r.type == Ok) {
                    std::construct_at(&t, r.t);
                } else {
                    std::construct_at(&e, r.e);
                }
            }
        };

        template<typename T, basic::Error E>
        class ResultRef : public ResultBase<T, E> {
            using Base = ResultBase<T, E>;
        public:
            using Base::ResultBase;

            constexpr ResultRef(ResultRef &&r) noexcept requires basic::Movable<T>
                    : Base(std::forward<ResultRef<T, E>>(r)) {}

            constexpr ResultRef(const ResultRef &r) noexcept requires basic::CopyAble<T>
                    : Base(r) {}

            constexpr ResultRef &operator=(ResultRef &&rhs) noexcept requires basic::Movable<T> {
                if (this == &rhs)
                    return *this;

                Base::operator=(std::forward<ResultRef>(rhs));
                return *this;
            }

            constexpr ResultRef &operator=(const ResultRef &rhs) noexcept requires basic::CopyAble<T> {
                if (this == &rhs)
                    return *this;

                Base::operator=(rhs);
                return *this;
            }
        };

        /**
         * 储存引用的`Result`.
         *
         * @tparam T 表示正确的值的左值引用类型
         * @tparam E 表示错误的值
         * @sa ResultBase
         */
        template<mstl::basic::LValRefType T, mstl::basic::Error E>
        class ResultRef<T, E> : public ResultBase<std::remove_reference_t<T> *, E> {
            using TPointer = std::remove_reference_t<T> *;
            using Base = ResultBase<TPointer, E>;
        public:
            constexpr ResultRef(T t) : Base(&t) {}

            constexpr ResultRef(const E &e) : Base(e) {}

            constexpr ResultRef(E &&e) : Base(std::forward<E>(e)) {}

            constexpr ResultRef(const ResultRef &r) : Base(r) {}

            constexpr ResultRef(ResultRef &&r) noexcept: Base(std::forward<Base>(r)) {}

            constexpr ResultRef &operator=(ResultRef &&rhs) noexcept {
                if (this == &rhs)
                    return *this;

                Base::operator=(std::forward<ResultRef>(rhs));
                return *this;
            }

            constexpr ResultRef &operator=(const ResultRef &rhs) noexcept {
                if (this == &rhs)
                    return *this;

                Base::operator=(rhs);
                return *this;
            }

            constexpr bool operator==(const ResultRef &rhs) const {
                if (this->type != rhs.type) {
                    return false;
                } else {
                    if (this->is_ok()) {
                        return ok_ref_unchecked() == rhs.ok_ref_unchecked();
                    } else {
                        return this->err_ref_unchecked() == rhs.err_ref_unchecked();
                    }
                }
            }

            constexpr static ResultRef ok(T t) {
                return ResultRef(t);
            }

        public:
            constexpr T unwrap() {
                if (this->is_ok())
                    return *Base::ok_ref_unchecked();
                else
                    MSTL_PANIC(this->err_ref_unchecked());
            }

            constexpr Option<T &> ok() {
                return ok_ref();
            }

            constexpr Option<const T &> ok() const {
                return ok_ref();
            }

            constexpr T &ok_unchecked() {
                return ok_ref_unchecked();
            }

            constexpr const T &ok_unchecked() const {
                return ok_ref_unchecked();
            }

            constexpr Option<T &> ok_ref() {
                if (this->is_ok())
                    return Option<T &>::some(*Base::ok_ref_unchecked());
                else
                    return Option<T &>::none();
            }

            constexpr Option<const T &> ok_ref() const {
                if (this->is_ok())
                    return Option<const T &>::some(*Base::ok_ref_unchecked());
                else
                    return Option<const T &>::none();
            }

            constexpr T &ok_ref_unchecked() {
                return *Base::ok_ref_unchecked();
            }

            constexpr const T &ok_ref_unchecked() const {
                return *Base::ok_ref_unchecked();
            }
        };
    }

    /**
     * @brief 返回一个潜在的错误.
     *
     * `Result`表示一个可能失败的函数的结果, 它要么是成功(Ok)的(此时它表示函数的执行结果), 要么是失败(Err)的(此时它表示一个错误). 它相当于一个二元Variant.
     *
     * `MSTL`希望以`Result`彻底取代标准C++中的异常机制.
     *
     * @tparam T 成功的返回值类型
     * @tparam E 失败的返回值类型
     *
     * @todo 在实现variant后, 可复用variant的逻辑
     * @sa ResuktBase, ResultRef
     */
    template<typename T, basic::Error E>
    using Result = _private::ResultRef<T, E>;
}

#endif //MODERN_STL_RESULT_H
