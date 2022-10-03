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
#if __cpp_aligned_new
            if (layout.align > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            {
                auto al = std::align_val_t(layout.align);
                return ::operator new(layout.size * length, al, std::nothrow_t{});
            }
#endif
            return ::operator new(layout.size * length, std::nothrow_t{});
        }

        void deallocate(void* ptr, const Layout& layout, usize len) noexcept{ // NOLINT(readability-convert-member-functions-to-static)
#if __cpp_sized_deallocation
# define MSTL_SIZED_DEALLOC(p, n) (p), (n) * (layout.size)
#else
# define MSTL_SIZED_DEALLOC(p, n) (p)
#endif

#if __cpp_aligned_new
            if (layout.align > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            {
                ::operator delete(MSTL_SIZED_DEALLOC(ptr, len), std::align_val_t{layout.align});
                return;
            }
#endif
            ::operator delete(MSTL_SIZED_DEALLOC(ptr, len));
        }

        template<typename T>
        T* allocate(usize length) noexcept {
            constexpr Layout layout = Layout::from_type<T>();
            return (T*)allocate(layout, length);
        }

        template<typename T>
        void deallocate(T* ptr, usize len) noexcept {
            constexpr Layout layout = Layout::from_type<T>();
            deallocate(ptr, layout, len);
        }
    };
}

#endif //MODERN_STL_ALLOCATOR_H