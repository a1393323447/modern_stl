//
// Created by 朕与将军解战袍 on 2022/9/12.
//

#ifndef __MODERN_STL_OPTION_H__
#define __MODERN_STL_OPTION_H__

#include <variant>

struct {} None;

template<typename T>
class Option: std::variant<T, decltype(None)> {
public:
    bool is_some() const {
        return std::holds_alternative<T>(*this);
    }

    bool is_none() const { return !this->is_some(); }
};

#endif //__MODERN_STL_OPTION_H__
