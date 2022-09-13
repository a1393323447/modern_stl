#include <iostream>
#include <utility/tuple.h>

int main() {
    auto a = mstl::utility::make_tuple();
    auto b = mstl::utility::make_tuple(1, 2);
    static_assert(a.size() == 0);

    int c, d;
    auto x = mstl::utility::tie(c, d);
    x = b;

    std::cout << c << std::endl << d << std::endl;
    return 0;
}
