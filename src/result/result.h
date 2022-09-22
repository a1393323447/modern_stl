//
// Created by Shiroan on 2022/9/20.
//

#ifndef MODERN_STL_RESULT_H
#define MODERN_STL_RESULT_H

#include <basic_concepts.h>
#include <global.h>
#include "option/option.h"

namespace mstl::result {
    using mstl::basic::Error;
    using mstl::basic::LValRefType;
    using mstl::basic::RValRefType;

    namespace _private {
        // Substitute E as reference types is forbiden.
        template<typename T, Error E> requires (!std::same_as<T, E> && !RValRefType<T>)
        class ResultBase {
        public:
            ResultBase(const T& t): type(Ok) {
                construct(t);
            }
            ResultBase(T&& t): type(Ok) {
                construct(std::forward<T>(t));
            }

            ResultBase(const E& e): type(Err) {
                construct(e);
            }
            ResultBase(E&& e): type(Err) {
                construct(std::forward<E>(e));
            }

            ResultBase(ResultBase&& r) noexcept : type(r.type) {
                move_impl(std::forward<ResultBase<T, E>>(r));
            }

            ~ResultBase() {
                if (type == Ok) {
                    destroy<T>();
                } else {
                    destroy<E>();
                }
            }

            ResultBase& operator=(ResultBase&& rhs)  noexcept {
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

             bool operator==(const ResultBase& rhs) const {
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

            inline bool is_ok() const {
                return type == Ok;
            }

            inline bool is_err() const {
                return type == Err;
            }

            // Comsume the Result, so that it's illegal after invoking this method
            T&& unwrap() {
                if (type == Ok) {
                    return std::move(reinterpret_as<T>());
                } else {
                    MSTL_PANIC(err_ref_unchecked());
                }
            }

            E&& unwrap_err() {
                if (type == Err) {
                    return std::move(reinterpret_as<E>());
                } else {
                    MSTL_PANIC("Result is \"Ok\".");
                }
            }

            Option<T&> ok_ref() {
                if (type == Ok) {
                    return Option<T&>::some(reinterpret_as<T>());
                } else {
                    return Option<T&>::none();
                }
            }

            Option<const T&> ok_ref() const {
                if (type == Ok) {
                    return Option<const T&>::some(reinterpret_as<T>());
                } else {
                    return Option<T&>::none();
                }
            }

            Option<E> err() const {
                if (type == Err) {
                    return Option<E>::some(reinterpret_as<E>());
                } else {
                    return Option<E>::none();
                }
            }

            E err_unchecked() const {
                return reinterpret_as<E>();
            }

            Option<E&> err_ref() {
                if (type == Err) {
                    return Option<E&>::some(reinterpret_as<E>());
                } else {
                    return Option<E&>::none();
                }
            }

            Option<const E&> err_ref() const {
                if (type == Err) {
                    return Option<const E&>::some(reinterpret_as<E>());
                } else {
                    return Option<const E&>::none();
                }
            }

            T& ok_ref_unchecked() {
                return reinterpret_as<T>();
            }

            const T& ok_ref_unchecked() const {
                return reinterpret_as<T>();
            }

            E& err_ref_unchecked() {
                return reinterpret_as<E>();
            }

            const E& err_ref_unchecked() const {
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
            void construct(In&& in)
            requires basic::OneOf<std::remove_cvref_t<In>, T, E> {
                auto ptr = reinterpret_cast<std::remove_cvref_t<In>*>(inner);
                std::construct_at(ptr, std::forward<In>(in));
            }

            template<basic::OneOf<T, E> In>
            void destroy() {
                auto ptr = reinterpret_cast<std::remove_cvref_t<In>*>(inner);
                std::destroy_at(ptr);
            }

            template<basic::OneOf<T, E> In>
            In& reinterpret_as() {
                auto ptr = reinterpret_cast<In*>(inner);
                return *ptr;
            }

            template<basic::OneOf<T, E> In>
            const In& reinterpret_as() const {
                auto ptr = reinterpret_cast<const In*>(inner);
                return *ptr;
            }

            ResultBase() = default;

        private:
            void move_impl(ResultBase&& r) {
                type = r.type;
                if (r.type == Ok) {
                    auto rInner = reinterpret_cast<T*>(r.inner);
                    construct(std::move(*rInner));
                } else {
                    auto rInner = reinterpret_cast<E*>(r.inner);
                    construct(std::move(*rInner));
                }
            }
        };

        template<typename T, Error E>
        class ResultRef: public ResultBase<T, E>{
            using Base = ResultBase<T, E>;
        public:
            using Base::ResultBase;

            ResultRef(ResultRef&& r) noexcept : Base(std::forward<Base>(r)) {}

            ResultRef& operator=(ResultRef&& rhs) noexcept {
                if (this == &rhs)
                    return *this;

                Base::operator=(std::forward<ResultRef>(rhs));
                return *this;
            }
        protected:
            using UnderlyingType = T;
            using ConstUnderlyingType = const T;

            UnderlyingType& get_underlying_object(){
                return Base::ok_ref_unchecked();
            }

            ConstUnderlyingType& get_underlying_object() const{
                return Base::ok_ref_unchecked();
            }

            ResultRef(): Base() {}
        };

        template<LValRefType T, Error E>
        class ResultRef<T, E>: public ResultBase<std::remove_reference_t<T>*, E>{
            using TPointer = std::remove_reference_t<T>*;
            using Base = ResultBase<TPointer , E>;
        public:
            ResultRef(T t): Base(&t) {}
            ResultRef(const E& e): Base(e) {}
            ResultRef(E&& e): Base(std::forward<E>(e)) {}
            ResultRef(const ResultRef& r): Base(r) {}

            ResultRef(ResultRef&& r) noexcept : Base(std::forward<Base>(r)) {}

            ResultRef& operator=(ResultRef&& rhs)  noexcept {
                if (this == &rhs)
                    return *this;

                Base::operator=(std::forward<ResultRef>(rhs));
                return *this;
            }

            T unwrap() {
                if (this->is_ok())
                    return *this->template reinterpret_as<TPointer>();
                else
                    MSTL_PANIC(this->err_ref_unchecked());
            }

            Option<T&> ok_ref() {
                if (this->is_ok())
                    return Option<T&>::some(*this->template reinterpret_as<TPointer>());
                else
                    return Option<T&>::none();
            }

            Option<const T&> ok_ref() const {
                if (this->is_ok())
                    return Option<const T&>::some(*this->template reinterpret_as<TPointer>());
                else
                    return Option<const T&>::none();
            }

            T& ok_ref_unchecked() {
                return *this->template reinterpret_as<TPointer>();
            }

            const T& ok_ref_unchecked() const {
                return *this->template reinterpret_as<TPointer>();
            }

        protected:
            using UnderlyingType = TPointer;
            // Get remove_reference_t<T>* (aka TPointer) underlying. Assuming this Result is Ok.
            TPointer get_underlying_object() const{
                return Base::ok_ref_unchecked();
            }
            ResultRef(): Base() {}
        };
    }

    template<typename T, Error E>
    class Result: public _private::ResultRef<T, E> {
        using Base = _private::ResultRef<T, E>;
    public:
        using Base::ResultRef;
        Result(Result&& r) noexcept : Base(std::forward<Base>(r)) {}
        Result() = delete;

        Result& operator=(Result&& rhs)  noexcept {
            Base::operator=(std::forward<Result>(rhs));
            return *this;
        }
    private:
    };

    template<typename T, Error E>
    requires basic::CopyAble<T> || LValRefType<T>
    class Result<T, E> final: public _private::ResultRef<T, E> {
        using Base = _private::ResultRef<T, E>;
    public:
        Result(const Result& r): Base() {
            copy_impl(r);
        }
        Result& operator=(const Result& rhs) {
            if (this == &rhs) {
                return *this;
            }
            if (this->type != rhs.type) {
                if (this->is_ok()) {
                    this->template destroy<typename Base::UnderlyingType>();
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

        Result& operator=(Result&& rhs)  noexcept {
            Base::operator=(std::forward<Result>(rhs));
            return *this;
        }

        Result(Result&& r) noexcept : Base(std::forward<Base>(r)) {}

        Result() = delete;
        using Base::ResultRef;

        Option<T> ok() const {
            if (this->is_ok()) {
                return Option<T>::some(this->template reinterpret_as<T>());
            } else {
                return Option<T>::none();
            }
        }

        T ok_unchecked() const {
                return this->template reinterpret_as<T>();
        }

    private:
        void copy_impl(const Result& r) {
            this->type = r.type;
            if (this->is_ok()) {
                this->construct(r.get_underlying_object());
            } else {
                this->construct(r.err_ref_unchecked());
            }
        }
    };
}


#endif //MODERN_STL_RESULT_H
