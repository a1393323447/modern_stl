#include <iostream>
#include <string>
#include <vector>
#include <memory>

struct A{
    std::string x = "Hello";
    A() {
        std::cout << "Default" << std::endl;
    }

    A(const std::string& s) {
        x = s;
        std::cout << "Non-Default" << std::endl;
    }

    A(const A& r) {
        x = r.x;
        std::cout << "Copy" << std::endl;
    }

    A(A&& r)  noexcept {
        x = std::move(r.x);
        std::cout << "Move" << std::endl;
    }

    ~A() {
        std::cout << "Deleted" << std::endl;
    }

    A& operator=(const A& r) {
        x = r.x;
        std::cout << "Copy =" << std::endl;
        return *this;
    }

    A& operator=(A&& r)  noexcept {
        x = std::move(r.x);
        std::cout << "Move =" << std::endl;
        return *this;
    }
};
int main() {
    std::allocator<A> allocator;

    auto arr = allocator.allocate(10);
    std::construct_at(arr, "He");
    std::cout << arr[0].x << std::endl;
    std::destroy_at(arr);
    allocator.deallocate(arr, 10);

    int i = 10;
    int& j = i;


    return 0;
}
