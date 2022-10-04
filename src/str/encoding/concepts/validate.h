//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_VALIDATE_H__
#define __MODERN_STL_VALIDATE_H__

#include <concepts>

#include "global.h"
#include "str/encoding/decode_error.h"
#include "result/result.h"
#include "utility/tuple.h"

namespace mstl::str::concepts {
    template<typename E, typename Iter>
    concept EncodingValidation = requires(Iter &iter) {
        requires std::same_as<typename Iter::Item, const u8 &>;
        { E::validate(iter) } -> std::same_as<result::Result<utility::Unit, encoding::DecodeError>>;
    };
}

#endif //__MODERN_STL_VALIDATE_H__
