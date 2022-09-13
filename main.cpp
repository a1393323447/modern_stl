#include <iostream>
#include "iter/iterator.h"
#include "option/option.h"

struct Iter {
    using Item = int;
    Option<int> next();
};

int main() {
    static_assert(Iterator<Iter>);
    return 0;
}
