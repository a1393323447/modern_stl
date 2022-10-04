//
// Created by 朕与将军解战袍 on 2022/9/19.
//

#include <iostream>
#include <global.h>

mstl::u8 hello_utf8[] = {
    0x00, 0XE4, 0xBD, 0XA0,
    0XE5, 0XA5, 0XBD
};

mstl::u8 hello_gbk[] = {

};

int main() {
    std::cout << (const char*)hello_utf8;

    return 0;
}