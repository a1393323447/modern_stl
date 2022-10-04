//
// Created by 朕与将军解战袍 on 2022/10/3.
//

#ifndef __MODERN_STL_UTF8_H__
#define __MODERN_STL_UTF8_H__

#include <bit>

// TODO: UTF-8 string literal. ex. "\U+20AC"

namespace mstl::str::encoding {
    struct UTF8 {
        static constexpr usize MAX_LEN = 4;
        using CodeType = VariableLengthCode;
        using Char = BasicChar<UTF8>;

        template<iter::Iterator Iter>
        requires std::same_as<typename Iter::Item, const u8 &>
        MSTL_INLINE constexpr static
        Option<Char> next(Iter &iter) {
            u8 x;
            auto next = iter.next();
            if (next.is_some()) {
                x = next.unwrap_uncheck();
            } else {
                return Option<Char>::none();
            }
            if (x < 0x7F) {
                const auto ascii_case = ValidBytes<1>{{ x }};
                return Option<Char>::some(ascii_case);
            }

            // SAFETY: `bytes` produces a UTF-8-like string,
            // so the iterator must produce a value here.
            const u8 y = iter.next().unwrap_uncheck();
            if (x < 0XE0) {
                const auto ascii_case = ValidBytes<2>{{ x, y }};
                return Option<Char>::some(ascii_case);
            }

            // [[x y z] w] case
            // SAFETY: `bytes` produces an UTF-8-like string,
            // so the iterator must produce a value here.
            const u8 z = iter.next().unwrap_uncheck();
            if (x < 0XF0) {
                const auto ascii_case = ValidBytes<3>{{ x, y, z }};
                return Option<Char>::some(ascii_case);
            }

            // [x y z w] case
            // SAFETY: `bytes` produces an UTF-8-like string,
            // so the iterator must produce a value here.
            const u8 w = iter.next().unwrap_uncheck();
            const auto ascii_case = ValidBytes<4>{{ x, y, z, w }};
            return Option<Char>::some(ascii_case);
        }

        template<iter::DoubleEndedIterator Iter>
        requires std::same_as<typename Iter::Item, const u8 &>
        MSTL_INLINE constexpr static
        Option<Char> last(Iter &iter) {
            auto byte = iter.prev();
            if (byte.is_none) {
                return Option<Char>::none();
            }
            const u8 w = byte.unwrap_uncheck();
            if (w < 0x7F) {
                const auto ch = ValidBytes<1>{{ w }};
                return Option<Char>::some(ch);
            }
            // SAFETY: `bytes` produces an UTF-8-like string,
            // so the iterator must produce a value here.
            const u8 z = iter.next().unwrap_uncheck();
            if (!utf8_is_cont_byte(z)) {
                const auto ch = ValidBytes<2>{{ z, w }};
                return Option<Char>::some(ch);
            }
            // SAFETY: `bytes` produces an UTF-8-like string,
            // so the iterator must produce a value here.
            const u8 y = iter.next().unwrap_uncheck();
            if (!utf8_is_cont_byte(y)) {
                const auto ch = ValidBytes<3>{{ y, z, w }};
                return Option<Char>::some(ch);
            }
            const u8 x = iter.next().unwrap_uncheck();
            const auto ch = ValidBytes<4>{{ x, y, z, w }};
            return Option<Char>::some(ch);
        }

        template<iter::ContinuousIterator Iter>
        requires std::same_as<typename Iter::Item, const u8 &>
        MSTL_INLINE constexpr static
        Option<DecodeError>
        validate(Iter &&iter) {
            // TODO: try to skip forward quickly in ascii case

            #define get_next(next, val)                     \
            next = iter.next();                             \
            if (next.is_some()) {                           \
                offset++;                                   \
                val = next.unwrap_uncheck();                \
            } else {                                        \
                return Option<DecodeError>::some({offset}); \
            }
            #define err()                                   \
            return Option<DecodeError>::some({offset})

            usize offset = 0;
            Option<const u8&> next = iter.next();
            while (next.is_some()) {
                const u8 first_byte = next.unwrap_uncheck();
                const usize width = utf8_char_width(first_byte);
                // 1-byte encoding is for codepoints  \u{0000} to  \u{007f}
                //        first  00                7F
                // 2-byte encoding is for codepoints  \u{0080} to  \u{07FF}
                //        first  C2 80        last DF BF
                // 3-byte encoding is for codepoints  \u{0800} to  \u{FFFF}
                //        first  E0 A0 80     last EF BF BF
                //   excluding surrogates codepoints  \u{D800} to  \u{DFFF}
                //               ED A0 80 to       ED BF BF
                // 4-byte encoding is for codepoints \u{1000}0 to \u{10FF}FF
                //        first  F0 90 80 80  last F4 8F BF BF
                //
                // Use the UTF-8 syntax from the RFC
                //
                // https://tools.ietf.org/html/rfc3629
                // UTF8-1      = %x00-7F
                // UTF8-2      = %xC2-DF UTF8-tail
                // UTF8-3      = %xE0 %xA0-BF UTF8-tail / %xE1-EC 2( UTF8-tail ) /
                //               %xED %x80-9F UTF8-tail / %xEE-EF 2( UTF8-tail )
                // UTF8-4      = %xF0 %x90-BF 2( UTF8-tail ) / %xF1-F3 3( UTF8-tail ) /
                //               %xF4 %x80-8F 2( UTF8-tail )
                switch (width) {
                case 1:
                    break;
                case 2: {
                    u8 next_byte;
                    get_next(next, next_byte)
                    if (!utf8_is_cont_byte(next_byte)) {
                        err();
                    }
                    break;
                }
                case 3: {
                    u8 next_byte;
                    get_next(next, next_byte)
                    const bool check =
                        (0xE0 == first_byte                       && 0xA0 <= next_byte && next_byte <= 0xBF) ||
                        (0XE1 <= first_byte && first_byte <= 0xEC && 0x80 <= next_byte && next_byte <= 0xBF) ||
                        (0xED == first_byte                       && 0x80 <= next_byte && next_byte <= 0x9F) ||
                        (0XEE <= first_byte && first_byte <= 0xEF && 0x80 <= next_byte && next_byte <= 0xBF);
                    if (!check) {
                        err();
                    }
                    get_next(next, next_byte)
                    if (!utf8_is_cont_byte(next_byte)) {
                        err();
                    }
                    break;
                }
                case 4: {
                    u8 next_byte;
                    get_next(next, next_byte)
                    const bool check =
                        (0xF0 == first_byte                       && 0x90 <= next_byte && next_byte <= 0xBF) ||
                        (0XF1 <= first_byte && first_byte <= 0xF3 && 0x80 <= next_byte && next_byte <= 0xBF) ||
                        (0xF4 == first_byte                       && 0x80 <= next_byte && next_byte <= 0x8F);
                    if (!check) {
                        err();
                    }
                    get_next(next, next_byte)
                    if (!utf8_is_cont_byte(next_byte)) {
                        err();
                    }

                    get_next(next, next_byte)
                    if (!utf8_is_cont_byte(next_byte)) {
                        err();
                    }
                    break;
                }
                default:
                    err();
                }
                offset++;
                next = iter.next();
            }

            return Option<DecodeError>::none();
        }

        /// Checks whether the byte is a UTF-8 continuation byte (i.e., starts with the
        /// bits `10`).
        MSTL_INLINE static constexpr
        bool utf8_is_cont_byte(const u8 byte) {
            return std::bit_cast<const i8>(byte) < -64;
        }

        static constexpr u8 CONT_MASK = 0b0011'1111;
        // https://tools.ietf.org/html/rfc3629
        static constexpr u8 UTF8_CHAR_WIDTH [256] = {
            // 1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 1
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 2
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 3
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 4
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 5
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 6
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 7
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B
            0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // C
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // D
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // E
            4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // F
        };

        MSTL_INLINE
        static constexpr
        usize utf8_char_width(u8 byte) {
            return UTF8_CHAR_WIDTH[byte];
        }
    };

    define_str_suffix(utf8, UTF8, "Invalid UTF-8 char.\n");
}

#endif //__MODERN_STL_UTF8_H__
