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
    public:
        template<typename... Args>
        MSTL_INLINE constexpr
        static Option<T> emplace(Args&&... args) {
            return { T{ std::forward<Args&&>(args)... } };
        }
        MSTL_INLINE constexpr static Option<T> some(T&& t)
        { return { std::forward<T&&>(t) }; }
        MSTL_INLINE constexpr static Option<T> some(const T& t)
        { return { t }; }
        MSTL_INLINE constexpr static Option<T> none()
        { return { }; }

        MSTL_INLINE constexpr bool is_some() const { return this->hold_value; }
        MSTL_INLINE constexpr bool is_none() const { return !this->hold_value; }

        MSTL_INLINE constexpr
        T unwrap() {
            if (this->hold_value) {
                this->hold_value = false;
                return std::move(this->value);
            } else {
                MSTL_PANIC("unwrap at a none value.");
            }
        }

        MSTL_INLINE constexpr
        T unwrap_uncheck() {
            this->hold_value = false;
            return std::move(this->value);
        }

        MSTL_INLINE constexpr
        T& as_ref() {
            if (this->hold_value) {
                return this->value;
            } else {
                MSTL_PANIC("get a ref of none value.");
            }
        }

        MSTL_INLINE constexpr
        const T& as_ref() const {
            if (this->hold_value) {
                return this->value;
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
            if (other.is_some()) {
                this->value = std::move(other.value);
            }
            // 注意: 此处可能会导致 Some 中的 T 延迟析构 -> Some = std::move(None)
            this->hold_value = other.hold_value;
            other.hold_value = false;
        }

        constexpr Option& operator=(Option&& other) noexcept
        requires (!basic::Movable<T>) = delete;

        constexpr Option& operator=(Option&& other) noexcept
        requires basic::Movable<T> {
            this->hold_value = other.hold_value;
            if (other.is_none() || this == &other) { // 防止重复移动
                // 注意: 此处可能会导致 Some 中的 T 延迟析构 -> Some = std::move(None)
                return *this;
            }
            other.hold_value = false;
            this->value = std::move(other.value);
            return *this;
        }

        constexpr Option(const Option& other)
        requires (!basic::CopyAble<T>) = delete;

        constexpr Option(const Option& other)
        requires basic::CopyAble<T> {
            if (other.is_some()) {
                this->value = other.value;
            }
            // 注意: 此处可能会导致 Some 中的 T 延迟析构
            this->hold_value = other.hold_value;
        }

        constexpr Option& operator=(const Option& other)
        requires (!basic::CopyAble<T>) = delete;

        constexpr Option& operator=(const Option& other)
        requires basic::CopyAble<T> {
            this->hold_value = other.hold_value;
            if (other.is_none() || this == &other) { // 防止重复赋值
                // 注意: 此处可能会导致 Some 中的 T 延迟析构
                return *this;
            }
            this->value = other.value;
            return *this;
        }

    private:
        constexpr Option(T&& t): value(std::forward<T&&>(t)), hold_value(true) {}
        constexpr Option(const T& t):   value(t), hold_value(true) {}
        constexpr Option(): value(T{}), hold_value(false) {}

        T value;
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
        Item unwrap_uncheck() {
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
