//
// Created by Shiroan on 2022/9/23.
//

#ifndef MODERN_STL_LAYOUT_ERROR_H
#define MODERN_STL_LAYOUT_ERROR_H

#include <ostream>

namespace mstl::memory {
    class LayoutError{};

    std::ostream &operator<<(std::ostream& os, const LayoutError&) {
        return os << "invalid parameters to Layout::from_size_align";
    }
}

#endif //MODERN_STL_LAYOUT_ERROR_H
