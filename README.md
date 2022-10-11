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
  - `Match`: 值匹配工具, 类似于`switch`.
  - `TypeList`: 类型运算工具.
- `Option<T>`: 用于表示有或无的类型
- `ops`: 用于定义一些列操作的 `concept`
- `result`: 用于表示可能的错误信息
  - `Result<T, E>`: 表示一个可能失败的函数的返回值. 可能包含成功运行的结果(`T`), 或失败的错误信息(`E`).

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
      1. encoding
      2. concepts
   8. utility
   9. Option
   10. Slice

## 开始使用
`mstl`是纯头文件库, 使用`CMake`进行管理进行构建. 
因此, 我们建议您在使用本库时, 也尝试使用`CMake`构建您的项目.

### 需求
| 包     | 版本         | 备注                      |
|-------|------------|-------------------------|
| CMake | \>=3.14    | 若您使用`CMake`构建本项目和您的其他项目 |
 | GCC   | \>= 12.2.0 | 暂不支持 `MSVC`             |

### 准备工作
Clone本仓库, 并Checkout `main` 分支.
```shell
git clone https://github.com/a1393323447/modern_stl.git 
cd modern_stl
git checkout main
```

### 构建与使用(通过`CMake`)
进入项目目录, 使用`CMake`构建并安装项目
```shell
# 假设您处于modern_stl目录下
md build
cd build
cmake ..  # 若您使用Windows操作系统, 还应该添加 -G "Unix Makefiles". MSTL还不支持MSVC
          # 若您需要指定安装路径, 还应该添加 -DCMAKE_INSTALL_PREFIX:PATH=...
cmake --build . --config Release --target install
```

然后您便可以在新的项目中使用`mstl`. 您需要在`CMakeLists.txt`文件中引入`mstl`.
```cmake
set(CMAKE_CXX_STANDARD 20)

# 若您安装的路径不在PATH环境变量内, 您可能还需要使用如下语句设置搜索路径
# list(APPEND CMAKE_PREFIX_PATH "...")  # 把 ... 替换为mstl的安装路径

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")  # 开启调试模式下的部分运行时检查功能

find_package(mstl REQUIRED)
add_executable(mstl_test src/main.cpp)
target_link_libraries(mstl_test mstl::mstl)
```

接着, 在您的项目文件中包含`mstl/mstl.h`
```c++
#include <mstl/mstl.h>
```

### 构建与使用(不通过`CMake`)
您可以直接把`include/mstl`目录复制到任意地方.
在您的项目文件中包含`mstl/mstl.h`
```c++
#include <mstl/mstl.h>
```
并在编译时指定`mstl`目录所在的目录.
```shell
g++ main.cpp -o main -I/path/to/the/directory
```
您也可以使用任何支持指定包含目录的构建工具.

## 关于文档
本项目的文档预计以仓库Wiki的形式提供. 由于工作量较大, 我们目前还没能开始撰写Wiji页面.

目前, 我们已经在头文件中撰写了部分类的文档, 您可以使用`Doxygen`构建它们.

您也可以参见`test`目录下的单元测试用例, 这些用例覆盖了库中组件的大多数用法.

## 目标🎯
- [x] 实现自己的*Allocator*.
- [x] 实现可常量求值的Result.
- [ ] 实现可常量求值的标准库容器.
  - [x] LinkedList
  - [x] Vector
  - [x] Array
  - [ ] Deque
  - [ ] Map
  - [ ] Set
- [x] 重构Tuple为可常量求值.
- [x] 实现带编码的字符串.
- [ ] Wiki页面.

## 示例
#### 容器类 (以`Array<T>`为例) 及迭代器
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

#### 字符串
`mstl` 提供了可扩展的字符 `BasicChar<Encoding>` 和 `BasicString<Encoding>`。
现已实现的编码为:
- `Ascii` : 对应 `AsciiChar` 和 `AsciiString`
- `UTF8` : 对应 `UTF8Char` 和 `UTF8String`

```cpp
#include <iter/iterator.h>
#include <str/string.h>
#include <iostream>

using namespace mstl::iter; // 迭代器相关功能
using namespace mstl::str; // 字符串相关功能
using namespace mstl::str::encoding; // 后缀运算符

UTF8Char utf8_ch = "你"_utf8;
std::cout << utf8_ch << '\n';

UTF8String str = "你好啊\xf0\xa0\x80\x80"_utf8;
str.chars() | for_each([](auto ch) {
    std::cout << ch << '\n';
});
std::cout << str << std::endl;

str.push_back("!"_utf8);
str.push_back("哈"_utf8);
std::cout << str << std::endl;
```
输出如下:
```text
你
你
好
啊
𠀀
你好啊𠀀
你好啊𠀀!哈
```

#### 类型运算
`mstl`提供了一套编译时的类型运算工具.
``` c++
    constexpr auto tuple = make_tuple(1, 2.0, 3l);
    constexpr auto ls1 = extract_from(tuple);       // 从类模板中提取模板参数
    static_assert(ls1 == TypeList<int, double, long>);

    constexpr auto ls2 = TypeList<int, double>;
    constexpr auto t1 = ls2 | apply_to<Tuple>();    // 把类型列表应用到类模板中
    static_assert(t1 == TypeConstInstance<Tuple<int, double>>);

    constexpr auto ls3 = ls2 | append<char, float>(); // 在类型列表后添加元素
    static_assert(ls3 == TypeList<int, double, char, float>);

    constexpr auto ls4 = ls3 | prepend<double*, const char*>();  // 在类型列表前添加元素
    static_assert(ls4 == TypeList<double*, const char*, int, double, char, float>);

    static_assert(ls4.at<1>() == TypeConstInstance<const char*>); // 按索引取出元素

    constexpr auto ls5 = ls4
                       | filter([]<typename T>(TypeConst<T>) {  // 过滤类型元素, 如同普通元素那样
                           return_v<(sizeof(T) < 8)>;           // 使用return_v返回一个编译期常量值. 此处返回bool类型, 滤去size小于8的类型
                       })
                       | map([]<typename T>(TypeConst<T>) {     // 转换类型元素, 如同普通元素那样.
                           return_t<T*>;                        // 使用return_t返回一个类型. 此处使列表中的每个元素T转换成T*.
                       });
    static_assert(ls5 == TypeList<int*, char*, float*>);
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

## 加入我们
我们欢迎您的积极参与, 您可以借由以下方法参与到本项目的开发之中:

1. 就您认为可改进的地方, 希望实现的功能等提出issue, 我们随时欢迎您的指教.
2. fork本仓库, 帮助我们完善文档和代码, 实现新功能. 

作为一个全新的项目, `mstl`还有很多不足. 
我们期待您的加入, 让我们一起打造更强大的Library!