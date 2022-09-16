//
// Created by Shiroan on 2022/9/14.
//

#include <iostream>
#include <collection/vector.h>
#include <iter/iter_concepts.h>

#define BOOST_TEST_MODULE Vector Test
#include <boost/test/unit_test.hpp>

using namespace mstl;
using namespace collection;
static_assert(iter::Iterator<collection::VectorIter<int>>);
static_assert(iter::IntoIterator<collection::Vector<int>>);

BOOST_AUTO_TEST_CASE(BASIC_TEST) {
    auto a = Vector<int>{0, 1, 2, 3};

    BOOST_REQUIRE(a.size() == 4);
    BOOST_CHECK(a[0] == 0);
    BOOST_CHECK(a[1] == 1);
    std::cout << a.capacity() << std::endl;

    a.push_back(10);
    BOOST_REQUIRE(a.size() == 5);
    BOOST_CHECK(a[4] == 10);

    BOOST_CHECK(a.front().unwrap() ==  0);
    BOOST_CHECK(a.back().unwrap() ==  10);
    BOOST_CHECK(a.at(2).unwrap() == 2);
    BOOST_CHECK(a.at(10).is_none());

    a.clear();
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.capacity() == 0);
    BOOST_CHECK(a.empty());
}

BOOST_AUTO_TEST_CASE (COPY_TEST) {
    auto a = Vector<int>{0, 1, 2, 3};
    auto b = a;

    BOOST_REQUIRE(a.size() == b.size());
    BOOST_CHECK(a[0] == b[0]);
    BOOST_CHECK(a == b);

    a[0] = 10;
    BOOST_CHECK(a[0] != b[0]);
}

BOOST_AUTO_TEST_CASE(MOVE_TEST) {
    auto a = Vector<int>{0, 1, 2, 3};
    BOOST_REQUIRE(a.size() == 4);

    auto b = std::move(a);
    BOOST_CHECK(a.empty());
    BOOST_CHECK(a.capacity() == 0);

    BOOST_CHECK(b.size() == 4);
    BOOST_CHECK(b[0] == 0);
}

//BOOST_AUTO_TEST_CASE(FOREACH_TEST) {
//    auto a = Vector<int>{0, 1, 2, 3};
//    std::cout << a << std::endl;
//}