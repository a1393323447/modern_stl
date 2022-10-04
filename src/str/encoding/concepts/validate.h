//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_VALIDATE_H__
#define __MODERN_STL_VALIDATE_H__

#include "global.h"
#include "option/option.h"
#include "iter/iter_concepts.h"
#include "str/encoding/decode_error.h"

namespace mstl::str::concepts {
    template<typename E, typename Iter>
    concept EncodingValidation = requires(Iter &iter) {
        requires iter::ContinuousIterator<Iter>;
        requires std::same_as<typename Iter::Item, const u8 &>;
        { E::validate(iter) } -> std::same_as<Option<encoding::DecodeError>>;
    };
}

#endif //__MODERN_STL_VALIDATE_H__
