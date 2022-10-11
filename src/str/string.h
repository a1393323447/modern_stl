//
// Created by 朕与将军解战袍 on 2022/9/29.
//

#ifndef __MODERN_STL_STRING_H__
#define __MODERN_STL_STRING_H__

#include <str/basic_string.h>
#include <str/encoding/ascii.h>
#include <str/encoding/utf8.h>

/**
 * mstl::str 中提供了
 * - BasicChar<Encoding>: 底层无编码的字符, 用 Encoding 指示该字符的编码
 *      - AsciiChar: Ascii 编码的字符
 *      - UTF8Char: UTF8 编码的字符
 *
 * - BasicString<Encoding>: 底层无编码的字符串, 用 Encoding 指示该字符的编码
 *      - AsciiString: Ascii 编码的字符串
 *      - UTF8String: UTF8 编码的字符串
 *
 * mstl::str::concepts 中提供了
 * - DecodeNext 和 DecodeLast 两个与解码相关的 concept
 * - EncodingInfo concept 用于提取与字符编码相关的信息
 * - EncodingValidation 用于指导对字符编码正确性的检查
 *
 * mstl::str::encoding 中提供了
 * - BlockCode: 用于在 EncodingInfo 中指示字符编码为定长编码
 * - VariableLengthCode: 用于在 EncodingInfo 中指示字符编码为变长编码
 * - ValidBytes: 是一个对 u8 数组的包装类, 用于表明该 u8 序列通过了字符编码的校验
 * - DecodeError: 是在对 u8 序列解码失败时返回的错误类型
 *
 * # Encoding
 * Encoding 是对字符编码的抽象, Encoding 必须满足 EncodingInfo 的约束,
 * 此外 Encoding 也可以选地满足 mstl::str::concepts 中包含的其它约束,
 * BasicChar 和 BasicString 也会根据 Encoding 所满足的约束, 提供不同功能。
 * ## EncodingInfo
 * EncodingInfo 要求 Encoding 中拥有:
 * - MAX_LEN: static constexpr usize , 用于指示该编码下, 单个字符的最大字节数,
 *   在当前实现中仅支持 (1 ~ 0xff) 之间的字节数
 * - CodeType: 一个类型别名, 用于指示该编码为定长编码还是变长编码, 可选值为: BlockCode 和 VariableLengthCode
 * ## DecodeNext 和 DecodeLast
 * 满足 DecodeXXX 约束的 Encoding 可以对字符进行解码。
 *
 * 满足 DecodeNext 的 Encoding 中需要有一个静态函数 next, 接收一个 u8 的迭代器, 返回下一个 BasicChar<Encoding>
 *
 * 满足 DecodeLast 的 Encoding 中需要有一个静态函数 last, 接收一个 u8 的迭代器, 返回最后一个 BasicChar<Encoding>
 *
 * ## EncodingValidation
 * 满足 EncodingValidation 约束的 Encoding 可以对字符编码进行校验。
 *
 * EncodingValidation 约束要求 Encoding 中需要有一个静态函数 validate 接收一个 u8 的迭代器,
 * 返回 Option<DecodeError> 表示编码中是否有错误。如果 validate 是一个 constexpr 还可以通过
 * str/encoding/utility/compile_time_validation.h 中提供的宏 define_suffix 快速定义
 * 该字符编码专用的后缀运算符。
 *
 * ```cpp
 * // in file str/encoding/ascii.h
 * define_str_suffix(ascii, Ascii, "Invalid ascii code in string literal");
 *
 * // example
 * AsciiChar   ch  = "Hi"_ascii; // check failed: "too many letter in a char"
 * AsciiString str = "Hi"_ascii; // check pass
 * AsciiString non_ascii = "你好"_ascii; // check failed: "Invalid ascii code in string literal"
 * ```
 *
 * # BasicChar
 * BasicChar 是一种底层无编码的字符, 用 Encoding 指示该字符的编码.
 * ## Basic usage
 * ### 构造
 * BasicChar 可通过 Encoding 提供的后缀运算符构造:
 * ```cpp
 * AsciiChar ch  = "H"_ascii;
 * ```
 * ### 成员函数
 * 因为 BasicChar 可能是变长编码的字符, 所以提供了成员函数 get_len() 用于获得有效的编码字节数
 * ```
 * constexpr UTF8Char ch = "你"_utf8;
 * static_assert(ch.get_len() == 3);
 * ```
 *
 * ## 与输入流交互
 * BasicChar 也可以进行输出
 * ```cpp
 * constexpr UTF8Char ch2 = "你"_utf8;
 * std::cout << ch2;
 * ```
 *
 * # BasicString
 * BasicChar 是一种底层无编码的字符, 用 Encoding 指示该字符的编码.
 * ## Basic usage
 * ### 构造
 * BasicString 可通过 Encoding 提供的后缀运算符构造:
 * ```cpp
 * UTF8String utf8_str = "你好啊"_utf8;
 * ```
 * ### 成员函数
 * #### push_back(ch)
 * push_back 接受一个 BasicChar<Encoding> , 并将其存入字符串末尾
 * #### pop_back()
 * 返回字符串中最后一个字符 Option<BasicChar<Encoding>>
 * #### chars()
 * 返回该字符串中的字符的迭代器
 * ```cpp
 * UTF8String utf8_str = "你好啊"_utf8;
 * utf8_str.chars() |
 * for_each([](auto& ch) {
 *     std::cout << ch << std::endl;
 * });
 * // output:
 * // 你
 * // 好
 * // 啊
 * ```
 */
namespace mstl::str {
    using AsciiChar   = BasicChar<encoding::Ascii>;
    using AsciiString = BasicString<encoding::Ascii>;

    using UTF8Char    = BasicChar<encoding::UTF8>;
    using UTF8String  = BasicString<encoding::UTF8>;
}

#endif //__MODERN_STL_STRING_H__
