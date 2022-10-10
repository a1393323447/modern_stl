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
    constexpr i32 i = 10;

    constexpr auto x = match(i)
            .when(10, []{return 10;})
            .rest([]{return 20;})
            .finale();

    BOOST_TEST_CHECK(x == 10);

    constexpr auto y = match(i)
                     .when(1, [](){return 5;})
                     .when(ops::Range(2, 10), [](){return 10;})
                     .rest([](){return 20;})
                     .finale();

    BOOST_TEST_CHECK(y == 20);

    constexpr auto z = match(i)
                     | when(1, [](){return 5;})
                     | when(ops::Range(2, 11), [](){return 10;})
                     | rest([](){return 20;})
                     | finale();
    BOOST_TEST_CHECK(z == 10);

}