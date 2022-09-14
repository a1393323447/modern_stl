//
// Created by Shiroan on 2022/9/14.
//
#include <utility/tuple.h>
#include <string>

#define BOOST_TEST_MODULE Tuple Test
#include <boost/test/unit_test.hpp>

using namespace mstl::utility;
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_CASE(BASIC_TEST) {
    auto a = make_tuple();
    auto b = make_tuple();
    BOOST_CHECK(a.size() == b.size() && a.size() == 0);
    auto c = make_tuple(0, 1, 2.0);
    BOOST_CHECK(c.size() == 3);

    static_assert(std::same_as<TupleElement<0, decltype(c)>::type, int>);
    static_assert(std::same_as<TupleElement<2, decltype(c)>::type, double>);
}

BOOST_AUTO_TEST_CASE(GET_TEST, * utf::tolerance(1e-6)) {
    auto t = make_tuple(0, 1, 2.0, std::string("Hello"));
    BOOST_CHECK(get<0>(t) == 0);
    BOOST_CHECK(get<1>(t) == 1);
    BOOST_CHECK(get<2>(t) == 2.0);
    BOOST_CHECK(get<3>(t) == "Hello");
}

BOOST_AUTO_TEST_CASE(TIE_TEST, * utf::tolerance(1e-6)) {
    auto t = make_tuple(0, 1, 2.0, std::string("Hello"));
    int a, b;
    double c;
    std::string d;
    tie(a, b, c, d) = t;
    BOOST_CHECK(a == 0);
    BOOST_CHECK(b == 1);
    BOOST_CHECK(c == 2.0);
    BOOST_CHECK(d == "Hello");
}

BOOST_AUTO_TEST_CASE(EQUAL_TEST) {
    auto a = make_tuple();
    auto b = make_tuple();
    BOOST_CHECK(a == b);

    auto c = make_tuple(1, 2, std::string("Hel"));
    auto d = make_tuple(1, 2, std::string("Hel"));
    auto e = d; // 直接使用赋值进行复制, 是否可行?
    BOOST_CHECK(c == d);
    BOOST_CHECK(d == e);
    get<1>(e) = 10;
    BOOST_CHECK(d != e);

    int i = 1;
    int j = 10;
    std::string k = "Hel";

    Tuple<int&, int&, std::string> f(i, j, k);
    BOOST_CHECK(e == f);
}