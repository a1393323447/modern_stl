//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_ASCII_H__
#define __MODERN_STL_ASCII_H__

#include <collection/array.h>
#include <option/option.h>
#include <str/basic_char.h>
#include <iter/iter_concepts.h>
#include "str/encoding/utility/compile_time_validation.h"
#include "str/encoding/utility/code_type.h"
#include <str/encoding/concepts/decode.h>

namespace mstl::str::encoding {
    struct Ascii {
        static constexpr usize MAX_LEN = 1;
        using CodeType = BlockCode;
        using Char = BasicChar<Ascii>;

        template<iter::Iterator Iter>
        requires std::same_as<typename Iter::Item, const u8 &>
        MSTL_INLINE constexpr static
        Option<Char> next(Iter &iter) {
            Option<const u8 &> next_byte = iter.next();
            if (next_byte.is_some()) {
                Char ascii_char = ValidBytes<1, Ascii> {{next_byte.unwrap_unchecked()} };
                return Option<Char>::some(ascii_char);
            } else {
                return Option<Char>::none();
            }
        }

        template<iter::DoubleEndedIterator Iter>
        requires std::same_as<typename Iter::Item, const u8 &>
        MSTL_INLINE constexpr static
        Option<Char> last(Iter &iter) {
            Option<const u8 &> last_byte = iter.prev();
            if (last_byte.is_some()) {
                Char ascii_char = ValidBytes<1, Ascii> {{last_byte.unwrap_unchecked()} };
                return Option<Char>::some(ascii_char);
            } else {
                return Option<Char>::none();
            }
        }

        template<iter::Iterator Iter>
        requires std::same_as<typename Iter::Item, const u8 &>
        MSTL_INLINE constexpr static
        Option<DecodeError>
        validate(Iter &&iter) {
            usize pos = 0;
            for (auto next = iter.next(); next.is_some(); next = iter.next()) {
                if (next.as_ref_uncheck() > 128) {
                    return Option<DecodeError>::some({pos});
                }
                pos++;
            }
            return Option<DecodeError>::none();
        }
    };

    define_str_suffix(ascii, Ascii, "Invalid ascii code in string literal");
}

#endif //__MODERN_STL_ASCII_H__
