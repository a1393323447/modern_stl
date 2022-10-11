//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_CONCEPTS_H__
#define __MODERN_STL_CONCEPTS_H__

#include <mstl/str/basic_char.h>
#include <mstl/str/encoding/utility/code_type.h>
#include <mstl/iter/iter_concepts.h>

namespace mstl::str::concepts {
    template<typename E, typename Iter>
    concept DecodeNext = requires (Iter& iter) {
        requires iter::Iterator<Iter>;
        requires std::same_as<typename Iter::Item, const u8&>;
        { E::template next<Iter>(iter) } -> std::same_as<Option<BasicChar<E>>>;
    };

    template<typename E, typename Iter>
    concept DecodeLast = requires (Iter& iter) {
        requires iter::DoubleEndedIterator<Iter>;
        requires std::same_as<typename Iter::Item, const u8&>;
        { E::template last<Iter>(iter) } -> std::same_as<Option<BasicChar<E>>>;
    };
}

#endif //__MODERN_STL_CONCEPTS_H__
