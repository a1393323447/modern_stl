//
// Created by 朕与将军解战袍 on 2022/9/29.
//
// 为了防止循环导入头文件

#ifndef __MODERN_STL_ENCODING_INFO_H__
#define __MODERN_STL_ENCODING_INFO_H__

#include <mstl/str/encoding/utility/code_type.h>
#include <mstl/iter/iter_concepts.h>

namespace mstl::str::concepts {
    template<typename E>
    concept EncodingInfo = requires {
        requires std::same_as<decltype(E::MAX_LEN), const usize>;
        requires (E::MAX_LEN > 0 && E::MAX_LEN <= 0xff);
        requires std::same_as<typename E::CodeType, encoding::BlockCode> ||
                 std::same_as<typename E::CodeType, encoding::VariableLengthCode>;
    };
}

#endif //__MODERN_STL_ENCODING_INFO_H__
