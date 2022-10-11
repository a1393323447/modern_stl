//
// Created by Shiroan on 2022/9/14.
//

#include <iostream>
#include <mstl.h>
#include "../TrackingAllocator.h"

#define BOOST_TEST_MODULE Vector Test

#include <boost/test/unit_test.hpp>

using namespace mstl;
using namespace collection;
using mstl::utility::to_string;

static_assert(iter::Iterator<collection::VectorIter<int>>);
static_assert(iter::IntoIterator<collection::Vector<int, TrackingAllocator<>>>);

#define INTVEC Vector<int, TrackingAllocator<>>{0, 1, 2, 3}

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
    auto b = INTVEC;
    a.push_back(10);
    BOOST_TEST_CHECK(a[0] == 10);
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

BOOST_AUTO_TEST_CASE(ITER_BASED_CONSTRUCT_TEST) {
    int x[] = {0, 1, 2, 3};
    auto a = Vector<int, TrackingAllocator<>>{x, x + 4};
    auto b = INTVEC;
    BOOST_CHECK(a == b);
}

BOOST_AUTO_TEST_CASE(DEFAULT_TEST) {
    auto a = Vector<int, TrackingAllocator<>>(10);
    auto b = Vector<int, TrackingAllocator<>>(10, 5);
    auto c = Vector<int, TrackingAllocator<>>();

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
    auto c = Vector<int, TrackingAllocator<>>{};
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

    auto c = Vector<int, TrackingAllocator<>>{};
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
    auto b = iter::collect<Vector<int, TrackingAllocator<>>>(iter);

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
    auto a = Vector<int, TrackingAllocator<>>(10, 0);
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

    auto e = iter::collect<Vector<int, TrackingAllocator<>>>(std::move(c));
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

    auto x = iter::collect<Vector<int, TrackingAllocator<>>>(a.iter());

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
            iter::CollectAs<Vector<int, TrackingAllocator<>>>{}
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

BOOST_AUTO_TEST_CASE(ERASE_TEST) {
    auto a = INTVEC;
    auto b = a;

    auto loa = a.begin() + 1;
    auto r1 = a.erase(loa);
    BOOST_TEST_CHECK(to_string(a) == "Vec [0, 2, 3]");
    BOOST_TEST_CHECK(*r1 == 2);

    VectorIter<int> lob = b.begin() + 1, hib = b.begin() + 3;
    auto r2 = b.erase(lob, hib);
    BOOST_TEST_CHECK(to_string(b) == "Vec [0, 3]");
    BOOST_TEST_CHECK(*r2 == 3);
}

BOOST_AUTO_TEST_CASE(EMPLACE_TEST) {
    auto a = INTVEC;
    auto lo = a.begin() + 1;

    a.emplace(lo, 10);
    BOOST_TEST_CHECK(to_string(a) == "Vec [0, 10, 1, 2, 3]");
}

BOOST_AUTO_TEST_CASE(EMPLACE_TEST_STR) {
    auto a = STRVEC;
    auto lo = a.begin() + 1;

    a.emplace(lo, "H");
    BOOST_TEST_CHECK(to_string(a) == "Vec [foo, H, bar, Hello, World]");
}

BOOST_AUTO_TEST_CASE(INSERT_TEST) {
    auto a = STRVEC;
    auto b = a, c = a, d = a, e = a;

    std::string ss[] = {"10", "10"};
    std::string s = "10";

    auto ra = a.insert(a.begin() + 1, s);  // ["foo", "10", "bar", "Hello", "World"]
    BOOST_TEST_CHECK(to_string(a) == "Vec [foo, 10, bar, Hello, World]");
    BOOST_TEST_CHECK(*ra == "10");

    auto rb = b.insert(b.begin() + 1, std::move(s));  // ["foo", "10", "bar", "Hello", "World"]
    BOOST_TEST_CHECK(to_string(b) == "Vec [foo, 10, bar, Hello, World]");
    BOOST_TEST_CHECK(*rb == "10");

    auto rc = c.insert(c.begin() + 1, 2, "10");  // ["foo", "10", "10", "bar", "Hello", "World"]
    BOOST_TEST_CHECK(to_string(c) == "Vec [foo, 10, 10, bar, Hello, World]");
    BOOST_TEST_CHECK(*rc == "10");

    auto rd = d.insert(d.begin() + 1, ss, ss + 2);  // ["foo", "10", "10", "bar", "Hello", "World"]
    BOOST_TEST_CHECK(to_string(d) == "Vec [foo, 10, 10, bar, Hello, World]");
    BOOST_TEST_CHECK(*rd == "10");

    auto re = e.insert(e.begin() + 1, {"10", "10"});  // ["foo", "10", "10", "bar", "Hello", "World"]
    BOOST_TEST_CHECK(to_string(e) == "Vec [foo, 10, 10, bar, Hello, World]");
    BOOST_TEST_CHECK(*re == "10");
}

BOOST_AUTO_TEST_CASE(MEMORY_TRACK) {
    std::cout << std::endl;
    std::cout << "================= MEMORY TRACK =================" << std::endl;
    TrackingAllocator<>::dump();
    std::cout << "================= MEMORY TRACK =================" << std::endl;

    BOOST_REQUIRE(TrackingAllocator<>::get_beholding_memory() == 0);
}

consteval usize pow(const usize b, const usize ex) {
    usize res = 1;
    for (usize i = 0; i < ex; i++) {
        res *= b;
    }
    return res;
}

template<usize b>
consteval auto get_pows(const usize ex) {
    mstl::collection::Vector<usize> tmp;
    Array<usize, b + 1> res = {0};
    for (usize i = 0; i <= b; i++) {
        tmp.push_back(pow(i, ex));
    }
    std::copy(tmp.begin(), tmp.end(), res.begin());
    return res;
}

BOOST_AUTO_TEST_CASE(CONSTEXPR_TEST) {
    constexpr auto pows = get_pows<10>(2);

    static_assert(pows[2] == 4);
}