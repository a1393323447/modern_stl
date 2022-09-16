//
// Created by 朕与将军解战袍 on 2022/9/14.
//
#include <option/option.h>
#include <iostream>
#include <string>

#define BOOST_TEST_MODULE Tuple Test
#include <boost/test/unit_test.hpp>

using namespace mstl;

BOOST_AUTO_TEST_CASE(test_ref){
    std::string str = "123";
    Option<std::string&> op = Option<std::string&>::some(str);
    std::string& ref = op.unwrap_uncheck();

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