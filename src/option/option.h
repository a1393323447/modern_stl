//
// Created by 朕与将军解战袍 on 2022/9/15.
//

#ifndef __MODERN_STL_OPTION_H__
#define __MODERN_STL_OPTION_H__

#include <cstdlib>
#include <utility>
#include <concepts>
#include <basic_concepts.h>

namespace mstl {
    template<typename T>
    class OptionBase {
    public:
        OptionBase(T t): value(t), hold_value(true) {}
        OptionBase(): hold_value(false) {}

        bool is_some() const { return hold_value; }
        bool is_none() const { return !hold_value; }

        T unwrap() {
            if (hold_value) {
                return std::move(value);
            } else {
                std::exit(-1);
            }
        }

        T unwrap_uncheck() {
            return std::move(value);
        }

    protected:
        bool hold_value = false;
        T value;
    };

    /**
     * Option 只能储存左值引用, 且将其转为指针储存
     */
    template<basic::LValRefType T>
    class OptionBase<T> {
    public:
        using StoreT = std::remove_cvref_t<T>;

        OptionBase(T ref_value): value(const_cast<StoreT*>(&ref_value)), hold_value(true) {
            static_assert(
                    std::is_same_v<decltype(const_cast<StoreT*>(&ref_value)), StoreT*>,
                    "Can not cast ref value to a pointer.\n"
            );
        }
        OptionBase(): hold_value(false) {}

        bool is_some() const { return hold_value; }
        bool is_none() const { return !hold_value; }

        T unwrap() {
            if (hold_value) {
                return *value;
            } else {
                std::exit(-1);
            }
        }

        T unwrap_uncheck() {
            return *value;
        }

    protected:
        bool    hold_value = false;
        StoreT *value = nullptr;
    };

    template<basic::RValRefType T>
    class OptionBase<T> {
        static_assert(
                !basic::RValRefType<T>,
                "Option can not store a rvalue reference.\n"
        );
    };

    template<typename T>
    class OptionCopyable: public OptionBase<T> {};

    template<basic::CopyAble T>
    class OptionCopyable<T>: public OptionBase<T> {
    public:
        OptionCopyable(T t): OptionBase<T>(t) {}
        OptionCopyable(): OptionBase<T>() {}

        OptionCopyable(const OptionCopyable<T>& other) {
            this->value = other.value;
            this->hold_value = other.hold_value;
        }
        OptionCopyable<T>& operator=(const OptionCopyable<T>& other) {
            this->value = other.value;
            this->hold_value = other.hold_value;
            return *this;
        }
    };

    template<typename T>
    class OptionMovable: public OptionCopyable<T> {
    public:
        static OptionMovable<T> some(T t) { return { t }; }
        static OptionMovable<T> none() { return { }; }
    };

    template<basic::Movable T>
    class OptionMovable<T>: public OptionCopyable<T> {
    public:
        static OptionMovable<T> some(T t) { return { t }; }
        static OptionMovable<T> none() { return { }; }

        OptionMovable(T t): OptionCopyable<T>(t) {}
        OptionMovable(): OptionCopyable<T>() {}

        OptionMovable(const OptionMovable<T>&& other) noexcept {
            this->value = std::move(other.value);
            this->hold_value = other.hold_value;
        }
        OptionMovable<T>& operator=(OptionMovable<T>&& other) noexcept {
            this->value = std::move(other.value);
            this->hold_value = other.hold_value;
            return *this;
        }
    };

    template<typename T>
    using Option = OptionMovable<T>;

    static_assert(std::is_same_v<decltype(Option<int&>{}.unwrap()), int&>);
}

#endif //__MODERN_STL_OPTION_H__
