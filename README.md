# Modern STL
`Modern STL` (简称: `mstl`) 是一个基于 `concept` 构建的 C++模板库，致力于以更现代化的方法实现STL提供的功能。目前正在开发。

## 组成
`mstl` 包含:
- `iter`: 定义了迭代器相关的 `concept` 以及一些用于操作迭代器的函数
- `collection`: `mstl` 的容器库, 现有:
    - `Array<T, N>`: 固定大小的数组
    - `Vector<T, A>`: 可变长的随机访问容器
    - `List<T, A>`: 双向链表容器
    - `ForwardList<T, A>`: 单向链表容器
- `memory`: `mstl`的内存管理库, 现有:
  - `Layout`: 描述一种类型的大小和对齐信息的对象.
  - `Allocator`: 运行时动态分配内存的设施.
- `utility`: 通用库, 现有:
  - `Tuple`: 可包含任意数量异构类型的容器
- `Option<T>`: 用于表示有或无的类型
- `ops`: 用于定义一些列操作的 `concept`
- `result`: 用于表示可能的错误信息
  - `Result<T, E>`: 表示一个可能失败的函数的返回值. 可能包含成功运行的结果(`T`), 或失败的错误信息('E').

### 命名空间
1. mstl
   1. basic
   2. collection
      1. concepts
   3. iter
      1. terminal
      2. adapter
      3. concepts
   4. memory
      1. concepts
      2. allocator
   5. ops
   6. result
   7. str
   8. utility
   9. Option
   10. Slice

## 目标🎯
- [x] 实现自己的*Allocator*.
- [x] 实现可常量求值的Result.
- [ ] 实现标准库容器.
  - [x] LinkedList
  - [x] Vector
  - [x] Array
  - [ ] Deque
  - [ ] Map
  - [ ] Set
- [x] 重构Tuple为可常量求值.
- [ ] 实现带编码的字符串.
- [ ] 文档

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

### 迭代器
`mstl` 提供了基于迭代器的零成本抽象:

#### 使用迭代器
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

#### 使用下标
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

#### 测试结果
```text
Running F:\C++\modern-stl\cmake-build-release\test\try_benchmark.exe
Run on (12 X 2603.18 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
BM_with_iter/real_time      108622 ns       106854 ns         6434
BM_with_index/real_time     107107 ns       106453 ns         6605
```

### Vector
`mstl`实现了与C++标准库的`std::vector`相类似的容器, 位于`mstl::collections::Vector`(下称`Vector`).

经测试, `Vector`的性能与标准库的`vector`较为接近, 部分操作甚至优于后者.

`Vector`在各版本的构造函数测试中, 对同规模测试所消耗的时间与`std::vector`均相差不到7%, 认为可忽略不计.
`resize`, `push_back`, `erase`等操作所消耗的时间与`std::vector`相差不到2%.
`insert`操作则比`std::vector`快约32%, 详见测试结果.

#### 测试方法
参与测试的类分别为实验组`Vector<std::string>`和对照组`std::vector<std::string>`.
基准程序代码见`test/collection_test/vector_benchmark.cpp`.
对基准程序, 我们一共运行50次, 取每次运行的各测试用例的单次迭代的CPU时间的平均值作为最终各测试用例的消耗时间.

#### 测试结果
```
Run on (16 X 2922.63 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 512 KiB (x8)
  L3 Unified 4096 KiB (x2)
----------------------------------------------  ---------
name                                            cpu_time (ns)
BM_default_construct<std::vector<std::string>>  11859722
BM_default_construct<Vector<std::string>>       11980360
BM_range_based<std::vector<std::string>>        90535732
BM_range_based<Vector<std::string>>             87606666
BM_ilist<std::vector<std::string>>              45165
BM_ilist<Vector<std::string>>                   42234
BM_push_back<std::vector<std::string>>          98303518
BM_push_back<Vector<std::string>>               100647102
BM_insert<std::vector<std::string>>             87467778
BM_insert<Vector<std::string>>                  59784404
BM_resize<std::vector<std::string>>             11846370
BM_resize<Vector<std::string>>                  11703414
BM_reserve<std::vector<std::string>>            7228
BM_reserve<Vector<std::string>>                 7359
BM_erase<std::vector<std::string>>              103869080
BM_erase<Vector<std::string>>                   105997260
----------------------------------------------  ---------
-----------------  -----
name                rate
default_construct  -1.0%
range_based         3.2%
ilist               6.5%
push_back          -2.4%
insert             31.6%
resize              1.2%
reserve            -1.8%
erase              -2.0%
-----------------  -----
```
### List
`MSTL`实现了与C++标准库的`std::list`与`std::forward_list`相类似的容器, 位于`mstl::collections::BaseList`.

与`STL`不同, 在`MSTL`中, 单向链表和双向链表共用同一个类模板, 并借由concept机制实现编译时多态.
`BaseList`表现为单向链表还是双向链表, 取决于其节点类型.
下面称单向链表为`ForwardList`, 称双向链表为`List`.

经测试, `List`和`ForwardList`的效率都与`STL`中的对应容器较为接近. 
对`List`而言, 基于范围的构造与基于初始化列表的构造分别比`std::list`快9.1%和13.8%.
`insert`和`resize`操作分别快9.1%和13.3%.
对`ForwardList`而言, `sort`和`merge`操作分别比`std::forward_list`快25.0%和11.1%.
详见测试结果.

#### 测试方法
参与测试的类分别为实验组`List<std::string>`, `ForwardList<std::string>`和对照组`std::list<std::string>`, `std::forward_list<std::string>`.
基准程序代码见`test/collection_test/list_benchmark.cpp`.
对基准程序, 我们一共运行50次, 取每次运行的各测试用例的单次迭代的CPU时间的平均值作为最终各测试用例的消耗时间.

#### 测试结果
```
---------------------------------------------------  -------------
name                                                 cpu_time (ms)
BM_assign_construct<std::list<std::string>>                    254
BM_assign_construct<List<std::string>>                         235
BM_assign_construct<std::forward_list<std::string>>            238
BM_assign_construct<ForwardList<std::string>>                  235
BM_range_based<std::list<std::string>>                         398
BM_range_based<List<std::string>>                              362
BM_range_based<std::forward_list<std::string>>                 363
BM_range_based<ForwardList<std::string>>                       362
BM_ilist<std::list<std::string>>                                 2
BM_ilist<List<std::string>>                                      2
BM_ilist<std::forward_list<std::string>>                         2
BM_ilist<ForwardList<std::string>>                               1
BM_push_back<std::list<std::string>>                           378
BM_push_back<List<std::string>>                                364
BM_push_front<std::forward_list<std::string>>                  357
BM_push_front<ForwardList<std::string>>                        364
BM_insert<std::list<std::string>>                              383
BM_insert<List<std::string>>                                   348
BM_insert_after<std::forward_list<std::string>>                364
BM_insert_after<ForwardList<std::string>>                      350
BM_resize<std::list<std::string>>                              251
BM_resize<List<std::string>>                                   217
BM_resize<std::forward_list<std::string>>                      234
BM_resize<ForwardList<std::string>>                            217
BM_erase<std::list<std::string>>                               183
BM_erase<List<std::string>>                                    183
BM_erase_after<std::forward_list<std::string>>                 180
BM_erase_after<ForwardList<std::string>>                       188
BM_sort<std::list<std::string>>                               2090
BM_sort<List<std::string>>                                    2011
BM_sort<std::forward_list<std::string>>                       2675
BM_sort<ForwardList<std::string>>                             2005
BM_merge<std::list<std::string>>                               638
BM_merge<List<std::string>>                                    605
BM_merge<std::forward_list<std::string>>                       675
BM_merge<ForwardList<std::string>>                             600
BM_splice<std::list<std::string>>                              181
BM_splice<List<std::string>>                                   182
BM_splice_after<std::forward_list<std::string>>                191
BM_splice_after<ForwardList<std::string>>                      198
BM_reverse<std::list<std::string>>                             209
BM_reverse<List<std::string>>                                  215
BM_reverse<std::forward_list<std::string>>                     202
BM_reverse<ForwardList<std::string>>                           215
BM_unique<std::list<std::string>>                              231
BM_unique<List<std::string>>                                   217
BM_unique<std::forward_list<std::string>>                      230
BM_unique<ForwardList<std::string>>                            223
---------------------------------------------------  -------------
------------------------------------------           -----
name                                                 rate
assign_construct<List<std::string>>                   7.6%
assign_construct<ForwardList<std::string>>            1.3%
range_based<List<std::string>>                        9.1%
range_based<ForwardList<std::string>>                 0.2%
ilist<List<std::string>>                             13.8%
ilist<ForwardList<std::string>>                       4.4%
push_back<List<std::string>>                          3.7%
push_front<ForwardList<std::string>>                 -1.9%
insert<List<std::string>>                             9.1%
insert_after<ForwardList<std::string>>                3.7%
resize<List<std::string>>                            13.3%
resize<ForwardList<std::string>>                      7.4%
erase<List<std::string>>                             -0.1%
erase_after<ForwardList<std::string>>                -4.4%
sort<List<std::string>>                               3.8%
sort<ForwardList<std::string>>                       25.0%
merge<List<std::string>>                              5.1%
merge<ForwardList<std::string>>                      11.1%
splice<List<std::string>>                            -0.7%
splice_after<ForwardList<std::string>>               -3.9%
reverse<List<std::string>>                           -3.1%
reverse<ForwardList<std::string>>                    -6.5%
unique<List<std::string>>                             6.0%
unique<ForwardList<std::string>>                      3.1%
------------------------------------------           -----
```