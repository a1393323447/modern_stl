//
// Created by Shiroan on 2022/9/17.
//

#ifndef MODERN_STL_PRINT_H
#define MODERN_STL_PRINT_H

#include <basic_concepts.h>
#include <string>
#include <sstream>

template<mstl::basic::Printable P>
std::string to_string(const P& printable) {
    std::ostringstream tmp;
    tmp << printable;
    return tmp.str();
}

#endif //MODERN_STL_PRINT_H
