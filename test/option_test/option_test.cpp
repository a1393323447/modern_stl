//
// Created by 朕与将军解战袍 on 2022/9/14.
//
#include <mstl/mstl.h>
#include <string>

#define BOOST_TEST_MODULE Option Test
#include <boost/test/unit_test.hpp>

using namespace mstl;

struct Movable {
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
    int num;
};

BOOST_AUTO_TEST_CASE(TEST_REF){
    static_assert(mstl::basic::Movable<Option<std::string&>>);
    static_assert(mstl::basic::CopyAble<Option<std::string&>>);

    std::string str = "foo";
    Option<std::string&> op = Option<std::string&>::some(str);
    std::string& ref = op.unwrap_unchecked();
    BOOST_CHECK_EQUAL(ref, str);
    str = "bar";
    BOOST_CHECK_EQUAL(ref, str);
    BOOST_CHECK(op.is_none());
}

/*
BOOST_AUTO_TEST_CASE(TEST_BOOL) {
    bool f = false;
    bool t = true;

    Option<bool> some_false = Option<bool>::some(f);
    Option<bool> some_true = Option<bool>::some(t);
    Option<bool> none = Option<bool>::none();

    BOOST_CHECK(some_false.is_some());
    BOOST_CHECK(!some_false.is_none());
    BOOST_CHECK_EQUAL(some_false.as_ref(), false);

    BOOST_CHECK(some_true.is_some());
    BOOST_CHECK(!some_true.is_none());
    BOOST_CHECK_EQUAL(some_true.as_ref(), true);

    BOOST_CHECK(none.is_none());
    BOOST_CHECK(!none.is_some());

    Option<bool> copy = some_false;
    BOOST_CHECK(copy.is_some());
    BOOST_CHECK(!copy.is_none());
    BOOST_CHECK_EQUAL(copy.as_ref(), false);

    copy = some_true;
    BOOST_CHECK(some_true.is_some());
    BOOST_CHECK(!some_true.is_none());
    BOOST_CHECK_EQUAL(some_true.as_ref(), true);

    copy = none;
    BOOST_CHECK(!copy.is_some());
    BOOST_CHECK(copy.is_none());
}
*/

BOOST_AUTO_TEST_CASE(TEST_MOVABLE) {
    static_assert(mstl::basic::Movable<Option<Movable>>);
    static_assert(!mstl::basic::CopyAble<Option<Movable>>);

    Movable movable{ 10 };
    auto op_1 = Option<Movable>::some(std::move(movable));

    auto op_2 = Option<Movable>::emplace(10);

    BOOST_CHECK(op_1.as_ref().num == op_2.as_ref().num);

    Option<Movable> op_1_move = std::move(op_1);
    BOOST_CHECK(op_1.is_none());
    BOOST_CHECK(op_1_move.as_ref().num == op_2.as_ref().num);

    movable = op_1_move.unwrap();
    BOOST_CHECK(movable.num == op_2.as_ref().num);
}

BOOST_AUTO_TEST_CASE(TEST_COPYABLE) {
    auto op_1 = Option<int>::some(20);
    auto op_2 = Option<int>::some(10);
    auto op_3 = Option<int>::none();

    op_2 = op_1;
    BOOST_TEST_CHECK(op_2.unwrap_unchecked() == 20);
    BOOST_CHECK(op_2.is_none());

    op_1 = op_2;
    BOOST_CHECK(op_1.is_none());

    op_3 = Option<int>::some(5);
    BOOST_REQUIRE(op_3.is_some());
    BOOST_TEST_CHECK(op_3.unwrap_unchecked() == 5);
}

struct DivT {
    i32 q, r;
};

constexpr Option<DivT> mstl_div(i32 a, i32 b) {
    if (b == 0) {
        return Option<DivT>::none();
    } else {
        return Option<DivT>::some({a / b, a % b});
    }
}

constexpr i32 get_q(Option<DivT> t) {
    return t.unwrap().q;
}

BOOST_AUTO_TEST_CASE(CONSTEXPR_TEST) {
    constexpr auto op_1 = Option<int>::some(20);
    constexpr auto op_2 = Option<int>::none();

    static_assert(op_1.is_some());
    static_assert(op_1.as_ref() == 20);

    static_assert(op_2.is_none());

    constexpr i32 q = get_q(mstl_div(10, 3));
    static_assert(q == 3);
}