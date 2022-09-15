//
// Created by 朕与将军解战袍 on 2022/9/14.
//

#include <option/option.h>
#include <iostream>
#include <string>

using namespace mstl;

void test_ref() {
    std::string str = "123";
    Option<std::string&> op = Option<std::string&>::some(str);
    std::string& ref = op.unwrap_uncheck();
    std::cout << ref << std::endl;
}

int main() {
    test_ref();

    return 0;
}