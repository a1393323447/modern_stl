//
// Created by 朕与将军解战袍 on 2022/9/18.
//

#ifndef __MODERN_STL_INTRINSICS_H__
#define __MODERN_STL_INTRINSICS_H__

#if __has_cpp_attribute(__gnu__::__always_inline__)
#define MSTL_INLINE [[__gnu__::__always_inline__]]
#elif __has_cpp_attribute(gnu::always_inline)
#define MSTL_INLINE [[gnu::always_inline]]
#elif __has_cpp_attribute(clang::always_inline)
#define MSTL_INLINE [[clang::always_inline]]
#elif __has_cpp_attribute(msvc::forceinline)
#define MSTL_INLINE [[msvc::forceinline]]
#else
#define MSTL_INLINE [[__gnu__::__always_inline__]]
#endif

#endif //__MODERN_STL_INTRINSICS_H__
