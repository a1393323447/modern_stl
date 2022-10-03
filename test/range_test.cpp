//
// Created by 朕与将军解战袍 on 2022/9/21.
//
#include <global.h>
#include <ops/range.h>
#include <iter/iterator.h>

#define BOOST_TEST_MODULE Range Test
#include <boost/test/unit_test.hpp>

using namespace mstl;
using namespace mstl::ops;

BOOST_AUTO_TEST_CASE(RANGE_EQ_TEST) {
    auto range_1 = Range<i32>(1, 10);
    auto range_2 = Range<i32>(2, 10);
    auto range_3 = Range<u32>(1, 10);
    auto range_4 = Range<u32>(2, 10);

    BOOST_CHECK(range_1 != range_2);
    BOOST_CHECK(range_1 == range_3);
    BOOST_CHECK(range_1 != range_4);
    BOOST_CHECK(range_2 != range_3);
    BOOST_CHECK(range_2 == range_4);
}

BOOST_AUTO_TEST_CASE(RANGE_CONTAIN_TEST) {
    auto range_1 = Range<i32>(1, 10);
    for(auto i: range_1) {
        BOOST_CHECK(range_1.contains(i));
    }
}

BOOST_AUTO_TEST_CASE(RANGE_COURSE_CMP_TEST) {
    auto range_1 = Range<i32>(1, 10);
    auto range_2 = Range<i32>(2, 10);
    auto range_3 = Range<u32>(1, 10);
    auto range_4 = Range<u32>(2, 10);

    auto b1 = range_1.begin();
    auto b2 = range_2.begin();
    auto b3 = range_3.begin();
    auto b4 = range_4.begin();

    BOOST_CHECK(b1 != b2);
    BOOST_CHECK(b1 == b1);
    BOOST_CHECK(b3 != b4);
    BOOST_CHECK(b3 == b3);
    BOOST_CHECK(b2 >  b1);
    BOOST_CHECK(b2 >= b1);
    BOOST_CHECK(b1 <= b2);
    BOOST_CHECK(b1 <= b1);
    BOOST_CHECK(b1 >= b1);
}

BOOST_AUTO_TEST_CASE(RANGE_CPP_ITER_TEST) {
    auto range_1 = Range<i32>(1, 10);
    auto range_2 = Range<u32>(1, 10);
    auto range_3 = Range<char>('a', 'z');

    i32 pos_1 = 1;
    for (auto i: range_1) {
        BOOST_CHECK(i == pos_1);
        pos_1++;
    }

    u32 pos_2 = 1;
    for (auto i: range_2) {
        BOOST_CHECK(i == pos_2);
        pos_2++;
    }

    char ch = 'a';
    for (auto i: range_3) {
        BOOST_CHECK(i == ch);
        ch++;
    }

    u32 pos_3 = 1;
    for (u32 i: Range(1, 10)) {
        BOOST_CHECK(i == pos_3);
        pos_3++;
    }
}

BOOST_AUTO_TEST_CASE(RANGE_MSTL_ITER_TEST) {
    auto range_1 = Range<i32>(1, 10);
    auto range_2 = Range<u32>(1, 10);
    auto range_3 = Range<char>('a', 'z');

    i32 pos_1 = 1;
    iter::combine(range_1,
        iter::ForEach{}, [&](auto i) {
            BOOST_CHECK(i == pos_1);
            pos_1++;
        }
    );

    u32 pos_2 = 1;
    iter::combine(range_2,
        iter::ForEach{}, [&](auto i) {
            BOOST_CHECK(i == pos_2);
            pos_2++;
        }
    );

    char ch = 'a';
    iter::combine(range_3,
        iter::ForEach{}, [&](auto i) {
            BOOST_CHECK(i == ch);
            ch++;
        }
    );

    u32 pos_3 = 1;
    iter::combine(Range(1, 10),
        iter::ForEach{}, [&](u32 i) {
            BOOST_CHECK(i == pos_3);
            pos_3++;
        }
    );
}