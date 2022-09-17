//
// Created by Shiroan on 2022/9/14.
//

#include <iostream>
#include <collection/vector.h>
#include <collection/arrary.h>
#include <iter/iterator.h>
#include <utility/utility.h>

#define BOOST_TEST_MODULE Vector Test

#include <boost/test/unit_test.hpp>

using namespace mstl;
using namespace collection;

static_assert(iter::Iterator<collection::VectorIter<int>>);
static_assert(iter::IntoIterator<collection::Vector<int>>);

#define INTVEC Vector<int>{0, 1, 2, 3}

#define STRVEC Vector<std::string>{ \
    "foo",        \
    "bar",        \
    "Hello",      \
    "World"\
}

BOOST_AUTO_TEST_CASE(BASIC_TEST) {
    auto a = INTVEC;

    BOOST_REQUIRE(a.size() == 4);
    BOOST_TEST_CHECK(a[0] == 0);
    BOOST_TEST_CHECK(a[1] == 1);

    a.push_back(10);
    BOOST_REQUIRE(a.size() == 5);
    BOOST_TEST_CHECK(a[4] == 10);

    BOOST_TEST_CHECK(a.front().unwrap() == 0);
    BOOST_TEST_CHECK(a.back().unwrap() == 10);
    BOOST_TEST(a.at(2).unwrap() == 2);
    BOOST_TEST_CHECK(a.at(10).is_none());
    BOOST_TEST_CHECK(a.at_unchecked(1) == 1);

    a.pop_back();
    BOOST_TEST_REQUIRE(a.size() == 4);
    BOOST_TEST_CHECK(a.back().unwrap() == 3);

    a.clear();
    BOOST_TEST_CHECK(a.size() == 0);
    BOOST_TEST_CHECK(a.capacity() == 0);
    BOOST_TEST_CHECK(a.empty());
}

BOOST_AUTO_TEST_CASE(BASIC_TEST_STR) {
    auto a = STRVEC;

    BOOST_REQUIRE(a.size() == 4);
    BOOST_TEST_CHECK(a[0] == "foo");
    BOOST_TEST_CHECK(a[1] == "bar");

    a.push_back("10");
    BOOST_REQUIRE(a.size() == 5);
    BOOST_TEST_CHECK(a[4] == "10");

    BOOST_TEST_CHECK(a.front().unwrap() == "foo");
    BOOST_TEST_CHECK(a.back().unwrap() == "10");
    BOOST_TEST(a.at(2).unwrap() == "Hello");
    BOOST_TEST_CHECK(a.at(10).is_none());
    BOOST_TEST_CHECK(a.at_unchecked(1) == "bar");

    a.clear();
    BOOST_TEST_CHECK(a.size() == 0);
    BOOST_TEST_CHECK(a.capacity() == 0);
    BOOST_TEST_CHECK(a.empty());
}

BOOST_AUTO_TEST_CASE(DEFAULT_TEST) {
    auto a = Vector<int>(10);
    auto b = Vector<int>(10, 5);
    auto c = Vector<int>();

    BOOST_TEST_REQUIRE(a.size() == 10);
    BOOST_TEST_REQUIRE(b.size() == 10);
    BOOST_TEST_REQUIRE(c.size() == 0);

    BOOST_TEST_CHECK(a[5] == 0);
    BOOST_TEST_CHECK(b[5] == 5);
}

BOOST_AUTO_TEST_CASE(DEFAULT_TEST_STR) {
    auto a = Vector<std::string>(10);
    auto b = Vector<std::string>(10, "5");
    auto c = Vector<std::string>();

    BOOST_TEST_REQUIRE(a.size() == 10);
    BOOST_TEST_REQUIRE(b.size() == 10);
    BOOST_TEST_REQUIRE(c.size() == 0);

    BOOST_TEST_CHECK(a[5] == "");
    BOOST_TEST_CHECK(b[5] == "5");
}

BOOST_AUTO_TEST_CASE (COPY_TEST) {
    auto a = INTVEC;
    auto b = a;
    auto c = Vector<int>{};
    c = b;

    BOOST_REQUIRE(a.size() == b.size());
    BOOST_CHECK(a[0] == b[0]);
    BOOST_CHECK(a == b);
    BOOST_CHECK(b == c);

    a[0] = 10;
    BOOST_CHECK(a[0] != b[0]);
}

BOOST_AUTO_TEST_CASE (COPY_TEST_STR) {
    auto a = STRVEC;
    auto b = a;
    auto c = Vector<std::string>{};
    c = b;

    BOOST_REQUIRE(a.size() == b.size());
    BOOST_CHECK(a[0] == b[0]);
    BOOST_CHECK(a == b);
    BOOST_CHECK(b == c);

    a[0] = "10";
    BOOST_CHECK(a[0] != b[0]);
}

BOOST_AUTO_TEST_CASE(MOVE_TEST) {
    auto a = INTVEC;
    BOOST_REQUIRE(a.size() == 4);

    auto b = std::move(a);


    BOOST_CHECK(b.size() == 4);
    BOOST_CHECK(b[0] == 0);

    auto c = Vector<int>{};
    c = std::move(b);

    BOOST_CHECK(c.size() == 4);
    BOOST_CHECK(c[0] == 0);
}

BOOST_AUTO_TEST_CASE(MOVE_TEST_STR) {
    auto a = STRVEC;
    BOOST_REQUIRE(a.size() == 4);

    auto b = std::move(a);


    BOOST_CHECK(b.size() == 4);
    BOOST_CHECK(b[0] == "foo");

    auto c = Vector<std::string>{};
    c = std::move(b);

    BOOST_CHECK(c.size() == 4);
    BOOST_CHECK(c[0] == "foo");
}

BOOST_AUTO_TEST_CASE(ASSIGN_TEST) {
    auto a = INTVEC;
    BOOST_REQUIRE(a.size() == 4);
    a.assign({5, 6, 7});
    BOOST_REQUIRE(a.size() == 3);
    BOOST_CHECK(a[2] == 7);
    a.assign(2, 10);
    BOOST_CHECK(a.size() == 2);
    BOOST_CHECK(a[1] == 10);

    auto b = STRVEC;
    BOOST_REQUIRE(b.size() == 4);
    b.assign({"FOO", "BAR"});
    BOOST_REQUIRE(b.size() == 2);
    BOOST_CHECK(b[0] == "FOO");
    b.assign(3, "BAR");
    BOOST_REQUIRE(b.size() == 3);
    BOOST_CHECK(b[1] == "BAR");
}

BOOST_AUTO_TEST_CASE(FROM_ITER_TEST) {
    auto a = Array<int, 4>{0, 1, 2, 3};
    auto iter = a.into_iter();
    auto b = iter::collect<Vector<int>>(iter);

    auto c = INTVEC;
    BOOST_CHECK(b == c);
}

BOOST_AUTO_TEST_CASE(FROM_ITER_TEST_STR) {
    auto a = Array<std::string, 4>{"foo", "bar", "Hello", "World"};
    auto iter = a.into_iter();
    auto b = iter::collect<Vector<std::string>>(iter);

    auto c = STRVEC;
    BOOST_CHECK(b == c);
}

BOOST_AUTO_TEST_CASE(RESERVE_TEST) {
    auto a = INTVEC;
    BOOST_CHECK(a.capacity() == 4);
    a.reserve(8);
    BOOST_CHECK(a.capacity() == 8);
}

BOOST_AUTO_TEST_CASE(SWAP_TEST) {
    auto a = Vector<int>(10, 0);
    auto b = INTVEC;

    a.swap(b);

    BOOST_REQUIRE(a.size() == 4);
    BOOST_REQUIRE(b.size() == 10);

    BOOST_CHECK(a[2] == 2);
    BOOST_CHECK(b[2] == 0);
}

BOOST_AUTO_TEST_CASE(SWAP_TEST_STR) {
    auto a = Vector<std::string>(10, "0");
    auto b = STRVEC;

    a.swap(b);

    BOOST_REQUIRE(a.size() == 4);
    BOOST_REQUIRE(b.size() == 10);

    BOOST_CHECK(a[2] == "Hello");
    BOOST_CHECK(b[2] == "0");
}

BOOST_AUTO_TEST_CASE(COMPARE_TEST) {
    auto a = INTVEC;
    auto b = a;
    BOOST_CHECK(a == b);

    b.push_back(1);
    BOOST_CHECK(b > a);

    a[0] = 10;
    BOOST_CHECK(a > b);
}

BOOST_AUTO_TEST_CASE(INTO_ITER_TEST) {
    auto a = INTVEC;
    auto b = INTVEC;

    auto c = a.into_iter();
    auto d = b.into_iter_reversed();

    BOOST_REQUIRE(a.empty());  // a and b have been moved
    BOOST_REQUIRE(b.empty());  // so that they are empty as defined

    auto e = iter::collect<Vector<int>>(std::move(c));
    auto f = INTVEC;
    BOOST_CHECK(e == f);

    BOOST_REQUIRE(d.next().unwrap() == 3);
    BOOST_REQUIRE(d.next().unwrap() == 2);
    BOOST_REQUIRE(d.next().unwrap() == 1);
    BOOST_REQUIRE(d.next().unwrap() == 0);
    BOOST_REQUIRE(d.next().is_none());
}

BOOST_AUTO_TEST_CASE(INTO_ITER_TEST_STR) {
    auto a = STRVEC;
    auto b = STRVEC;

    auto c = a.into_iter();
    auto d = b.into_iter_reversed();

    BOOST_REQUIRE(a.empty());  // a and b have been moved
    BOOST_REQUIRE(b.empty());  // so that they are empty as defined

    auto e = iter::collect<Vector<std::string>>(std::move(c));
    auto f = STRVEC;
    BOOST_CHECK(e == f);

    BOOST_REQUIRE(d.next().unwrap() == "World");
    BOOST_REQUIRE(d.next().unwrap() == "Hello");
    BOOST_REQUIRE(d.next().unwrap() == "bar");
    BOOST_REQUIRE(d.next().unwrap() == "foo");
    BOOST_REQUIRE(d.next().is_none());
}

BOOST_AUTO_TEST_CASE(ITERATION_TEST) {
    auto a = INTVEC;

    auto x = iter::collect<Vector<int>>(a.iter());

    BOOST_CHECK(a[2] == 2);
    BOOST_CHECK(x[2] == 2);

    std::cout << "ITERATION_TEST [";  // Test for range-based for statement
    for (const auto &i: x) {
        std::cout << i << ", ";
    }
    std::cout << "]" << std::endl;

    auto b = iter::combine(
            a.iter(),
            iter::Map{}, [](int a) {
                return a * 2;
            },
            iter::CollectAs<Vector<int>>{}
    );

    BOOST_TEST_CHECK(to_string(b) == "Vec [0, 2, 4, 6]");
}

BOOST_AUTO_TEST_CASE(RESIZE_TEST) {
    auto a = INTVEC;
    a.resize(6);
    BOOST_TEST_REQUIRE(a.size() == 6);
    BOOST_TEST_CHECK(to_string(a) == "Vec [0, 1, 2, 3, 0, 0]");

    a.resize(3);
    BOOST_TEST_REQUIRE(a.size() == 3);
    BOOST_TEST_CHECK(to_string(a) == "Vec [0, 1, 2]");

    a.resize(5, 10);
    BOOST_TEST_REQUIRE(a.size() == 5);
    BOOST_TEST_CHECK(to_string(a) == "Vec [0, 1, 2, 10, 10]");

}
