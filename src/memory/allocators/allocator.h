//
// Created by Shiroan on 2022/9/23.
//

#ifndef MODERN_STL_ALLOCATOR_H
#define MODERN_STL_ALLOCATOR_H

#include <memory/layout.h>

namespace mstl::memory::allocator {
    class Allocator{
    public:
        void* allocate(const Layout& layout, usize length) noexcept { // NOLINT(readability-convert-member-functions-to-static)
            return ::operator new(layout.size * length, (std::align_val_t)layout.align, std::nothrow_t{});
        }

        void deallocate(void* ptr, const Layout& layout, usize) noexcept{ // NOLINT(readability-convert-member-functions-to-static)
            ::operator delete(ptr, (std::align_val_t)layout.align);
        }
    };
}

#endif //MODERN_STL_ALLOCATOR_H