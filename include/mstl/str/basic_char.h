//
// Created by 朕与将军解战袍 on 2022/9/28.
//

#ifndef __MODERN_STL_BASIC_CHAR_H__
#define __MODERN_STL_BASIC_CHAR_H__

#include <mstl/str/encoding/concepts/validate.h>
#include <mstl/str/encoding/utility/compile_time_validation.h>
#include <mstl/str/encoding/concepts/encoding_info.h>
#include <ostream>

namespace mstl::str {
    template<typename T>
    struct Bytes {
        mstl::u8 bytes[sizeof(T)]{};
    };

    template<typename From>
    constexpr Bytes<std::remove_cvref_t<From>> bit_cast(From from) {
        return std::bit_cast<Bytes<std::remove_cvref_t<From>>>(from);
    }

    template<typename Char, usize Size>
    struct CharType {

    };

    template<typename Char, usize Size>
    requires (Size > 2 && Size <= 4)
    struct CharType<Char, Size> {
        using type = char32_t;
        static constexpr bool value = std::same_as<Char, u32> ||
                                      std::same_as<Char, char32_t>;
    };

    template<typename Char, usize Size>
    requires (Size > 1 && Size <= 2)
    struct CharType<Char, Size> {
        using type = char16_t;
        static constexpr bool value = std::same_as<Char, u16> ||
                                      std::same_as<Char, char16_t>;
    };

    template<typename Char, usize Size>
    requires (Size <= 1)
    struct CharType<Char, Size> {
        using type = char8_t;
        static constexpr bool value = std::same_as<Char, u8>   ||
                                      std::same_as<Char, char> ||
                                      std::same_as<Char, char8_t>;
    };

    template<concepts::EncodingInfo Encoding>
    struct BasicChar {
        static_assert(
            !concepts::EncodingInfo<Encoding>,
            "BasicChar Unexpected Error: This is a bug\n"
        );
    };

    template<concepts::EncodingInfo Encoding>
    requires std::same_as<typename Encoding::CodeType, encoding::BlockCode>
    struct BasicChar<Encoding> {
        BasicChar() = default;

        template<usize N>
        requires (N > 0)
        constexpr BasicChar(const encoding::ValidBytes<N, Encoding>& valid) {
            static_assert(N <= Encoding::MAX_LEN, "too many letter in a char");
            for (usize i = 0; i < N; i++) {
                bytes[i] = valid.arr[i];
            }
        }

        MSTL_INLINE constexpr
        u8 get_len() const {
            return Encoding::MAX_LEN;
        }

        u8 bytes[Encoding::MAX_LEN]{0};
    };

    template<concepts::EncodingInfo Encoding>
    requires std::same_as<typename Encoding::CodeType, encoding::VariableLengthCode>
    struct BasicChar<Encoding> {
        BasicChar() = default;

        template<usize N>
        requires (N > 0)
        constexpr BasicChar(const encoding::ValidBytes<N, Encoding>& valid) {
            static_assert(N <= Encoding::MAX_LEN, "too many letter in a char");
            len = N;
            for (usize i = 0; i < N; i++) {
                bytes[i] = valid.arr[i];
            }
        }

        MSTL_INLINE constexpr
        u8 get_len() const {
            return len;
        }

        u8 len{0};
        u8 bytes[Encoding::MAX_LEN]{0};
    };
}

template <mstl::str::concepts::EncodingInfo Encoding>
std::ostream& operator<<(std::ostream& os, const mstl::str::BasicChar<Encoding>& ch) {
    os.write(reinterpret_cast<const char *>(&ch.bytes[0]), ch.get_len());
    return os;
}

#endif //__MODERN_STL_BASIC_CHAR_H__
