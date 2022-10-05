//
// Created by Shiroan on 2022/9/23.
//

#ifndef MODERN_STL_ALLOCATOR_H
#define MODERN_STL_ALLOCATOR_H

#include <memory/layout.h>

namespace mstl::memory::allocator {

    /// 预置的分配器类.
    class Allocator{
    public:
        /**
         * @brief 分配空间.
         *
         * 使用 [`::operator new(usize, std::nothrow_t{})`](https://en.cppreference.com/w/cpp/memory/new/operator_new)
         * 或 [`::operator new(usize, std::align_val_t, std::nothrow_t{})`](https://en.cppreference.com/w/cpp/memory/new/operator_new) 分配空间.
         * @param layout 描述所分配的内存空间的Layout.
         * @param length 需要容纳的layout所描述的类型的对象的数量.
         * @return 若分配成功, 则返回指向新分配的空间的首地址的指针; 否则, 返回nullptr.
         */
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

        /**
         * @brief 解分配空间.
         *
         * 使用 [`::operator delete(void*, usize, std::nothrow_t{})`](https://en.cppreference.com/w/cpp/memory/new/operator_delete)
         * 或 [`::operator delete(void*, usize, std::align_val_t, std::nothrow_t{})`](https://en.cppreference.com/w/cpp/memory/new/operator_delete) 解分配空间.
         */
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
    };
}

#endif //MODERN_STL_ALLOCATOR_H