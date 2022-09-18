# Modern STL
`Modern STL` (简称: `mstl`) 是一个基于 `concept` 构建的 `STL`。目前正在开发。

## 组成
`mstl` 包含:
- `iter`: 定义了迭代器相关的 `concept` 以及一些用于操作迭代器的函数
- `collection`: `mstl` 的容器库, 现有:
    - `Array<T, N>`: 固定大小的数组
    -  `Vector<T, A>`: 可变长的随机访问容器
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

## Benchmark
`mstl` 提供了基于迭代器的零成本抽象:

### 使用迭代器
```c++
usize BM_with_iter(Array<std::string, 1000>& arr) {
    usize total_len = 0;
    combine(arr.iter(),
        Filter{}, [](const auto& str) {
            usize cnt = 0;
            for (auto c: str) {
                if (c >= 'A' + 13) {
                    cnt++;
                }
            }
            return cnt >= 10;
        },
        Map{}, [](const auto& str) {
            return str.size();
        },
        ForEach{}, [&](usize size) {
            total_len += size;
        }
    );

    return total_len;
}
```

### 使用下标
```c++
usize BM_with_index(Array<std::string, 1000>& arr) {
    usize total_len = 0;

    for(usize i = 0; i < arr.size(); i++) {
        usize cnt = 0;
        for (const auto& c: arr[i]) {
            if (c == 'A' + 13) {
                cnt++;
            }
        }

        if (cnt >= 10) {
            total_len += arr[i].size();
        }
    }

    return total_len;
}
```

### 测试结果
```text
Start To Gen Test Data
Gen All Test Data
2022-09-18T08:26:47-00:00
Running F:\C++\modern-stl\cmake-build-release\test\try_benchmark.exe
Run on (12 X 2600.14 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
BM_with_iter/real_time      116540 ns       115665 ns         6079
BM_with_index/real_time     113037 ns       115203 ns         6239
```
