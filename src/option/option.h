//
// Created by 朕与将军解战袍 on 2022/9/15.
//

#ifndef __MODERN_STL_OPTION_H__
#define __MODERN_STL_OPTION_H__

#include <global.h>
#include <concepts>
#include <intrinsics.h>
#include <basic_concepts.h>

namespace mstl {
    template<typename T>
    class Option {
        class NoneType{};

    public:
        template<typename... Args>
        MSTL_INLINE constexpr
        static Option<T> emplace(Args&&... args) {
            return { T{ std::forward<Args>(args)... } };
        }
        MSTL_INLINE constexpr static Option<T> some(T&& t)
        { return { std::forward<T>(t) }; }
        MSTL_INLINE constexpr static Option<T> some(const T& t)
        { return { t }; }
        MSTL_INLINE constexpr static Option<T> none()
        { return { }; }

        MSTL_INLINE constexpr bool is_some() const { return this->hold_value; }
        MSTL_INLINE constexpr bool is_none() const { return !this->hold_value; }

        MSTL_INLINE constexpr
        T unwrap() {
            if (this->hold_value) {
                return unwrap_unchecked();
            } else {
                MSTL_PANIC("unwrap at a none value.");
            }
        }

        MSTL_INLINE constexpr
        T unwrap_unchecked() {
            T res = std::move(value);
            reset();
            return res;
        }

        MSTL_INLINE constexpr
        T& as_ref() {
            if (this->hold_value) {
                return as_ref_uncheck();
            } else {
                MSTL_PANIC("get a ref of none value.");
            }
        }

        MSTL_INLINE constexpr
        const T& as_ref() const {
            if (this->hold_value) {
                return as_ref_uncheck();
            } else {
                MSTL_PANIC("get a ref of none value.");
            }
        }

        MSTL_INLINE constexpr
        T& as_ref_uncheck() {
            return this->value;
        }

        MSTL_INLINE constexpr
        const T& as_ref_uncheck() const {
            return this->value;
        }

        template<typename Lambda>
        requires std::invocable<Lambda, T> &&
               (!std::same_as<std::invoke_result_t<Lambda, T>, void>)
        MSTL_INLINE constexpr
        decltype(auto) map(Lambda m) {
            using AfterMap = std::invoke_result_t<Lambda, T>;
            if (hold_value) {
                return Option<AfterMap>::some(m(std::move(value)));
            } else {
                return Option<AfterMap>::none();
            }
        }

        constexpr Option(Option&& other) noexcept
        requires (!basic::Movable<T>) = delete;

        constexpr Option(Option&& other) noexcept
        requires basic::Movable<T> {
            hold_value = other.hold_value;

            if (hold_value) {
                construct_val(std::move(other.value));
                other.reset();
            } else {
                construct_non();
            }
        }

        constexpr Option& operator=(Option&& other) noexcept
        requires (!basic::Movable<T>) = delete;

        constexpr Option& operator=(Option&& other) noexcept
        requires basic::Movable<T> {
            if (this == &other) {
                return *this;
            }
            if (hold_value == other.hold_value) {
                if (hold_value) {
                    value = std::move(other.value);
                }
            } else {
                hold_value = other.hold_value;
                if (hold_value) {
                    destroy_non();
                    construct_val(std::move(other.value));

                    other.reset();      // set other to none
                } else {
                    destroy_val();
                    construct_non();
                }
            }
            return *this;
        }

        constexpr Option(const Option& other)
        requires (!basic::CopyAble<T>) = delete;

        constexpr Option(const Option& other)
        requires basic::CopyAble<T> {
            hold_value = other.hold_value;

            if (hold_value) {
                construct_val(other.value);
            } else {
                construct_non();
            }
        }

        constexpr Option& operator=(const Option& other)
        requires (!basic::CopyAble<T>) = delete;

        constexpr Option& operator=(const Option& other)
        requires basic::CopyAble<T> {
            if (this == &other) {
                return *this;
            }
            if (hold_value == other.hold_value) {
                if (hold_value) {
                    value = other.value;
                }
            } else {
                hold_value = other.hold_value;
                if (hold_value) {
                    destroy_non();
                    construct_val(other.value);
                } else {
                    destroy_val();
                    construct_non();
                }
            }
            return *this;
        }

        constexpr ~Option() {
            if (hold_value) {
                destroy_val();
            } else {
                destroy_non();
            }
        }

    private:
        constexpr Option(T&& t): value(std::forward<T&&>(t)), hold_value(true) {}
        constexpr Option(const T& t):   value(t), hold_value(true) {}
        constexpr Option(): non_value{}, hold_value(false) {}

        template<typename ...Args>
        constexpr void construct_val(Args&& ...args) {
            std::construct_at(&value, std::forward<Args>(args)...);
        }

        constexpr void destroy_val() {
            std::destroy_at(&value);
        }

        constexpr void construct_non() {
            std::construct_at(&non_value);
        }

        constexpr void destroy_non() {
            std::destroy_at(&non_value);
        }

        constexpr void reset() {
            destroy_val();
            construct_non();
            hold_value = false;
        }

        union {
            T value;
            NoneType non_value;
        };

        bool hold_value = false;
    };

    template<basic::RValRefType T>
    class Option<T> {
        static_assert(
            !basic::RValRefType<T>,
            "Should not store a rvalue reference in an Option.\n"
        );
    };

    template<basic::LValRefType T>
    class Option<T> {
    public:
        using StoreT = std::remove_reference_t<T>;
        using Item = StoreT&;
        using ConstItem = const StoreT&;
        MSTL_INLINE constexpr
        static Option some(T t) {
            auto *p = const_cast<StoreT*>(std::addressof(t));
            return { p };
        }
        MSTL_INLINE constexpr static Option none()
        { return { }; }

        MSTL_INLINE constexpr bool is_some() const
        { return ptr != nullptr; }
        MSTL_INLINE constexpr bool is_none() const
        { return !is_some(); }

        MSTL_INLINE constexpr
        Item unwrap() {
            if (is_some()) {
                Item ref = *this->ptr;
                this->ptr = nullptr;
                return ref;
            } else {
                MSTL_PANIC("unwrap is at a none value.");
            }
        }

        MSTL_INLINE constexpr
        Item unwrap_unchecked() {
            auto* tmp = ptr;
            this->ptr = nullptr;
            return *tmp;
        }

        MSTL_INLINE constexpr
        Item as_ref() {
            if (is_some()) {
                return *this->ptr;
            } else {
                MSTL_PANIC("try to get a None value ref.");
            }
        }

        MSTL_INLINE constexpr
        ConstItem as_ref() const {
            if (is_some()) {
                return *this->ptr;
            } else {
                MSTL_PANIC("try to get a None value ref.");
            }
        }

        MSTL_INLINE constexpr
        Item as_ref_uncheck() {
            return *this->ptr;
        }

        MSTL_INLINE constexpr
        ConstItem as_ref_uncheck() const {
            return *this->ptr;
        }

        template<typename Lambda>
        requires std::invocable<Lambda, T> &&
                 (!std::same_as<std::invoke_result_t<Lambda, T>, void>)
        MSTL_INLINE constexpr
        decltype(auto) map(Lambda m) {
            using AfterMap = std::invoke_result_t<Lambda, T>;
            if (is_some()) {
                return Option<AfterMap>::some(m(*ptr));
            } else {
                return Option<AfterMap>::none();
            }
        }

        constexpr Option(const Option& other) {
            // 不需要判断对方是否是 Some
            this->ptr = other.ptr;
        }

        constexpr Option& operator=(const Option& other) { // NOLINT(bugprone-unhandled-self-assignment)
            // 没必要为了处理 this == &other 而引入分支
            this->ptr = other.ptr;

            return *this;
        }

        constexpr Option(Option&& other) noexcept {
            // 不需要判断对方是否是 Some
            this->ptr = other.ptr;
            other.ptr = nullptr;
        }

        constexpr Option& operator=(Option&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            this->ptr = other.ptr;
            other.ptr = nullptr;
            return *this;
        }

    private:
        constexpr Option(StoreT* p): ptr(p) {}
        constexpr Option(): ptr(nullptr) {}

        StoreT *ptr = nullptr;
    };
}

#endif //__MODERN_STL_OPTION_H__
