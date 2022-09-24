//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#ifndef __MODERN_STL_GLOBAL_H__
#define __MODERN_STL_GLOBAL_H__

#define MSTL_PANIC(...) mstl::panic(__FILE__, __LINE__, __VA_ARGS__)

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <utility/print.h>
#include <intrinsics.h>

namespace mstl {
    using u8    = std::uint8_t;
    using u16   = std::uint16_t;
    using u32   = std::uint32_t;
    using u64   = std::uint64_t;
    using usize = std::uintptr_t;

    using i8    = std::int8_t;
    using i16   = std::int16_t;
    using i32   = std::int32_t;
    using i64   = std::int64_t;
    using isize = std::intptr_t;

    template <basic::Printable Arg, basic::Printable ...Args>
    MSTL_INLINE inline std::ostream& print(std::ostream& os, Arg&& arg, Args&& ...args) {
        os << arg << std::endl;
        if constexpr (sizeof...(args) != 0) {
            return print(os, std::forward<Args>(args)...);
        }
        return os;
    }

    template <basic::Printable ...Args>
    MSTL_NORETURN MSTL_INLINE inline void panic(const char* filename, int line, Args&& ...args) noexcept {
        std::cerr << "Panicked at " << filename << ":" << line << std::endl;
        if constexpr (sizeof...(args)) {
            std::cerr << "Messages:\n";
        }
        print(std::cerr, std::forward<Args>(args)...);
        std::exit(101);
    }
}

#endif //__MODERN_STL_GLOBAL_H__
