//
// Created by 朕与将军解战袍 on 2022/9/29.
//
#include <mstl/mstl.h>
#include <iostream>

#define BOOST_TEST_MODULE String Test
#include <boost/test/unit_test.hpp>

using namespace mstl::iter;
using namespace mstl::str;
using namespace mstl::str::encoding;

BOOST_AUTO_TEST_CASE(ASCII_STR_TEST) {
    AsciiString str = "123"_ascii;
    std::cout << str;

    AsciiChar ch = "1"_ascii;
    std::cout << ch;

    BOOST_CHECK(str.pop_back().is_some());
    BOOST_CHECK(str.pop_back().is_some());
    BOOST_CHECK(str.pop_back().is_some());
    BOOST_CHECK(str.pop_back().is_none());

    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);
    str.push_back("1"_ascii);


    std::cout << str << std::endl;

    str.chars() | for_each([](auto ch) {
       std::cout << ch << std::endl;
    });

    UTF8String s = "你好啊\xf0\xa0\x80\x80"_utf8;
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);
    s.insert("你"_utf8, 3);

    s.chars() | for_each([](auto ch) {
        std::cout << ch << std::endl;
    });
    std::cout << s << std::endl;
}
