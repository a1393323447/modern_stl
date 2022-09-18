//
// Created by 朕与将军解战袍 on 2022/9/16.
//

#ifndef __MODERN_STL_RANGE_H__
#define __MODERN_STL_RANGE_H__

namespace mstl::ops {
    template<typename Idx>
    struct Range {
        Idx start;
        Idx end;
    };
}

#endif //__MODERN_STL_RANGE_H__
