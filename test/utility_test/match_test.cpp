//
// Created by Shiroan on 2022/10/6.
//
#include <utility/utility.h>
#include <ops/ops.h>

#define BOOST_TEST_MODULE Tuple Test
#include <boost/test/unit_test.hpp>

using namespace mstl;
using namespace mstl::utility;
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_CASE(BASIC_TEST) {
    constexpr i32 x = 10;

    auto y = match(x)
            .when(1, [](){return 5;})
            .when(ops::Range(2, 10), [](){return 10;})
            .rest([](){return 20;})
            .finale();

    BOOST_TEST_CHECK(y == 20);

    auto z = match(x)
           | when(1, [](){return 5;})
           | when(ops::Range(2, 10), [](){return 10;})
           | rest([](){return 20;})
           | finale();
    BOOST_TEST_CHECK(z == 20);

}