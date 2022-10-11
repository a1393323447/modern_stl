//
// Created by Shiroan on 2022/9/20.
//
#include <iostream>
#include <mstl.h>

#define BOOST_TEST_MODULE Vector Test

#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

using namespace mstl;
using namespace result;


struct TestClass{
    std::string x = "Hello";
    std::string mode;
    TestClass() {
        mode = "Default";
    }

    TestClass(const std::string& s) {
        x = s;
        mode = "Non-Default";
    }

    TestClass(const TestClass& r) {
        x = r.x;
        mode = "Copy";
    }

    TestClass(TestClass&& r)  noexcept {
        x = std::move(r.x);
        mode = "Move";
    }

    ~TestClass() {
        mode = "Deleted";
    }

    TestClass& operator=(const TestClass& r) {
        x = r.x;
        mode = "Copy =";
        return *this;
    }

    TestClass& operator=(TestClass&& r)  noexcept {
        x = std::move(r.x);
        mode = "Move =";
        return *this;
    }
};

class MovableClass {
public:
    MovableClass() = default;
    MovableClass(const MovableClass&) = delete;
    MovableClass(MovableClass&& ) = default;

    MovableClass& operator=(const MovableClass&)=delete;
    MovableClass& operator=(MovableClass&&)=default;
};

class CopyableClass: public MovableClass {
public:
    using MovableClass::MovableClass;
    using MovableClass::operator=;

    CopyableClass(const CopyableClass&) {

    }
};

BOOST_AUTO_TEST_CASE(BASIC_TEST, * utf::tolerance(1e-6)) {
    Result<int, double> a(10), b(1.08);

    BOOST_CHECK(a.is_ok());
    BOOST_CHECK(b.is_err());
    BOOST_CHECK(a != b);

    BOOST_CHECK(a.ok_ref().unwrap_unchecked() == 10);
    BOOST_CHECK(a.ok_ref_unchecked() == 10);
    BOOST_CHECK(a.unwrap() == 10);
    BOOST_CHECK(a.err().is_none());

    BOOST_CHECK(b.err_ref().unwrap_unchecked() == 1.08);
    BOOST_CHECK(b.err().unwrap_unchecked() == 1.08);
    BOOST_CHECK(b.err_unchecked() == 1.08);

    Result<std::string, int> c("Hello"), d("Hello");
    BOOST_CHECK(c.is_ok());
    BOOST_CHECK(d.is_ok());
    BOOST_CHECK(c == d);

    auto e = Result<TestClass, int>::ok("foo"), f = Result<TestClass, int>::err(0);
    BOOST_CHECK(e.is_ok());
    BOOST_CHECK(f.is_err());
    BOOST_CHECK(e.ok_unchecked().x == "foo");
    BOOST_CHECK(f.err_unchecked() == 0);
}

BOOST_AUTO_TEST_CASE(REFERENCE_TEST) {
    std::string a = "foo";
    Result<std::string&, int> b(a);
    BOOST_REQUIRE(b.is_ok());
    auto&& c = b.unwrap();
    auto&& d = b.ok_ref().unwrap();
    c = "bar";

    BOOST_TEST_CHECK(a == "bar");
    BOOST_TEST_CHECK(d == "bar");
}

BOOST_AUTO_TEST_CASE(MOVEABLE_TEST) {
    MovableClass a;
    Result<MovableClass, int> b(std::move(a));
    b.ok_ref();
    static_assert(std::constructible_from<Result<MovableClass, int>, MovableClass&&>);

    Result<TestClass, int> d(TestClass{}), e(TestClass{});
    BOOST_TEST_CHECK(d.ok_ref_unchecked().mode == "Move");
    e = std::move(d);
    BOOST_TEST_CHECK(e.ok_ref_unchecked().mode == "Move =");
    Result<TestClass, int> f = std::move(e);
    BOOST_TEST_CHECK(f.unwrap().mode == "Move");
}

BOOST_AUTO_TEST_CASE(COPYABLE_TEST) {
    Result<TestClass, int> res1(TestClass{}), res2(TestClass{});
    res1 = res2;
    BOOST_TEST_CHECK(res1.ok_ref_unchecked().mode == "Copy =");
    Result<TestClass, int> res3 = res2;
    BOOST_TEST_CHECK(res3.ok_ref_unchecked().mode == "Copy");

    std::string str1 = "foo";
    std::string str2 = "bar";

    Result<std::string, int> a(str1), b(str2);
    auto c = a, d = b;
    BOOST_CHECK(a == c);
    BOOST_CHECK(b == d);

    c = d;
    BOOST_CHECK(c == d);

    Result<std::string&, int> e(str1);
    Result<std::string&, int> f = e;

    BOOST_CHECK(e == f);
    auto&& rstr1 = e.unwrap();
    auto&& rstr2 = f.unwrap();

    rstr1 = "bar";

    BOOST_TEST_CHECK(str1 == str2);
    BOOST_TEST_CHECK(rstr1 == str2);
    BOOST_TEST_CHECK(rstr1 == rstr2);
}

BOOST_AUTO_TEST_CASE(CONSTEXPR_TEST) {
    constexpr Result<int, double> a(10);
    static_assert(a.is_ok());
    static_assert(a.ok_ref_unchecked() == 10);

    constexpr Result<int, double> b(1.0);
    static_assert(b.is_err());
    static_assert(b.err_ref_unchecked() == 1.0);

    constexpr auto c = a, d = b;
    static_assert(c.is_ok());
    static_assert(c.ok_ref_unchecked() == 10);
    static_assert(c == a);

    static_assert(d.is_err());
    static_assert(d.err_ref_unchecked() == 1.0);
    static_assert(d == b);
}