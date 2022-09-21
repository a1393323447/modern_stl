#include <iostream>
#include <string>
#include <vector>
#include <result/result.h>
using mstl::result::Result;

int main() {
    Result<int, double> a(1);
    a.unwrap_err();
}
