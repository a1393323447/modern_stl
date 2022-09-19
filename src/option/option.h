//
// Created by 朕与将军解战袍 on 2022/9/15.
//

#ifndef __MODERN_STL_OPTION_H__
#define __MODERN_STL_OPTION_H__

#include <cstdlib>
#include <utility>
#include <global.h>
#include <concepts>
#include <intrinsics.h>
#include <basic_concepts.h>

namespace mstl {
    namespace _private {
        template<typename T>
        class OptionBase {
        public:
            OptionBase(T t): value(t), hold_value(true) {}
            OptionBase(): hold_value(false) {}

            MSTL_INLINE bool is_some() const { return hold_value; }
            MSTL_INLINE bool is_none() const { return !hold_value; }

            MSTL_INLINE T unwrap() {
                if (hold_value) {
                    return std::move(value);
                } else {
                    std::exit(-1);
                }
            }

            MSTL_INLINE T unwrap_uncheck() {
                return std::move(value);
            }

        protected:
            static constexpr bool
            has_hold_value() {
                return true;
            }
            bool hold_value = false;
            T value;
        };

        template<>
        class OptionBase<bool> {
        public:
            OptionBase(bool t): value(t | HOLD_MASK) {}
            OptionBase() {}

            MSTL_INLINE bool is_some() const { return value & HOLD_MASK; }
            MSTL_INLINE bool is_none() const { return !is_some(); }

            MSTL_INLINE bool unwrap() {
                if (is_some()) {
                    return value & VALUE_MASK;
                } else {
                    std::exit(-1);
                }
            }

            MSTL_INLINE bool unwrap_uncheck() {
                return value & VALUE_MASK;
            }

        protected:
            static constexpr u8 VALUE_MASK = 0b00000001;
            static constexpr u8 HOLD_MASK  = 0b10000000;
            static constexpr bool
            has_hold_value() {
                return false;
            }
            // 0       4      7
            u8 value = 0; // [ value | hold ]
        };

        /**
         * Option 只能储存左值引用, 且将其转为指针储存
         */
        template<basic::LValRefType T>
        class OptionBase<T> {
        public:
            using StoreT = std::remove_cvref_t<T>;

            OptionBase(T ref_value): value(const_cast<StoreT*>(&ref_value)) {
                static_assert(
                        std::is_same_v<decltype(const_cast<StoreT*>(&ref_value)), StoreT*>,
                        "Can not cast ref value to a pointer.\n"
                );
            }
            OptionBase(): value(nullptr) {}

            MSTL_INLINE bool is_some() const { return value != nullptr; }
            MSTL_INLINE bool is_none() const { return !is_some(); }

            MSTL_INLINE T unwrap() {
                if (value != nullptr) {
                    return *value;
                } else {
                    std::exit(-1);
                }
            }

            MSTL_INLINE T unwrap_uncheck() {
                return *value;
            }

        protected:
            static constexpr bool
            has_hold_value() {
                return false;
            }
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
        class OptionMovable: public OptionBase<T> {};

        template<basic::Movable T>
        class OptionMovable<T>: public OptionBase<T> {
        public:
            OptionMovable(T t): OptionBase<T>(t) {}
            OptionMovable(): OptionBase<T>() {}

            OptionMovable(const OptionMovable<T>&& other) noexcept {
                this->value = std::move(other.value);
                if constexpr (OptionBase<T>::has_hold_value()) {
                    this->hold_value = other.hold_value;
                }
            }
            OptionMovable<T>& operator=(OptionMovable<T>&& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                this->value = std::move(other.value);
                if constexpr (OptionBase<T>::has_hold_value()) {
                    this->hold_value = other.hold_value;
                }
                return *this;
            }
        };

        template<typename T>
        class OptionCopyable: public OptionMovable<T> {
        public:
            MSTL_INLINE static OptionCopyable<T> some(T t) { return { t }; }
            MSTL_INLINE static OptionCopyable<T> none() { return { }; }
        };

        template<basic::CopyAble T>
        class OptionCopyable<T>: public OptionMovable<T> {
        public:
            MSTL_INLINE static OptionCopyable<T> some(T t) { return { t }; }
            MSTL_INLINE static OptionCopyable<T> none() { return { }; }

            OptionCopyable(T t): OptionMovable<T>(t) {}
            OptionCopyable(): OptionMovable<T>() {}

            OptionCopyable(const OptionCopyable<T>& other) {
                this->value = other.value;
                if constexpr (OptionCopyable<T>::has_hold_value()) {
                    this->hold_value = other.hold_value;
                }
            }
            OptionCopyable<T>& operator=(const OptionCopyable<T>& other) {
                if (this == &other) {
                    return *this;
                }
                this->value = other.value;
                if constexpr (OptionCopyable<T>::has_hold_value()) {
                    this->hold_value = other.hold_value;
                }
                return *this;
            }
        };
    }

    template<typename T>
    using Option = _private::OptionCopyable<T>;

    static_assert(std::is_same_v<decltype(Option<int&>{}.unwrap()), int&>);
}

#endif //__MODERN_STL_OPTION_H__
