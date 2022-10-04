//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_STRING_H__
#define __MODERN_STL_STRING_H__

#include <str/basic_string.h>
#include <str/encoding/ascii.h>
#include <str/encoding/utf8.h>

namespace mstl::str {
    using AsciiChar   = BasicChar<encoding::Ascii>;
    using AsciiString = BasicString<encoding::Ascii>;

    using UTF8Char    = BasicChar<encoding::UTF8>;
    using UTF8String  = BasicString<encoding::UTF8>;
}

#endif //__MODERN_STL_STRING_H__
