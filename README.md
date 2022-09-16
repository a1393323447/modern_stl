# Modern STL
`Modern STL` (简称: `mstl`) 是一个基于 `concept` 构建的 `STL`。目前正在开发。

## 组成
`mstl` 包含:
- `iter`: 定义了迭代器相关的 `concept` 以及一些用于操作迭代器的函数
- `collection`: `mstl` 的容器库, 现有:
    - `Array<T, N>`: 固定大小的数组
- `utility`: 通用库, 现有:
  - `Tuple`: 可包含任意数量异构类型的容器
- `Option<T>`: 用于表示有或无的类型
- `ops`: 用于定义一些列操作的 `concept`

## 示例
`mstl::collection::Array<T>` 及迭代器的使用:
```c++
#include <iostream>
#include <iter/iterator.h>
#include <collection/array.h>

using namespace mstl;
using namespace mstl::iter;
using namespace mstl::collection;

template<typename T>
struct Pow {
    T pow;
};

int main() {
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
    
    combine(pow_arr.iter(),
        ForEach{}, [](const auto& ele) {
            std::cout << ele.pow << " ";    
        }
    );
    return 0;
}
```