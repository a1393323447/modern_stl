//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_BASIC_STRING_H__
#define __MODERN_STL_BASIC_STRING_H__

// std
#include <cstdlib>
#include <algorithm>

#include <mstl/slice.h>
#include <mstl/memory/allocators/allocator.h>
#include <mstl/memory/allocators/allocator_concept.h>

#include <mstl/str/basic_char.h>
#include <mstl/str/encoding/concepts/decode.h>
#include <mstl/str/encoding/concepts/validate.h>
#include <mstl/str/encoding/utility/compile_time_validation.h>

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
                os.write(reinterpret_cast<const char *>(&str.storage.local[0]), str.len);
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
                alloc.deallocate(alloc.bytes, LAYOUT, storage.cap);
            }
        }

        template<usize N>
        BasicString(const encoding::ValidBytes<N, Encoding>& bytes, const Allocator& a = {}): alloc(a) {
            this->len = N;

            if constexpr (N <= LOCAL_STORAGE_SIZE) {
                copy(&storage.local[0], &bytes.arr[0], N);
            } else {
                // FIXME: 可能会溢出
                constexpr usize CAP = N + N / 2;
                storage.cap = CAP;
                alloc.bytes = (u8*)alloc.allocate(LAYOUT, CAP);
                copy(alloc.bytes, &bytes.arr[0], N);
            }
        }

        constexpr BasicString(const BasicString& other) {
            this->len = other.len;

            if (other.len > LOCAL_STORAGE_SIZE) {
                // FIXME: 可能会溢出
                storage.cap = other.storage.cap;
                alloc.bytes = (u8*)alloc.allocate(LAYOUT, storage.cap);
                copy(alloc.bytes, other.alloc.bytes, other.len);
            } else {
                this->storage = other.storage;
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

            // 1. 直接复制 local storage
            if (this->len < LOCAL_STORAGE_SIZE && other.len < LOCAL_STORAGE_SIZE) {
                this->len = other.len;
                this->alloc = other.alloc;
                this->storage = other.storage;
            }
            // 2. this 需要分配空间
            else if (this->len < LOCAL_STORAGE_SIZE && other.len > LOCAL_STORAGE_SIZE) {
                this->len = other.len;
                this->alloc = other.alloc;
                this->storage.cap = other.storage.cap;

                this->alloc.bytes = (u8*)this->alloc.allocate(LAYOUT, storage.cap);
                copy(this->alloc.bytes, other.alloc.bytes, other.len);
            }
            // 3. 需要解分配 this 的空间
            else if (this->len > LOCAL_STORAGE_SIZE && other.len < LOCAL_STORAGE_SIZE) {
                // deallocate before replace allocator
                this->alloc.deallocate(this->alloc.bytes, LAYOUT, this->storage.cap);
                this->alloc = other.alloc;

                this->len = other.len;
                this->storage = other.storage;
            }
            // 4. 需要判断 this.storage.cap < other.len ? 如果是才分配新空间
            else {
                this->len = other.len;

                if (this->storage.cap < other.len) {
                    // deallocate before replace allocator
                    this->alloc.deallocate(this->alloc.bytes, LAYOUT, this->storage.cap);
                    this->alloc = other.alloc;
                    this->storage.cap = len + len / 2;
                    this->alloc.bytes = (u8*)this->alloc.allocate(LAYOUT, storage.cap);
                }
                copy(this->alloc.bytes, other.alloc.bytes, len);
            }

            return *this;
        }

        constexpr BasicString(BasicString&& other) noexcept {
            this->len = other.len;
            this->alloc = std::move(other.alloc);
            this->alloc.bytes = other.alloc.bytes;
            this->storage = other.storage;

            other.len = 0;
            other.alloc.bytes = nullptr;
            other.storage.cap = 0;
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

            // 1. 直接复制 local storage
            if (this->len < LOCAL_STORAGE_SIZE && other.len < LOCAL_STORAGE_SIZE) {
                this->len = other.len;
                this->alloc = std::move(other.alloc);
                this->storage = other.storage;

                other.len = 0;
            }
            // 2. 需要将 other.bytes other.len other.cap 置为空
            else if (this->len < LOCAL_STORAGE_SIZE && other.len > LOCAL_STORAGE_SIZE) {
                this->len = other.len;
                this->alloc = std::move(other.alloc);
                this->alloc.bytes = other.alloc.bytes;
                this->storage.cap = other.storage.cap;

                other.len = 0;
                other.alloc.bytes = nullptr;
                other.storage.cap = 0;
            }
            // 3. 需要将 this.bytes 解分配 复制 other.storage
            else if (this->len > LOCAL_STORAGE_SIZE && other.len < LOCAL_STORAGE_SIZE) {
                // deallocate before replace allocator
                this->alloc.deallocate(this->alloc.bytes, LAYOUT, this->storage.cap);

                this->len = other.len;
                this->alloc = std::move(other.alloc);
                this->storage = other.storage;

                other.len = 0;
            }
            // 4. 需要将 this.bytes 解分配, 并获取 other.bytes
            else {
                // deallocate before replace allocator
                this->alloc.deallocate(this->alloc.bytes, LAYOUT, this->storage.cap);
                this->alloc = std::move(other.alloc);
                this->alloc.bytes = other.alloc.bytes;
                this->len = other.len;
                this->storage.cap = other.storage.cap;

                other.len = 0;
                other.alloc.bytes = nullptr;
                other.storage.cap = 0;
            }

            return *this;
        }

        constexpr
        void push_back(const Char& ch) {
            usize new_len = len + ch.get_len();

            AllocateInfo info = try_allocate_new_space(new_len);

            if (info.alloc_new_space) {
                // copy original data to new space
                copy(info.des, info.src, len);
                if (info.need_deallocate) {
                    // deallocate old space
                    alloc.deallocate(info.src, LAYOUT, storage.cap);
                }
                // set new cap
                storage.cap = info.new_cap;
            }

            // copy new char data to new space
            copy(&info.des[len], &ch.bytes[0], ch.get_len());

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

        constexpr void insert(const Char& ch, usize idx)
        requires concepts::CheckCharBoundary<Encoding, typename Slice<u8>::ConstRefIter> {
            if (!is_char_boundary(idx)) {
                MSTL_PANIC("idx is not in a char boundary");
            }

            usize new_len = len + ch.get_len();
            AllocateInfo info = try_allocate_new_space(new_len);

            if (info.alloc_new_space) {
                // because we allocate new space
                // we have two different memory region
                // and this two region would not overlap

                // copy original data to new space
                // copy src[0..idx] to des[0..idx]
                copy(info.des, info.src, idx);
                // copy des char data to des space
                // copy char.bytes to des[idx..idx + char.len()]
                copy(&info.des[idx], &ch.bytes[0], ch.get_len());
                // copy remain original data to des space
                // copy src[idx..len] to des[idx + char.len()..] copy size = len - idx
                copy(&info.des[idx + ch.get_len()], info.src + idx, len - idx);
                if (info.need_deallocate) {
                    // deallocate src space
                    alloc.deallocate(info.src, LAYOUT, storage.cap);
                }
                // set new cap
                storage.cap = info.new_cap;
            } else {
                // copy src[idx..len] to des[idx + ch.get_len()..]
                // note that this two memory regions is overlapped
                copy(&info.des[idx + ch.get_len()], &info.src[idx], len - idx, true);
                // copy char.bytes to des[idx..]
                copy(&info.des[idx], &ch.bytes[0], ch.get_len());
            }
            // set new_len
            len = new_len;
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

        constexpr bool is_char_boundary(usize idx) {
            if (idx == 0) {
                return true;
            }

            if (idx >= this->len) {
                return idx == this->len;
            } else {
                const auto start = idx > Encoding::MAX_LEN ? idx - Encoding::MAX_LEN : 0;
                const auto end = idx + 1;

                const auto slice_len = end - start;
                u8* start_ptr = nullptr;
                if (len > LOCAL_STORAGE_SIZE) {
                    start_ptr = &this->alloc.bytes[start];
                } else {
                    start_ptr = &this->storage.local[start];
                }
                auto slice = Slice<u8>::from_raw(start_ptr, slice_len);

                return Encoding::is_char_boundary(slice.iter());
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
            this->storage.cap = new_cap;
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
                return Slice<u8>::from_raw(&storage.local[0], len);
            }
        }

        template<typename T>
        constexpr
        T* copy(T* des, const T* src, usize size, bool overlap = false) {
            if (std::is_constant_evaluated()) {
                std::copy(src, src + size, des);
            } else if (overlap) {
                memmove(des, src, size);
            } else {
                memcpy(des, src, size);
            }
            return des;
        }

        struct AllocateInfo {
            /// points to the start of memory region of original data
            u8 *src = nullptr;
            /// points to the start of memory region of new data
            /// des = src when alloc_new_space is false
            u8 *des = nullptr;
            /// new cap
            /// should only be used when allocate_new_space is true
            usize new_cap = 0;
            /// true if new space was allocated
            bool alloc_new_space = false;
            /// true if src need to deallocate
            bool need_deallocate = false;
        };

        /// decide whether to allocate memory based on new_Len
        /// and return the info
        AllocateInfo try_allocate_new_space(usize new_len) {
            AllocateInfo info{};
            if (len > LOCAL_STORAGE_SIZE) {
                // original data is on heap
                info.src = alloc.bytes;
                if (new_len > storage.cap) {
                    // need to allocate new space
                    info.new_cap = new_len + new_len / 2;
                    alloc.bytes = (u8*)alloc.allocate(LAYOUT, info.new_cap);
                    info.des = alloc.bytes;
                    info.alloc_new_space = true;
                    info.need_deallocate = true;
                } else {
                    // don't need to allocate new space
                    // so des = src
                    info.des = info.src;
                }
            } else {
                // original data is on local storage
                info.src = &storage.local[0];
                if (new_len > LOCAL_STORAGE_SIZE) {
                    // need to alloc heap space
                    info.new_cap = new_len + new_len / 2;
                    alloc.bytes = (u8*)alloc.allocate(LAYOUT, info.new_cap);
                    info.des = alloc.bytes;
                    info.alloc_new_space = true;
                    // don't need to deallocate old space
                    // because old space is on stack
                } else {
                    // don't need to allocate new space
                    // so des = src
                    info.des = info.src;
                }
            }

            return info;
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
        } storage;
        Allocate alloc;
    };
}

#endif //__MODERN_STL_BASIC_STRING_H__
