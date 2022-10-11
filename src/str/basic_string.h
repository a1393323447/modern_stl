//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_BASIC_STRING_H__
#define __MODERN_STL_BASIC_STRING_H__

// std
#include <cstdlib>
#include <algorithm>

#include <slice.h>
#include <memory/allocators/allocator.h>
#include <memory/allocators/allocator_concept.h>

#include <str/basic_char.h>
#include <str/encoding/concepts/decode.h>
#include <str/encoding/concepts/validate.h>
#include <str/encoding/utility/compile_time_validation.h>

namespace mstl::str {
    template <typename Encoding>
    class Chars {
    public:
        using Item = BasicChar<Encoding>;
        using InnerIter = SliceRefIter<u8, const u8&>;

        Chars(InnerIter iter): iter(iter) { }

        MSTL_INLINE constexpr
        Option<Item> next() noexcept
        requires concepts::DecodeNext<Encoding, InnerIter> {
            return Encoding::next(iter);
        }

    private:
        InnerIter iter;
    };

    template <typename Encoding, memory::concepts::Allocator Allocator = memory::allocator::Allocator>
    class BasicString {
        friend std::ostream& operator<<(std::ostream& os, const mstl::str::BasicString<Encoding>& str) {
            if (str.len < LOCAL_STORAGE_SIZE) {
                os.write(reinterpret_cast<const char *>(&str.storge.local[0]), str.len);
            } else {
                os.write(reinterpret_cast<const char *>(str.alloc.bytes), str.len);
            }
            return os;
        }
    public:
        using Char = BasicChar<Encoding>;
        using Item = Char;
        static constexpr memory::Layout LAYOUT = memory::Layout::from_type<u8>();

        constexpr BasicString() = default;
        constexpr BasicString(Allocator&& a): alloc(std::forward<Allocator&&>(a)) {}
        constexpr ~BasicString() {
            if (this->len > LOCAL_STORAGE_SIZE) {
                alloc.deallocate(alloc.bytes, LAYOUT, storge.cap);
            }
        }

        template<usize N>
        BasicString(const encoding::ValidBytes<N>& bytes, const Allocator& a = {}): alloc(a) {
            this->len = N;

            if constexpr (N <= LOCAL_STORAGE_SIZE) {
                copy(&storge.local[0], &bytes.arr[0], N);
            } else {
                // FIXME: 可能会溢出
                constexpr usize CAP = N + N / 2;
                storge.cap = CAP;
                alloc.bytes = (u8*)alloc.allocate(LAYOUT, CAP);
                copy(alloc.bytes, &bytes.arr[0], N);
            }
        }

        constexpr BasicString(const BasicString& other) {
            this->len = other.len;

            if (other.len > LOCAL_STORAGE_SIZE) {
                // FIXME: 可能会溢出
                storge.cap = other.storge.cap;
                alloc.bytes = (u8*)alloc.allocate(LAYOUT, storge.cap);
                copy(alloc.bytes, other.alloc.bytes, other.len);
            } else {
                this->storge = other.storge;
            }
        }

        MSTL_INLINE constexpr
        BasicString& operator=(const BasicString& other) {
            if (this == &other) {
                return *this;
            }

            //         分类讨论
            //    this    |   other
            // 1. local   |   local
            // 2. local   |   heap
            // 3. heap    |   local
            // 4. heap    |   heap

            // 1. 直接复制 local storge
            if (this->len < LOCAL_STORAGE_SIZE && other.len < LOCAL_STORAGE_SIZE) {
                this->len = other.len;
                this->alloc = other.alloc;
                this->storge = other.storge;
            }
            // 2. this 需要分配空间
            else if (this->len < LOCAL_STORAGE_SIZE && other.len > LOCAL_STORAGE_SIZE) {
                this->len = other.len;
                this->alloc = other.alloc;
                this->storge.cap = other.storge.cap;

                this->alloc.bytes = (u8*)this->alloc.allocate(LAYOUT, storge.cap);
                copy(this->alloc.bytes, other.alloc.bytes, other.len);
            }
            // 3. 需要解分配 this 的空间
            else if (this->len > LOCAL_STORAGE_SIZE && other.len < LOCAL_STORAGE_SIZE) {
                // deallocate before replace allocator
                this->alloc.deallocate(this->alloc.bytes, LAYOUT, this->storge.cap);
                this->alloc = other.alloc;

                this->len = other.len;
                this->storge = other.storge;
            }
            // 4. 需要判断 this.storge.cap < other.len ? 如果是才分配新空间
            else {
                this->len = other.len;

                if (this->storge.cap < other.len) {
                    // deallocate before replace allocator
                    this->alloc.deallocate(this->alloc.bytes, LAYOUT, this->storge.cap);
                    this->alloc = other.alloc;
                    this->storge.cap = len + len / 2;
                    this->alloc.bytes = (u8*)this->alloc.allocate(LAYOUT, storge.cap);
                }
                copy(this->alloc.bytes, other.alloc.bytes, len);
            }

            return *this;
        }

        constexpr BasicString(BasicString&& other) noexcept {
            this->len = other.len;
            this->alloc = std::move(other.alloc);
            this->alloc.bytes = other.alloc.bytes;
            this->storge = other.storge;

            other.len = 0;
            other.alloc.bytes = nullptr;
            other.storge.cap = 0;
        }

        MSTL_INLINE constexpr
        BasicString& operator=(BasicString&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            //         分类讨论
            //    this    |   other
            // 1. local   |   local
            // 2. local   |   heap
            // 3. heap    |   local
            // 4. heap    |   heap

            // 1. 直接复制 local storge
            if (this->len < LOCAL_STORAGE_SIZE && other.len < LOCAL_STORAGE_SIZE) {
                this->len = other.len;
                this->alloc = std::move(other.alloc);
                this->storge = other.storge;

                other.len = 0;
            }
            // 2. 需要将 other.bytes other.len other.cap 置为空
            else if (this->len < LOCAL_STORAGE_SIZE && other.len > LOCAL_STORAGE_SIZE) {
                this->len = other.len;
                this->alloc = std::move(other.alloc);
                this->alloc.bytes = other.alloc.bytes;
                this->storge.cap = other.storge.cap;

                other.len = 0;
                other.alloc.bytes = nullptr;
                other.storge.cap = 0;
            }
            // 3. 需要将 this.bytes 解分配 复制 other.storge
            else if (this->len > LOCAL_STORAGE_SIZE && other.len < LOCAL_STORAGE_SIZE) {
                // deallocate before replace allocator
                this->alloc.deallocate(this->alloc.bytes, LAYOUT, this->storge.cap);

                this->len = other.len;
                this->alloc = std::move(other.alloc);
                this->storge = other.storge;

                other.len = 0;
            }
            // 4. 需要将 this.bytes 解分配, 并获取 other.bytes
            else {
                // deallocate before replace allocator
                this->alloc.deallocate(this->alloc.bytes, LAYOUT, this->storge.cap);
                this->alloc = std::move(other.alloc);
                this->alloc.bytes = other.alloc.bytes;
                this->len = other.len;
                this->storge.cap = other.storge.cap;

                other.len = 0;
                other.alloc.bytes = nullptr;
                other.storge.cap = 0;
            }

            return *this;
        }

        constexpr
        void push_back(const Char ch) {
            usize new_len = len + ch.get_len();

            if (new_len > LOCAL_STORAGE_SIZE) {
                if (new_len > storge.cap) {
                    auto* old = alloc.bytes;
                    // allocate new space
                    auto new_cap = new_len + new_len / 2;
                    alloc.bytes = (u8*)alloc.allocate(LAYOUT, new_cap);
                    // copy original data to new space
                    copy(alloc.bytes, old, len);
                    // deallocate old space
                    alloc.deallocate(old, LAYOUT, storge.cap);
                    // set new cap
                    storge.cap = new_cap;
                }
                // copy new char data to new space
                copy(&alloc.bytes[len], &ch.bytes[0], ch.get_len());
            } else {
                // copy new char data to new space
                copy(&storge.local[len], &ch.bytes[0], ch.get_len());
            }

            len = new_len;
        }

        MSTL_INLINE constexpr
        Option<Char> pop_back()
        requires concepts::DecodeLast<Encoding, typename Slice<u8>::ConstRefIter> {
            auto slice = make_slice();
            auto bytes_iter = slice.iter();
            Option<Char> ch = Encoding::last(bytes_iter);
            if (ch.is_some()) {
                this->len -= ch.as_ref_uncheck().get_len();
            }
            return ch;
        }

        constexpr void shrink_to_fit() {
            // TODO
        }

        constexpr void reserve(usize new_cap) {
            using std::strong_ordering;
            switch (new_cap <=> this->cap) {
                case strong_ordering::greater:
                    extent_space(new_cap);
                    break;
                case strong_ordering::less:
                    shrink_to_fit();
                    break;
                case strong_ordering::equal:
                case strong_ordering::equivalent:
                    break;
            }
        }

        constexpr usize size() const {
            return this->len;
        }

        constexpr Chars<Encoding> chars() {
            auto slice = make_slice();
            return Chars<Encoding> { slice.iter() };
        }

    private:
        /// 扩大空间, 要求 new_cap > this->cap
        constexpr void extent_space(usize new_cap) {
            this->storge.cap = new_cap;
            u8 *new_space = (u8*)alloc.allocate(LAYOUT, new_cap);
            // copy to new space
            copy(new_space, alloc.bytes, len);
            // deallocate
            alloc.deallocate(alloc.bytes, LAYOUT, len);

            this->bytes = new_space;
        }

        MSTL_INLINE constexpr
        Slice<u8> make_slice() {
            if (this->len > LOCAL_STORAGE_SIZE) {
                return Slice<u8>::from_raw(alloc.bytes, len);
            } else {
                return Slice<u8>::from_raw(&storge.local[0], len);
            }
        }

        template<typename T>
        constexpr
        T* copy(T* des, const T* src, usize size) {
            if (std::is_constant_evaluated()) {
                std::copy(src, src + size, des);
            } else {
                memcpy(des, src, size);
            }
            return des;
        }

        /// 所有对 Allocate 的 move copy 都只会对其继承的 Allocator 进行 move copy
        /// 不会对 bytes 进行 move copy
        struct Allocate: public Allocator {
            constexpr ~Allocate() { this->~Allocator(); }

            constexpr Allocate(): Allocator() {}
            constexpr Allocate(Allocator&& a): Allocator(a) {}
            constexpr Allocate(const Allocator& a): Allocator(a) {}

            constexpr Allocate(const Allocate& other): Allocator(other) {}
            constexpr Allocate(Allocate&& other) noexcept :
            Allocator(std::forward<Allocator&&>(other))  {}
            constexpr Allocate& operator=(const Allocate& other) {
                if (this == &other) {
                    return *this;
                }
                Allocator::operator=(other);
            }
            constexpr Allocate& operator=(Allocate&& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                Allocator::operator=(std::forward<Allocate&&>(other));
            }

            u8 *bytes = nullptr;
        };

        static constexpr usize LOCAL_STORAGE_SIZE = 16;

        usize    len = 0;
        union {
            u8 local[LOCAL_STORAGE_SIZE];
            usize cap;
        } storge;
        Allocate alloc;
    };
}

#endif //__MODERN_STL_BASIC_STRING_H__
