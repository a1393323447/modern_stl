//
// Created by Shiroan on 2022/10/10.
//

#include <mstl.h>

using namespace mstl::utility;

int main() {
    constexpr auto tuple = make_tuple(1, 2.0, 3l);
    constexpr auto ls1 = extract_from(tuple);
    static_assert(ls1 == TypeList<int, double, long>);

    constexpr auto ls2 = TypeList<int, double>;
    constexpr auto t1 = ls2 | apply_to<Tuple>();
    static_assert(t1 == TypeConstInstance<Tuple<int, double>>);

    constexpr auto ls3 = ls2 | append<char, float>();
    static_assert(ls3 == TypeList<int, double, char, float>);

    constexpr auto ls4 = ls3 | prepend<double*, const char*>();
    static_assert(ls4 == TypeList<double*, const char*, int, double, char, float>);

    static_assert(ls4.at<1>() == TypeConstInstance<const char*>);

    constexpr auto ls5 = ls4
                       | filter([]<typename T>(TypeConst<T>) {
                           return_v<(sizeof(T) < 8)>;
                       })
                       | map([]<typename T>(TypeConst<T>) {
                           return_t<T*>;
                       });
    static_assert(ls5 == TypeList<int*, char*, float*>);
}