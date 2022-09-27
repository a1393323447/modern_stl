//
// Created by Shiroan on 2022/9/22.
//

#include <collection/linked_list.h>
#include <collection/vector.h>
#include <iter/iterator.h>
#include <string>
#include "../TrackingAllocator.h"
#include <utility/utility.h>
#define BOOST_TEST_MODULE Vector Test
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#define STRS {"foo", "bar", "hello", "world"}

using namespace mstl;
using utility::to_string;
template <typename T>
using ForwardList = collection::ForwardList<T, TrackingAllocator<>>;

template <typename T>
using List = collection::List<T, TrackingAllocator<>>;

static collection::Vector<std::string> vec = STRS;

BOOST_AUTO_TEST_CASE(CONSTRUCT_ASSIGN_TEST) {
    List<std::string> ls1 = {"a", "b", "c"};
    BOOST_TEST_CHECK(ls1.size() == 3);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "a");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "c");
    BOOST_TEST_CHECK(to_string(ls1) == "List [a, b, c]");

    List<std::string> ls2;
    BOOST_TEST_CHECK(ls2.empty());
    BOOST_TEST_CHECK(!ls2.moved());
    BOOST_TEST_CHECK(ls2.front().is_none());
    BOOST_TEST_CHECK(ls2.back().is_none());

    List<std::string> ls3c1 = ls1;
    BOOST_TEST_CHECK(ls3c1.size() == 3);
    BOOST_TEST_CHECK(ls3c1.front().unwrap() == "a");
    BOOST_TEST_CHECK(ls3c1.back().unwrap() == "c");
    BOOST_TEST_CHECK(to_string(ls3c1) == "List [a, b, c]");

    List<std::string> ls4m1 = std::move(ls1);
    BOOST_TEST_CHECK(ls1.moved());  // ls1 has been moved, and should not be used in normal code
    BOOST_TEST_CHECK(ls4m1.size() == 3);
    BOOST_TEST_CHECK(ls4m1.front().unwrap() == "a");
    BOOST_TEST_CHECK(ls4m1.back().unwrap() == "c");
    BOOST_TEST_CHECK(to_string(ls4m1) == "List [a, b, c]");

    List<std::string> ls5(2, "foo");
    BOOST_TEST_CHECK(ls5.size() == 2);
    BOOST_TEST_CHECK(ls5.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls5.back().unwrap() == "foo");
    BOOST_TEST_CHECK(to_string(ls5) == "List [foo, foo]");

    List<std::string> ls6(2);
    BOOST_TEST_CHECK(ls6.size() == 2);
    BOOST_TEST_CHECK(ls6.front().unwrap() == "");
    BOOST_TEST_CHECK(ls6.back().unwrap() == "");
    BOOST_TEST_CHECK(to_string(ls6) == "List [, ]");

    List<std::string> ls7(vec.begin(), vec.end());
    BOOST_TEST_CHECK(ls7.size() == 4);
    BOOST_TEST_CHECK(ls7.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls7.back().unwrap() == "world");
    BOOST_TEST_CHECK(to_string(ls7) == "List [foo, bar, hello, world]");

    List<std::string> ls8c5;
    ls8c5 = ls5;
    BOOST_TEST_CHECK(ls8c5.size() == 2);
    BOOST_TEST_CHECK(ls8c5.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls8c5.back().unwrap() == "foo");
    BOOST_TEST_CHECK(to_string(ls8c5) == "List [foo, foo]");

    List<std::string> ls8m5;
    ls8m5 = std::move(ls5);
    BOOST_TEST_CHECK(ls8m5.size() == 2);
    BOOST_TEST_CHECK(ls8m5.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls8m5.back().unwrap() == "foo");
    BOOST_TEST_CHECK(to_string(ls8m5) == "List [foo, foo]");

    List<std::string> ls9;
    ls9 = {"foo", "bar"};
    BOOST_TEST_CHECK(ls9.size() == 2);
    BOOST_TEST_CHECK(ls9.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls9.back().unwrap() == "bar");
    BOOST_TEST_CHECK(to_string(ls9) == "List [foo, bar]");
}

BOOST_AUTO_TEST_CASE(ITER_TEST) {
    List<std::string> ls1 = STRS;
    auto vec1cls1 = iter::collect<collection::Vector<std::string>>(ls1.iter());
    BOOST_TEST_CHECK(to_string(vec1cls1) == "Vec [foo, bar, hello, world]");

    std::cout << "ITERATION_TEST [";  // Test for range-based for statement
    for (const auto &i: ls1) {
        std::cout << i << ", ";
    }
    std::cout << "]" << std::endl;

    static_assert(iter::Iterator<decltype(ls1.iter())>);
    using iter::operator|;
    std::string less_than_4_char = ls1.iter() |
            iter::filter([](const auto& s){ return s.size() < 4; }) |
            iter::fold(std::string(""), [](const std::string& init, const auto& s) { return init + s; });
    BOOST_TEST_CHECK(less_than_4_char == "foobar");

    auto lo = ls1.before_begin();
    lo++;
    BOOST_TEST_CHECK(*lo == "foo");
    lo++;
    BOOST_TEST_CHECK(*lo == "bar");
    lo = std::next(lo, 3);
    BOOST_CHECK(lo == ls1.end());

    auto r = ls1.rbegin();
    BOOST_TEST_CHECK(*r == "world");
}

BOOST_AUTO_TEST_CASE(EDITOR_TEST) {
    List<std::string> ls1 = STRS;
    ls1.clear();
    BOOST_TEST_CHECK(ls1.empty());
    BOOST_TEST_CHECK(ls1.front().is_none());
    BOOST_TEST_CHECK(ls1.back().is_none());

    ls1.push_front("foo");
    BOOST_TEST_CHECK(ls1.size() == 1);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "foo");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo]");

    ls1.push_back("bar");
    BOOST_TEST_CHECK(ls1.size() == 2);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "bar");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, bar]");

    auto iter1 = ls1.insert_after(ls1.before_begin(), "foo");  // iter2 points to the 1st element.
    BOOST_TEST_CHECK(ls1.size() == 3);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "bar");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, foo, bar]");
    BOOST_TEST_CHECK(*iter1 == "foo");

    auto iter2 = ls1.insert_after(iter1, 2, "bar");  // iter2 points to the 2nd element.
    BOOST_TEST_CHECK(ls1.size() == 5);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "bar");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, bar, bar, foo, bar]");
    BOOST_TEST_CHECK(*iter2 == "bar");
    iter2--;
    BOOST_CHECK(iter1 == iter2);

    auto iter3 = ls1.insert_after(iter2, vec.begin(), vec.begin() + 2);
    BOOST_TEST_CHECK(ls1.size() == 7);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "bar");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, foo, bar, bar, bar, foo, bar]");
    BOOST_TEST_CHECK(*iter3 == "foo");
    iter3--;
    BOOST_CHECK(iter3 == iter2);

    ls1.clear();
    auto iter4 = ls1.insert_after(ls1.before_begin(), STRS);
    BOOST_TEST_CHECK(ls1.size() == 4);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "world");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, bar, hello, world]");
    BOOST_TEST_CHECK(*iter4 == "foo");

    auto iter5 = ls1.erase_after(ls1.begin());
    BOOST_TEST_CHECK(ls1.size() == 3);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "world");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, hello, world]");
    BOOST_TEST_CHECK(*iter5 == "hello");

    auto iter6 = ls1.erase_after(ls1.begin(), std::next(ls1.begin(), 2));
    BOOST_TEST_CHECK(ls1.size() == 2);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "world");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, world]");
    BOOST_TEST_CHECK(*iter6 == "world");

    auto iter7 = ls1.insert(std::next(ls1.begin(), 1), "bar");
    BOOST_TEST_CHECK(ls1.size() == 3);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "world");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, bar, world]");
    BOOST_TEST_CHECK(*iter7 == "bar");

    iter7 = ls1.insert(ls1.begin(), 2, "foo");
    BOOST_TEST_CHECK(ls1.size() == 5);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "world");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, foo, foo, bar, world]");
    BOOST_TEST_CHECK(*iter7 == "foo");

    iter7 = ls1.insert(ls1.begin(), vec.begin(), vec.begin() + 2);
    BOOST_TEST_CHECK(ls1.size() == 7);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "world");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, bar, foo, foo, foo, bar, world]");
    BOOST_TEST_CHECK(*iter7 == "foo");

    iter7 = ls1.insert(ls1.begin(), {"a", "b"});
    BOOST_TEST_CHECK(ls1.size() == 9);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "a");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "world");
    BOOST_TEST_CHECK(to_string(ls1) == "List [a, b, foo, bar, foo, foo, foo, bar, world]");
    BOOST_TEST_CHECK(*iter7 == "a");

    ls1.pop_back();
    ls1.pop_front();
    BOOST_TEST_CHECK(ls1.size() == 7);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "b");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "bar");
    BOOST_TEST_CHECK(to_string(ls1) == "List [b, foo, bar, foo, foo, foo, bar]");

    iter7 = ls1.erase(ls1.begin());
    BOOST_TEST_CHECK(ls1.size() == 6);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "foo");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "bar");
    BOOST_TEST_CHECK(to_string(ls1) == "List [foo, bar, foo, foo, foo, bar]");
    BOOST_TEST_CHECK(*iter7 == "foo");

    iter7 = ls1.erase(ls1.begin(), std::next(ls1.begin(), 5));
    BOOST_TEST_CHECK(ls1.size() == 1);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "bar");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "bar");
    BOOST_TEST_CHECK(to_string(ls1) == "List [bar]");
    BOOST_TEST_CHECK(*iter7 == "bar");

    ls1.resize(5);
    BOOST_TEST_CHECK(ls1.size() == 5);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "bar");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "");
    BOOST_TEST_CHECK(to_string(ls1) == "List [bar, , , , ]");

    ls1.resize(1);
    BOOST_TEST_CHECK(ls1.size() == 1);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "bar");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "bar");
    BOOST_TEST_CHECK(to_string(ls1) == "List [bar]");

    ls1.resize(3, "foo");
    BOOST_TEST_CHECK(ls1.size() == 3);
    BOOST_TEST_CHECK(ls1.front().unwrap() == "bar");
    BOOST_TEST_CHECK(ls1.back().unwrap() == "foo");
    BOOST_TEST_CHECK(to_string(ls1) == "List [bar, foo, foo]");

    List<std::string> ls2 = {"a"};
    ls1.swap(ls2);
    BOOST_TEST_CHECK(to_string(ls1) == "List [a]");
    BOOST_TEST_CHECK(to_string(ls2) == "List [bar, foo, foo]");
}

BOOST_AUTO_TEST_CASE(OPERATIONS_TEST) {
//    List<std::string> ls1 = {"a", "c", "e"};
//    List<std::string> ls2 = {"b", "d", "f"};

//    ls1.merge(ls2);
//    BOOST_TEST_CHECK(to_string(ls1) == "List [a, b, c, d, e, f]");
//    BOOST_TEST_CHECK(ls2.empty());

    List<std::string> ls1 = {"a", "b", "c", "d", "e", "f"};


    List<std::string> ls3 = STRS;
    ls3.remove_if([](const auto& a) { return a.size() < 4; });
    ls3.remove("world");
    BOOST_TEST_CHECK(to_string(ls3) == "List [hello]");

    ls1.reverse();
    BOOST_TEST_CHECK(to_string(ls1) == "List [f, e, d, c, b, a]");

    ForwardList<std::string> fls1 = {"a", "b", "c"};
    fls1.reverse();
    BOOST_TEST_CHECK(to_string(fls1) == "ForwardList [c, b, a]");

//    ls1.sort();
//    BOOST_TEST_CHECK(to_string(ls1) == "List [a, b, c, d, e, f]");

    List<std::string> ls4 = {"a", "a", "b", "b"};
    ls4.unique();
    BOOST_TEST_CHECK(to_string(ls4) == "List [a, b]");
}

BOOST_AUTO_TEST_CASE(MEMORY_TRACK) {
    std::cout << std::endl;
    std::cout << "================= MEMORY TRACK =================" << std::endl;
    TrackingAllocator<>::dump();
    std::cout << "================= MEMORY TRACK =================" << std::endl;

    BOOST_REQUIRE(TrackingAllocator<>::get_beholding_memory() == 0);
}