//
// Created by 朕与将军解战袍 on 2022/9/18.
//

#ifndef __MODERN_STL_INTRINSICS_H__
#define __MODERN_STL_INTRINSICS_H__

#ifdef __has_cpp_attribute
    #if __has_cpp_attribute(gnu::always_inline)
    #define MSTL_INLINE [[gnu::always_inline]]
    #elif __has_cpp_attribute(__gnu__::__gnu_inline__)
    #define MSTL_INLINE [[__gnu__::__gnu_inline__]]
    #elif __has_cpp_attribute(clang::always_inline)
    #define MSTL_INLINE [[clang::always_inline]]
    #elif __has_cpp_attribute(_Clang::__always_inline__)
    #define MSTL_INLINE [[_Clang::__always_inline__]]
    #else
    #define MSTL_INLINE
    #endif
#endif

#ifdef __has_cpp_attribute
    #if __has_cpp_attribute(noreturn)
    #define MSTL_NORETURN [[noreturn]]
    #elif __has_cpp_attribute(gnu::noreturn)
    #define MSTL_NORETURN [[gnu::noreturn]]
    #else
    #define MSTL_NORETURN
    #endif
#endif

#ifdef MSTL_DEBUG
#undef MSTL_INLINE
#define MSTL_INLINE
#endif

#endif //__MODERN_STL_INTRINSICS_H__
