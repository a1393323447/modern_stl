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

        template<typename T, mstl::basic::Error E>
        requires (!std::same_as<T, E> &&
                  !mstl::basic::RValRefType<T> &&
                  !mstl::basic::RefType<E>)
        class ResultBase {
        public:
            constexpr ResultBase(const T& t) requires basic::CopyAble<T>
                    : type(Ok), t(t) {
            }
            constexpr ResultBase(T&& t) requires basic::Movable<T>
                    : type(Ok), t(std::forward<T>(t)) {
            }

            constexpr ResultBase(const E& e): type(Err), e(e) {
            }

            constexpr ResultBase(E&& e): type(Err), e(std::forward<E>(e)) {
            }

            constexpr ResultBase(const ResultBase& r) noexcept requires basic::CopyAble<T>: type(r.type) {
                copy_impl(r);
            }

            constexpr ResultBase(ResultBase&& r) noexcept requires basic::Movable<T>
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

            constexpr ResultBase& operator=(ResultBase&& rhs) noexcept
                    requires basic::Movable<T> {
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

            constexpr ResultBase& operator=(const ResultBase& rhs) noexcept requires basic::CopyAble<T> {
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

            constexpr bool operator==(const ResultBase& rhs) const {
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

            constexpr inline bool is_ok() const {
                return type == Ok;
            }

            constexpr inline bool is_err() const {
                return type == Err;
            }

            // Comsume the Result, so that it's illegal after invoking this method
            constexpr T&& unwrap() {
                if (type == Ok) {
                    return std::move(t);
                } else {
                    MSTL_PANIC(err_ref_unchecked());
                }
            }

            constexpr E&& unwrap_err() {
                if (type == Err) {
                    return std::move(e);
                } else {
                    MSTL_PANIC("Result is \"Ok\".");
                }
            }

            constexpr Option<T&> ok_ref() {
                if (type == Ok) {
                    return Option<T&>::some(t);
                } else {
                    return Option<T&>::none();
                }
            }

            constexpr Option<const T&> ok_ref() const {
                if (type == Ok) {
                    return Option<const T&>::some(t);
                } else {
                    return Option<T&>::none();
                }
            }

            constexpr Option<E> err() const {
                if (type == Err) {
                    return Option<E>::some(e);
                } else {
                    return Option<E>::none();
                }
            }

            constexpr E err_unchecked() const {
                return e;
            }

            constexpr Option<E&> err_ref() {
                if (type == Err) {
                    return Option<E&>::some(e);
                } else {
                    return Option<E&>::none();
                }
            }

            constexpr Option<const E&> err_ref() const {
                if (type == Err) {
                    return Option<const E&>::some(e);
                } else {
                    return Option<const E&>::none();
                }
            }

            constexpr T& ok_ref_unchecked() {
                return t;
            }

            constexpr const T& ok_ref_unchecked() const {
                return t;
            }

            constexpr E& err_ref_unchecked() {
                return e;
            }

            constexpr const E& err_ref_unchecked() const {
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

            constexpr void move_impl(ResultBase&& r) requires basic::Movable<T> {
                if (r.type == Ok) {
                    std::construct_at(&t, std::move(r.t));
                } else {
                    std::construct_at(&e, std::move(r.e));
                }
            }

            constexpr void copy_impl(const ResultBase& r) requires basic::CopyAble<T> {
                if (r.type == Ok) {
                    std::construct_at(&t, r.t);
                } else {
                    std::construct_at(&e, r.e);
                }
            }
        };

        template<typename T, basic::Error E>
        class ResultRef: public ResultBase<T, E> {
            using Base = ResultBase<T, E>;
        public:
            using Base::ResultBase;

            constexpr ResultRef(ResultRef&& r) noexcept requires basic::Movable<T>
                    :Base(std::forward<ResultRef<T, E>>(r)) {}

            constexpr ResultRef(const ResultRef& r) noexcept requires basic::CopyAble<T>
                    :Base(r) {}

            constexpr ResultRef& operator=(ResultRef&& rhs) noexcept requires basic::Movable<T> {
                if (this == &rhs)
                    return *this;

                Base::operator=(std::forward<ResultRef>(rhs));
                return *this;
            }

            constexpr ResultRef& operator=(const ResultRef& rhs) noexcept requires basic::CopyAble<T> {
                if (this == &rhs)
                    return *this;

                Base::operator=(rhs);
                return *this;
            }
        };

        template<mstl::basic::LValRefType T, mstl::basic::Error E>
        class ResultRef<T, E>: public ResultBase<std::remove_reference_t<T>*, E> {
            using TPointer = std::remove_reference_t<T>*;
            using Base = ResultBase<TPointer , E>;
        public:
            constexpr ResultRef(T t): Base(&t) {}
            constexpr ResultRef(const E& e): Base(e) {}
            constexpr ResultRef(E&& e): Base(std::forward<E>(e)) {}
            constexpr ResultRef(const ResultRef& r): Base(r) {}

            constexpr ResultRef(ResultRef&& r) noexcept : Base(std::forward<Base>(r)) {}

            constexpr ResultRef& operator=(ResultRef&& rhs)  noexcept {
                if (this == &rhs)
                    return *this;

                Base::operator=(std::forward<ResultRef>(rhs));
                return *this;
            }

            constexpr ResultRef& operator=(const ResultRef& rhs) noexcept {
                if (this == &rhs)
                    return *this;

                Base::operator=(rhs);
                return *this;
            }

            constexpr bool operator==(const ResultRef& rhs) const {
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

            constexpr T unwrap() {
                if (this->is_ok())
                    return *Base::ok_ref_unchecked();
                else
                    MSTL_PANIC(this->err_ref_unchecked());
            }

            constexpr Option<T&> ok_ref() {
                if (this->is_ok())
                    return Option<T&>::some(*Base::ok_ref_unchecked());
                else
                    return Option<T&>::none();
            }

            constexpr Option<const T&> ok_ref() const {
                if (this->is_ok())
                    return Option<const T&>::some(*Base::ok_ref_unchecked());
                else
                    return Option<const T&>::none();
            }

            constexpr T& ok_ref_unchecked() {
                return *Base::ok_ref_unchecked();
            }

            constexpr const T& ok_ref_unchecked() const {
                return *Base::ok_ref_unchecked();
            }
        };
    }
    template <typename T, basic::Error E>
    using Result = _private::ResultRef<T, E>;
}

#endif //MODERN_STL_RESULT_H
