//
// Created by 朕与将军解战袍 on 2022/9/14.
//
#include <option/option.h>
#include <string>

#define BOOST_TEST_MODULE Option Test
#include <boost/test/unit_test.hpp>

using namespace mstl;

class Movable {
public:
    Movable() = default;
    Movable(int n): num(n) { }
    Movable(const Movable&) = delete;
    Movable& operator=(const Movable&) = delete;

    Movable(Movable&& m) noexcept {
        if (this == &m) return;
        num = m.num;
    }

    Movable& operator=(Movable&& m) noexcept {
        if (this == &m) return *this;
        num = m.num;

        return *this;
    }


private:
    int num;
};

template<typename U>
void test(U&& u) {
    static_assert(std::is_reference_v<U>, "This is not a reference");
}

BOOST_AUTO_TEST_CASE(test_ref){
    std::string str = "123";
    Option<std::string&> op = Option<std::string&>::some(str);
    std::string& ref = op.unwrap_uncheck();

    test(ref);

    auto o = Option<Movable>::some(1);

    BOOST_CHECK_EQUAL(ref, str);
}

BOOST_AUTO_TEST_CASE(test_bool) {
    bool f = false;
    bool t = true;

    Option<bool> some_false = Option<bool>::some(f);
    Option<bool> some_true = Option<bool>::some(t);
    Option<bool> none = Option<bool>::none();

    BOOST_CHECK(some_false.is_some());
    BOOST_CHECK(!some_false.is_none());
    BOOST_CHECK_EQUAL(some_false.unwrap(), false);

    BOOST_CHECK(some_true.is_some());
    BOOST_CHECK(!some_true.is_none());
    BOOST_CHECK_EQUAL(some_true.unwrap(), true);

    BOOST_CHECK(none.is_none());
    BOOST_CHECK(!none.is_some());

    Option<bool> copy = some_false;
    BOOST_CHECK(copy.is_some());
    BOOST_CHECK(!copy.is_none());
    BOOST_CHECK_EQUAL(copy.unwrap(), false);

    copy = some_true;
    BOOST_CHECK(some_true.is_some());
    BOOST_CHECK(!some_true.is_none());
    BOOST_CHECK_EQUAL(some_true.unwrap(), true);

    copy = none;
    BOOST_CHECK(!copy.is_some());
    BOOST_CHECK(copy.is_none());
}