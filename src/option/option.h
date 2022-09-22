//
// Created by 朕与将军解战袍 on 2022/9/15.
//

#ifndef __MODERN_STL_OPTION_H__
#define __MODERN_STL_OPTION_H__

#include <forward_list>
#include <utility>
#include <global.h>
#include <concepts>
#include <intrinsics.h>
#include <basic_concepts.h>

namespace mstl {
    // 现在的 Option 中的 T 是延迟析构的 !
    template<typename T>
    class Option {

    };


    template<typename T>
    requires (!basic::RefType<T>)
    class Option<T> {
    public:
        template<typename... Args>
        MSTL_INLINE static Option<T> emplace(Args&&... args) {
            return { T{ std::forward<Args&&>(args)... } };
        }
        MSTL_INLINE static Option<T> some(T&& t) { return { std::forward<T&&>(t) }; }
        MSTL_INLINE static Option<T> some(const T& t)   { return { t }; }
        MSTL_INLINE static Option<T> none()      { return { }; }

        MSTL_INLINE bool is_some() const { return this->hold_value; }
        MSTL_INLINE bool is_none() const { return !this->hold_value; }

        MSTL_INLINE T unwrap() {
            if (this->hold_value) {
                return std::move(this->value);
            } else {
                MSTL_PANIC("unwrap at a none value.");
            }
        }

        MSTL_INLINE T unwrap_uncheck() {
            return std::move(this->value);
        }

        MSTL_INLINE T& as_ref() {
            if (this->hold_value) {
                return std::move(this->value);
            } else {
                MSTL_PANIC("get a ref of none value.");
            }
        }

        MSTL_INLINE T& as_ref_uncheck() {
            return this->value;
        }

        template<typename U>
        requires std::same_as<U, T> &&
                 basic::CopyAble<U>
        Option(const Option<U>& other) {
            if (other.is_some()) {
                this->value = other.value;
            }
            // 注意: 此处可能会导致 Some 中的 T 延迟析构
            this->hold_value = other.hold_value;
        }

        template<typename U>
        requires std::same_as<U, T> &&
                 basic::CopyAble<U>
        Option<T>& operator=(const Option<U>& other) {
            this->hold_value = other.hold_value;
            if (other.is_none() || this == &other) { // 防止重复赋值
                // 注意: 此处可能会导致 Some 中的 T 延迟析构
                return *this;
            }
            this->value = other.value;
            return *this;
        }

        template<typename U>
        requires std::same_as<U, T> &&
                 basic::Movable<U>
        Option(Option<U>&& other) noexcept {
            if (other.is_some()) {
                this->value = std::move(other.value);
            }
            // 注意: 此处可能会导致 Some 中的 T 延迟析构 -> Some = std::move(None)
            this->hold_value = other.hold_value;
            other.hold_value = false;
        }

        template<typename U>
        requires std::same_as<U, T> &&
                 basic::Movable<U>
        Option<T>& operator=(Option<U>&& other) noexcept {
            this->hold_value = other.hold_value;
            if (other.is_none() || this == &other) { // 防止重复移动
                // 注意: 此处可能会导致 Some 中的 T 延迟析构 -> Some = std::move(None)
                return *this;
            }
            other.hold_value = false;
            this->value = std::move(other.value);
        }

    private:
        Option(T&& t): value(std::forward<T&&>(t)), hold_value(true) {}
        Option(const T& t):   value(t), hold_value(true) {}
        Option(): hold_value(false) {}

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
        using StoreT = std::remove_cvref_t<T>;
        MSTL_INLINE static Option<T> some(T t) {
            auto *p = const_cast<StoreT*>(std::addressof(t));
            return { p };
        }
        MSTL_INLINE static Option<T> none() { return { }; }

        MSTL_INLINE bool is_some() const { return this->hold_value; }
        MSTL_INLINE bool is_none() const { return !this->hold_value; }

        MSTL_INLINE T unwrap() {
            if (this->hold_value) {
                return *this->ptr;
            } else {
                MSTL_PANIC("unwrap at a none value.");
            }
        }

        MSTL_INLINE T unwrap_uncheck() {
            return *this->ptr;
        }

        MSTL_INLINE T as_ref() {
            return this->unwrap();
        }

        MSTL_INLINE T as_ref_uncheck() {
            return this->unwrap_uncheck();
        }

        Option(const Option& other) {
            // 不需要判断对方是否是 Some
            this->ptr = other.ptr;
            this->hold_value = other.hold_value;
        }

        Option& operator=(const Option& other) {
            // 没必要为了处理 this == &other 而引入分支
            this->ptr = other.ptr;
            this->hold_value = other.hold_value;

            return *this;
        }

        Option(Option&& other) noexcept {
            // 不需要判断对方是否是 Some
            this->ptr = other.ptr;
            this->hold_value = other.hold_value;
            other.ptr = nullptr;
            other.hold_value = false;
        }

        Option& operator=(Option&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            this->ptr = other.ptr;
            this->hold_value = other.hold_value;
            other.ptr = nullptr;
            other.hold_value = false;
            return *this;
        }

    private:
        Option(StoreT* p): ptr(p), hold_value(true) {}
        Option(): ptr(nullptr), hold_value(false) {}

        StoreT *ptr = nullptr;
        bool hold_value = false;
    };
}

#endif //__MODERN_STL_OPTION_H__
