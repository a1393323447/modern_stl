//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_DECODE_RESULT_H__
#define __MODERN_STL_DECODE_RESULT_H__

#include <global.h>
#include <option/option.h>

// std
#include <ostream>

namespace mstl::str::encoding {
    struct DecodeError {
        usize valid_up_to{};
    };

    std::ostream& operator<<(std::ostream& os, const DecodeError& error) {
        os << "Decode Error: valid_up_to " << error.valid_up_to;
        return os;
    }
}



#endif //__MODERN_STL_DECODE_RESULT_H__
