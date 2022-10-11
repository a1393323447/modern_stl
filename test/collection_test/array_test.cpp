//
// Created by 朕与将军解战袍 on 2022/9/13.
//

#include <mstl.h>
#include <iostream>
#include <string>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

template<typename T>
struct Pow {
    T pow;
};

void test_into_iter() {
    Array<i32, 10> array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto it = array.into_iter();
    auto next = it.next();

    while (next.is_some()) {
        std::cout << next.unwrap() << " ";
        next = it.next();
    }
    std::cout << std::endl;
}

void test_combine() {
    Array<i32, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    auto pow_arr = combine(arr.into_iter(),
        Map{}, [](auto ele) {
            return ele * ele;
        },
        Map{}, [](auto ele) {
            return Pow<usize>{ static_cast<usize>(ele * ele) };
        },
        Filter{}, [](auto& ele) {
            return ele.pow % 2 == 0;
        },
        CollectAs<Array<Pow<usize> , 5>>{}
    );

    auto iter = pow_arr.into_iter();
    auto next = iter.next();
    while (next.is_some()) {
        std::cout << next.unwrap().pow << " ";
        next = iter.next();
    }
    std::cout << std::endl;

    i32 limit = 50;
    auto first = combine(arr.into_iter(),
        Map{}, [](auto ele) {
            return ele * ele;
        },
        FindFirst{}, [&](auto ele) {
            return ele > limit;
        }
    );

    if (first.is_some()) {
        std::cout << "find " << first.unwrap() << '\n';
    } else {
        std::cout << "Can not find \n";
    }

    combine(arr.into_iter(),
        Filter{}, [](auto ele) {
            return ele % 2 == 1;
        },
        ForEach{}, [](auto ele) {
            std::cout << ele << " ";
        }
    );
    std::cout << '\n';
}

void test_combine_string() {
    using namespace std;
    Array<string, 10> arr = {
            "First", "Second", "Third", "Fourth", "Fifth",
            "Sixth", "Seventh", "Eighth", "Ninth", "Tenth"
    };

    combine(arr.iter(),
        Filter<Likely>{}, [](auto&& str) {
            return str.size() >= 3;
        },
        Map{}, [](auto&& str) {
            return str + "?";
        },
        ForEach{}, [](const auto& str) {
            std::cout << str << '\n';
        }
    );

    using namespace std;
    Array<string, 10> arr_own = {
            "First", "Second", "Third", "Fourth", "Fifth",
            "Sixth", "Seventh", "Eighth", "Ninth", "Tenth"
    };

    auto iter = arr_own.into_iter();
    combine(iter,
        Filter<Likely>{}, [](const auto& str) {
            std::cout << "In Filter: " << str << '\n';
            return str.size() >= 3;
        },
        Map{}, [](auto&& str) {
            std::cout << "In Map: " << str << '\n';
            return str + "?";
        },
        ForEach{}, [](auto&& str) {
            std::cout << "In ForEach: " << str << '\n';
        }
    );
}

void test_array_ref() {
    using namespace std;
    Array<string, 10> arr = {
            "First", "Second", "Third", "Fourth", "Fifth",
            "Sixth", "Seventh", "Eighth", "Ninth", "Tenth"
    };

    auto usize_arr = combine(arr.into_iter(),
        Map{}, [](auto&& str) {
            return str.size();
        },
        CollectAs<Array<usize, 10>>{}
    );

    combine(usize_arr.iter(),
        ForEach{}, [](auto&& size) {
            std::cout << size << " ";
        }
    );

    std::cout << '\n';
}

template<typename T>
struct Test {
    template<typename U>
    requires basic::Movable<U> &&
             std::same_as<T, U>
    Test(U&& value): value(std::forward<U>(value)) {}

    template<typename U>
    requires basic::Movable<U> &&
             std::same_as<T, U>
    Test(Test<U>&& other) {
        value = std::move(other.value);
    }

    template<typename U>
    requires basic::Movable<U> &&
             std::same_as<T, U>
    Test<T>& operator=(Test<U>&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        value = std::move(other.value);
    }

    template<typename U>
    requires basic::CopyAble<U> &&
             std::same_as<T, U>
    Test(const Test<U>& other) {
        value = other.value;
    }

    template<typename U>
    requires basic::CopyAble<U> &&
             std::same_as<T, U>
    Test<T>& operator=(const Test<U>& other) noexcept {
        if (this == &other) {
            return *this;
        }

        value = other.value;
    }

    T value;
};

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

class Copyable {
public:
    Copyable() = default;
    Copyable(int n): num(n) { }
    Copyable(const Copyable&) = default;
    Copyable& operator=(const Copyable&) = default;

    Copyable(Copyable&& m) noexcept {
        if (this == &m) return;
        num = m.num;
    }

    Copyable& operator=(Copyable&& m) noexcept {
        if (this == &m) return *this;
        num = m.num;

        return *this;
    }


private:
    int num;
};

int main() {
    test_into_iter();
    test_combine();
    test_combine_string();
    test_array_ref();

    Test<Movable> t = { Movable{1} };
    Test<Movable> d = { Movable{2} };
//    t = d;
    t = std::move(d);

    Test<Copyable> c = { Copyable{1} };
    Test<Copyable> b = { Copyable{2} };

    c = b;
    c = std::move(b);

    return 0;
}