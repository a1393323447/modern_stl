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
                    : type(Ok) {
                construct(t);
            }
            constexpr ResultBase(T&& t) requires basic::Movable<T>
                    : type(Ok) {
                construct(std::forward<T>(t));
            }

            constexpr ResultBase(const E& e): type(Err) {
                construct(e);
            }
            constexpr ResultBase(E&& e): type(Err) {
                construct(std::forward<E>(e));
            }

            constexpr ResultBase(const ResultBase& r) noexcept requires basic::CopyAble<T> {
                copy_impl(r);
            }

            constexpr ResultBase(ResultBase&& r) noexcept requires basic::Movable<T>
                    : type(r.type) {
                move_impl(std::forward<ResultBase<T, E>>(r));
            }

            constexpr ~ResultBase() {
                if (type == Ok) {
                    destroy<T>();
                } else {
                    destroy<E>();
                }
            }

            constexpr ResultBase& operator=(ResultBase&& rhs) noexcept
                    requires basic::Movable<T> {
                if (this == &rhs) {
                    return *this;
                }
                if (this->type != rhs.type) {
                    if (this->is_ok()) {
                        this->template destroy<T>();
                    } else {
                        this->template destroy<E>();
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
                        this->template destroy<T>();
                    } else {
                        this->template destroy<E>();
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
                    return std::move(reinterpret_as<T>());
                } else {
                    MSTL_PANIC(err_ref_unchecked());
                }
            }

            constexpr E&& unwrap_err() {
                if (type == Err) {
                    return std::move(reinterpret_as<E>());
                } else {
                    MSTL_PANIC("Result is \"Ok\".");
                }
            }

            constexpr Option<T&> ok_ref() {
                if (type == Ok) {
                    return Option<T&>::some(reinterpret_as<T>());
                } else {
                    return Option<T&>::none();
                }
            }

            constexpr Option<const T&> ok_ref() const {
                if (type == Ok) {
                    return Option<const T&>::some(reinterpret_as<T>());
                } else {
                    return Option<T&>::none();
                }
            }

            constexpr Option<E> err() const {
                if (type == Err) {
                    return Option<E>::some(reinterpret_as<E>());
                } else {
                    return Option<E>::none();
                }
            }

            constexpr E err_unchecked() const {
                return reinterpret_as<E>();
            }

            constexpr Option<E&> err_ref() {
                if (type == Err) {
                    return Option<E&>::some(reinterpret_as<E>());
                } else {
                    return Option<E&>::none();
                }
            }

            constexpr Option<const E&> err_ref() const {
                if (type == Err) {
                    return Option<const E&>::some(reinterpret_as<E>());
                } else {
                    return Option<const E&>::none();
                }
            }

            constexpr T& ok_ref_unchecked() {
                return reinterpret_as<T>();
            }

            constexpr const T& ok_ref_unchecked() const {
                return reinterpret_as<T>();
            }

            constexpr E& err_ref_unchecked() {
                return reinterpret_as<E>();
            }

            constexpr const E& err_ref_unchecked() const {
                return reinterpret_as<E>();
            }

        protected:
            enum Type {
                Ok,
                Err
            } type;

            static constexpr usize max_size = std::max(
                    sizeof(T), sizeof(E)
            );

            u8 inner[max_size] = {0};

            template<typename In>
            constexpr void construct(In&& in)
            requires basic::OneOf<std::remove_cvref_t<In>, T, E> {
                auto ptr = reinterpret_cast<std::remove_cvref_t<In>*>(inner);
                std::construct_at(ptr, std::forward<In>(in));
            }

            template<basic::OneOf<T, E> In>
            constexpr void destroy() {
                auto ptr = reinterpret_cast<std::remove_cvref_t<In>*>(inner);
                std::destroy_at(ptr);
            }

            template<basic::OneOf<T, E> In>
            constexpr In& reinterpret_as() {
                auto ptr = reinterpret_cast<In*>(inner);
                return *ptr;
            }

            template<basic::OneOf<T, E> In>
            constexpr const In& reinterpret_as() const {
                auto ptr = reinterpret_cast<const In*>(inner);
                return *ptr;
            }

            constexpr ResultBase() = default;

        private:

            constexpr void move_impl(ResultBase&& r) requires basic::Movable<T> {
                type = r.type;
                if (r.type == Ok) {
                    auto rInner = reinterpret_cast<T*>(r.inner);
                    construct(std::move(*rInner));
                } else {
                    auto rInner = reinterpret_cast<E*>(r.inner);
                    construct(std::move(*rInner));
                }
            }

            constexpr void copy_impl(const ResultBase& r) requires basic::CopyAble<T> {
                type = r.type;
                if (is_ok()) {
                    construct(r.ok_ref_unchecked());
                } else {
                    construct(r.err_ref_unchecked());
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

            constexpr T unwrap() {
                if (this->is_ok())
                    return *this->template reinterpret_as<TPointer>();
                else
                    MSTL_PANIC(this->err_ref_unchecked());
            }

            constexpr Option<T&> ok_ref() {
                if (this->is_ok())
                    return Option<T&>::some(*this->template reinterpret_as<TPointer>());
                else
                    return Option<T&>::none();
            }

            constexpr Option<const T&> ok_ref() const {
                if (this->is_ok())
                    return Option<const T&>::some(*this->template reinterpret_as<TPointer>());
                else
                    return Option<const T&>::none();
            }

            constexpr T& ok_ref_unchecked() {
                return *this->template reinterpret_as<TPointer>();
            }

            constexpr const T& ok_ref_unchecked() const {
                return *this->template reinterpret_as<TPointer>();
            }
        };
    }
    template <typename T, basic::Error E>
    using Result = _private::ResultRef<T, E>;
}

#endif //MODERN_STL_RESULT_H
