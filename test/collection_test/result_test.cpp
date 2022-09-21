//
// Created by Shiroan on 2022/9/20.
//
#include <iostream>
#include <result/result.h>

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

    BOOST_CHECK(a.ok_ref().unwrap_uncheck() == 10);
    BOOST_CHECK(a.ok_ref_unchecked() == 10);
    BOOST_CHECK(a.unwrap() == 10);
    BOOST_CHECK(a.err().is_none());

    BOOST_CHECK(b.err_ref().unwrap_uncheck() == 1.08);
    BOOST_CHECK(b.err().unwrap_uncheck() == 1.08);
    BOOST_CHECK(b.err_unchecked() == 1.08);

    Result<std::string, int> c("Hello"), d("Hello");
    BOOST_CHECK(c.is_ok());
    BOOST_CHECK(d.is_ok());
    BOOST_CHECK(c == d);
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

    Result<TestClass, int> d(TestClass{});
    BOOST_TEST_CHECK(d.ok_ref_unchecked().mode == "Move");
    Result<TestClass, int> e = std::move(d);
    BOOST_TEST_CHECK(e.unwrap().mode == "Move");
}

BOOST_AUTO_TEST_CASE(COPYABLE_TEST) {
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