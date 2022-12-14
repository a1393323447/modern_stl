//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_VALIDATE_BYTES_H__
#define __MODERN_STL_VALIDATE_BYTES_H__

#include <bit>
#include <mstl/global.h>
#include <mstl/collection/array.h>

#define define_str_suffix(name, encode, msg)                                   \
template<mstl::str::encoding::CharsWrap wrap>                                  \
constexpr                                                                      \
decltype(auto) operator ""_##name() noexcept {                                 \
    constexpr usize LEN = decltype(wrap)::LEN - 1;                             \
    static_assert(LEN > 0, "empty string.\n");                                 \
    using ReturnType =  mstl::str::encoding::ValidBytes<LEN, encode>;          \
    constexpr mstl::collection::Array <u8, LEN> arr { wrap.chars };            \
    constexpr auto op = encode::validate(arr.iter());                          \
    static_assert(op.is_none(), msg);                                          \
    return ReturnType{ arr };                                                  \
}                                                                              \
static_assert(true)

namespace mstl::str::encoding {
    template<usize N, typename E>
    struct ValidBytes {
        using Encoding = E;
        const collection::Array<u8, N> arr;
    };

    template<usize N>
    struct CharsWrap {
        static constexpr usize LEN = N;
        u8 chars[N - 1]{};

        constexpr CharsWrap(char const(&s)[N]) {
            for (usize i = 0; i < N - 1; i++) {
                chars[i] = s[i];
            }
        };
    };
}

#endif //__MODERN_STL_VALIDATE_BYTES_H__
