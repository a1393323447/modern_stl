//
// Created by Shiroan on 2022/9/14.
//

#ifndef MODERN_STL_VECTOR_H
#define MODERN_STL_VECTOR_H

#include <initializer_list>
#include <algorithm>
#include <ostream>
#include <compare>
#include <iterator>

#include <mstl/global.h>
#include <mstl/iter/iterator.h>
#include <mstl/option/option.h>
#include <mstl/memory/memory.h>
#include <mstl/ops/cmp.h>

namespace mstl::collection {

    template<typename T>
    class VectorIter;

    template<typename T, mstl::memory::concepts::Allocator A, bool Reversed>
    class VectorIntoIter;

    template<typename T, mstl::memory::concepts::Allocator A>
    requires (!basic::RefType<T>)
    class Vector;

    namespace _private {
        template<typename T, mstl::memory::concepts::Allocator A>
        VectorIntoIter<T, A, false> vec_into_iter(Vector<T, A>&& v) {
            return VectorIntoIter<T, A, false>{std::forward<Vector<T, A>&&>(v)};
        }

        template<typename T, mstl::memory::concepts::Allocator A>
        VectorIntoIter<T, A, true> vec_into_iter_reversed(Vector<T, A>&& v) {
            return VectorIntoIter<T, A, true>{std::forward<Vector<T, A>&&>(v)};
        }
    }

    /**
     * @brief 连续储存的, 空间可增长的数组. 与`std::vector`相类似.
     *
     * @tparam T 储存的元素类型
     * @tparam A 分配器类型
     *
     * ## Example
     * @code
     *      Vector<int> vec = {1, 2, 3};
     *      assert(vec[0] == 1);
     *      vec.push_back(4);
     *      assert(vec[3] == 4);
     * @endcode
     */
    template<typename T, mstl::memory::concepts::Allocator A = mstl::memory::allocator::Allocator>
    requires (!basic::RefType<T>)
    class Vector {
    public:
        using Item = T;
        using IntoIter = VectorIntoIter<T, A, false>;
        using IntoIterReversed = VectorIntoIter<T, A, true>;
        using AllocatorType = A;
        using Iter = VectorIter<T>;
        using ConstIter = VectorIter<const T>;

        constexpr Vector(std::initializer_list<T> list, const A &allocator = A{}) : len{0}, cap{0}, alloc(allocator) {
            copy_impl(list);
        }

        constexpr Vector() : alloc{} {
            allocate(2);
        }

        constexpr Vector(const A &allocator) : alloc(allocator), len(0), cap(0) {

        }

        explicit constexpr Vector(usize count, const A &allocator = {}): Vector(count, {}, allocator) {}

        constexpr Vector(usize count, const T &val, const A &allocator = {}) : len{0}, cap{0}, alloc(allocator) {
            assign(count, val);
        }

        constexpr Vector(const Vector &r) : len{}, cap{}, alloc{} {
            copy_impl(r);
        }

        constexpr Vector(const Vector &r, const A &allocator) : len{}, cap{}, alloc(allocator) {
            copy_impl(r);
        }

        constexpr Vector(Vector &&r) noexcept: len{0}, cap{0}, alloc{} {
            move_impl(std::forward<Vector &&>(r));
        }

        constexpr Vector(Vector &&r, const A &allocator) noexcept: len{0}, cap{0}, alloc(allocator) {
            move_impl(std::forward<Vector&&>(r));
        }

        template< iter::LegacyInputIterator InputIt >
        constexpr Vector(InputIt first, InputIt last, const A& alloc = A{}): alloc(alloc) {
            usize distance = std::abs(std::distance(first, last));
            allocate(distance);
            while (first != last) {
                construct_at(len++, *first);
                first++;
            }
        }

        constexpr ~Vector() {
            clear();
        }

        // fixme
        constexpr Vector &operator=(const Vector &other) {
            if (&other == this) {
                return *this;
            }
            clear();

            copy_impl(other);

            return *this;
        }

        constexpr Vector &operator=(Vector &&r) noexcept {
            if (&r == this) {
                return *this;
            }
            clear();

            move_impl(std::forward<Vector&&>(r));
            return *this;
        }

        constexpr Vector &operator=(std::initializer_list<T> list) {
            clear();

            copy_impl(list);
            return *this;
        }

        /**
         * @brief 获取位于pos的元素.
         * @note 在DEBUG模式下, 该函数进行越界检查: 当下标越界, 则引发panic.
         * @param pos 元素的索引.
         *
         * ## Example
         * @code
         *      Vector<int> vec = {1, 2, 3};
         *      assert(vec[0] == 1);
         *      // vec[4];  // this will raise panic.
         * @endcode
         */
        constexpr T &operator[](usize pos) {
            MSTL_DEBUG_ASSERT(pos < len, "Out of range.");
            return beginPtr[pos];
        }

        constexpr const T &operator[](usize pos) const {
            MSTL_DEBUG_ASSERT(pos < len, "Out of range.");
            return beginPtr[pos];
        }

        constexpr AllocatorType get_allocator() const noexcept {
            return alloc;
        }

    public:
        /**
         * @brief 对Vector赋值, 以使其储存count个val.
         *
         * @param[in] count
         * @param[in] val
         *
         * ## Example
         * @code
         *      Vector<int> vec;
         *      vec.assign(3, 2);
         *      assert(vec == Vector<int>{2, 2, 2});
         * @endcode
         */
        constexpr void assign(usize count, const T &val) {
            clear();  // fixme reuse

            allocate(count);

            len = count;

            for (usize i = 0; i < count; i++) {
                construct_at(i, val);
            }
        }

        constexpr void assign(std::initializer_list<T> list) {
            clear();

            copy_impl(list);
        }

        /**
         * @brief 安全地取出Vector中储存的元素.
         * @param pos 元素的索引
         * @return 返回第pos个元素的引用的Option.
         *
         * ## Example
         * @code
         *      Vector<int> vec{1, 2, 3};
         *      assert(vec.at(0).unwrap() == 1);
         *      assert(vec.at(4).is_none());
         * @endcode
         */
        constexpr Option<T &> at(usize pos) {
            if (pos < len) {
                return Option<T &>::some(beginPtr[pos]);
            } else {
                return Option<T &>::none();
            }
        }

        constexpr Option<const T &> at(usize pos) const {
            if (pos < len) {
                return Option<T &>::some(beginPtr[pos]);
            } else {
                return Option<T &>::none();
            }
        }

        /**
         * @brief 取出Vector中储存的元素, 且不进行越界检查.
         * @param pos 元素的索引
         * @return 返回第pos个元素的引用. 若下标越界, 则行为未定义.
         *
         * ## Example
         * @code
         *      Vector<int> vec{1, 2, 3};
         *      assert(vec.at_unchecked(0) == 1);
         * @endcode
         */
        constexpr T &at_unchecked(usize pos) {
            return beginPtr[pos];
        }

        constexpr const T &at_unchecked(usize pos) const {
            return beginPtr[pos];
        }

        /**
         * @brief 安全地取出Vector中的第一个元素的引用.
         * @return 返回第一个元素的引用的Option.
         *
         * ## Example
         * @code
         *      Vector<int> vec{1, 2, 3};
         *      assert(vec.front().unwrap() == 1);
         *      vec.clear();
         *      assert(vec.front().is_none());
         * @endcode
         */
        constexpr Option<T &> front() {
            if (len != 0) {
                return Option<T &>::some(beginPtr[0]);
            } else {
                return Option<T &>::none();
            }
        }

        constexpr Option<const T &> front() const {
            if (len != 0) {
                return Option<T &>::some(beginPtr[0]);
            } else {
                return Option<T &>::none();
            }
        }

        /**
         * @brief 取出Vector中储存的第一个元素, 且不进行越界检查.
         * @return 返回第pos个元素的引用. 若Vector为空, 则行为未定义.
         */
        constexpr T &front_unchecked() {
            return beginPtr[0];
        }

        constexpr const T &front_unchecked() const {
            return beginPtr[0];
        }

        /**
         * @brief 安全地取出Vector中储存的最后一个元素.
         * @return 返回最后一个元素的引用的Option.
         *
         * ## Example
         * @code
         *      Vector<int> vec{1, 2, 3};
         *      assert(vec.back().unwrap() == 3);
         *      vec.clear();
         *      assert(vec.back().is_none());
         * @endcode
         */
        constexpr Option<T &> back() {
            if (len != 0) {
                return Option<T &>::some(beginPtr[len - 1]);
            } else {
                return Option<T &>::none();
            }
        }

        constexpr Option<const T &> back() const {
            if (len != 0) {
                return Option<T &>::some(beginPtr[len - 1]);
            } else {
                return Option<T &>::none();
            }
        }

        /**
         * @brief 取出Vector中储存的最后一个元素, 且不进行越界检查.
         * @return 返回最后一个元素的引用. 若Vector为空, 则行为未定义.
         */
        constexpr T &back_unchecked() {
            return beginPtr[len - 1];
        }

        constexpr const T &back_unchecked() const {
            return beginPtr[len - 1];
        }

        /**
         * @brief 取出Vector的底层数组.
         * @return Vector的底层数组.
         *
         * ## Example
         * @code
         *      Vector<int> vec{1, 2, 3};
         *      int* ptr = vec.data();
         *      assert(ptr[0] == 1);
         * @endcode
         */
        constexpr T *data() noexcept {
            return beginPtr;
        }

        constexpr const T *data() const noexcept {
            return beginPtr;
        }

    public:
        /**
         * @brief 把Vector转换为迭代器.
         *
         * 该迭代器将接管Vector中的所有元素的所有权.
         *
         * @attention 调用该函数后, Vector将被消耗(或视为已被移动).
         * @return Vector转换而来的迭代器.
         */
        IntoIter into_iter() {
            return _private::vec_into_iter(std::move(*this));
        }

        /**
         * @brief 把Vector转换为迭代器(逆向).
         *
         * 该迭代器将接管Vector中的所有元素的所有权. 该迭代器逆向迭代Vector中的元素.
         *
         * @attention 调用该函数后, Vector将被消耗(或视为已被移动).
         * @return Vector转换而来的迭代器.
         */
        IntoIterReversed into_iter_reversed() {
            return _private::vec_into_iter_reversed(std::move(*this));
        }

        /**
         * @brief 获取Vector的迭代器.
         *
         * 该迭代器将储存Vector的引用.
         *
         * @return Vector的迭代器.
         */
        constexpr Iter iter() {
            return Iter{beginPtr, beginPtr + len};
        }

        /**
         * @brief 获取Vector的迭代器.
         *
         * 该迭代器将储存Vector的引用. 该迭代器迭代各元素的常量引用.
         *
         * @return Vector的迭代器.
         */
        constexpr ConstIter citer() const {
            return ConstIter{beginPtr, beginPtr + len};
        }

        constexpr Iter begin() {
            return Iter{beginPtr, beginPtr + len, beginPtr};
        }

        constexpr Iter end() {
            return Iter{beginPtr, beginPtr + len, beginPtr + len};
        }

        constexpr ConstIter cbegin() const {
            return ConstIter{beginPtr, beginPtr + len, beginPtr};
        }

        constexpr ConstIter cend() const {
            return ConstIter{beginPtr, beginPtr + len, beginPtr + len};
        }

        constexpr ConstIter begin() const {
            return cbegin();
        }

        constexpr ConstIter end() const {
            return cend();
        }

        /**
         * @brief 从一个MSTL风格的迭代器构建一个Vector.
         * 它一般由collect()函数调用.
         * @param iter MSTL风格的迭代器.
         * @attention 由于Vector不能储存引用, 因此, 迭代左值引用的迭代器将生成其所迭代的元素的副本, 而迭代右值引用的迭代器将使得其迭代元素被移入新Vector.
         * @return 新构造的Vector.
         */
        template<iter::Iterator Iter>
        static decltype(auto) from_iter(Iter iter) {
            Vector v;
            auto val = iter.next();
            while (val.is_some()) {
                v.push_back(val.unwrap_unchecked());
                val = iter.next();
            }
            return v;
        }

    public:
        /**
         * @brief 检查当前Vector是否储存有元素.
         * @return 若Vector为空, 则返回true; 否则, 返回false.
         */
        constexpr bool empty() const {
            return len == 0;
        }

        /**
         * @brief 检查当前Vector储存元素的数量.
         * @return 返回当前Vector储存元素的数量.
         */
        constexpr usize size() const {
            return len;
        }

        /**
         * @brief 为Vector预留空间.
         *
         * 若当前容量小于newCap, 则扩展到该容量; 否则什么也不做.
         *
         * @param newCap 为Vector预留的容量
         */
        constexpr void reserve(usize newCap) {
            if (newCap > cap) {
                allocate_reserve(newCap);
            }
        }

        /**
         * @brief 检查Vector的容量.
         * @return Vector的容量
         */
        constexpr usize capacity() const {
            return cap;
        }

    public:
        /**
         * @brief 清空Vector.
         *
         * 销毁所有元素, 并解分配预分配的空间.
         *
         * ## Example
         * @code
         *      Vector<int> vec = {1, 2, 3};
         *      vec.clear();
         *      assert(vec.empty());
         * @endcode
         */
        constexpr void clear() {
            for (usize i = 0; i < len; i++) {
                destroy_at(i);
            }
            deallocate();
        }

        /**
         * @brief 在pos所指向的位置构造一个元素.
         *
         * 在pos指向的位置, 以vs构造一个元素.
         *
         * @param pos 指向希望构造元素的位置.
         * @param vs  构造元素所需的参数.
         *
         * ## Example
         * @code
         *      Vector<int> vec = {1, 2, 3};
         *      vec.emplace(vec.begin(), 2);
         *      assert(vec[0] == 2);
         * @endcode
         */
        template<typename ...Args>
        constexpr void emplace(ConstIter pos, Args...vs) {
            if (len == cap) {
                extend_space();
            }

            auto p = pos.pos();
            move_elements_back(p, 1);
            construct_at(p, std::forward<Args>(vs)...);
        }

        /**
         * @brief 擦除pos所指向的元素.
         * @param pos 指向需擦除的元素的迭代器.
         * @return 指向被擦除的元素的下一个元素的迭代器.
         *
         * ## Example
         * @code
         *      Vector<int> vec = {1, 2, 3};
         *      vec.erase(vec.begin());
         *      assert(vec[0] == 2);
         * @endcode
         */
        constexpr Iter erase(ConstIter pos) {
            auto p = pos.pos();
            if (p == len - 1) {
                pop_back();
                return end();
            } else if (p >= 0 && p < len - 1) {
                std::move(beginPtr + p + 1, beginPtr + len, beginPtr + p);
                pop_back();
                return {beginPtr, beginPtr + len, beginPtr + p};
            } else {
                return {nullptr, nullptr};
            }
        }

        /**
         * @brief 擦除区间内的元素.
         *
         * 擦除[first, last)范围内的元素.
         *
         * @return 返回指向最后被擦除的元素的下一个元素的迭代器.
         */
        constexpr Iter erase(ConstIter first, ConstIter last) {
            if (first < begin() || last > end()) {  // UB
                return VectorIter<T>{nullptr, nullptr};
            } else {
                auto lo = first.pos(), hi = last.pos();
                std::move(beginPtr + hi, beginPtr + len, beginPtr + lo);
                auto d = hi - lo;

                while (d > 0) {
                    pop_back();
                    d--;
                }

                return begin() + lo;
            }
        }

        /**
         * @brief 在pos所指向的位置以复制的方法插入一个元素.
         *
         * @return 返回指向被插入的元素的迭代器.
         */
        constexpr Iter insert(ConstIter pos, const T& val) {
            usize p = pos.pos();
            emplace(pos, val);
            return begin() + p;
        }

        /**
         * @brief 在pos所指向的位置以移动的方法插入一个元素.
         *
         * @return 返回指向被插入的元素的迭代器.
         */
        constexpr Iter insert(ConstIter pos, T&& val) {
            usize p = pos.pos();
            emplace(pos, std::forward<T&&>(val));
            return begin() + p;
        }

        /**
         * @brief 在pos所指向的位置以复制的方法插入count个元素.
         *
         * @return 返回指向第一个被插入的元素的迭代器.
         */
        constexpr Iter insert(ConstIter pos, usize count, const T& val) {
            auto p = pos.pos();

            move_elements_back(p, count);

            for (usize i = 0; i < count; i++) {
                construct_at(p + i, val);
            }

            return begin() + p;
        }

        /**
         * @brief 在pos所指向的位置以复制的方法插入特定范围的元素.
         *
         * 在pos所指向的位置插入[first, pos)范围内的元素.
         *
         * @return 返回指向第一个被插入的元素的迭代器.
         */
        template<iter::LegacyInputIterator InputIt>
        constexpr Iter insert(ConstIter pos, InputIt first, InputIt last) {
            auto p = pos.pos();
            usize distance = std::abs(std::distance(first, last));

            if (len + distance > cap) {             // if no enougn space
                allocate_reserve(len + distance);   // then extend the vector
            }

            move_elements_back(p, distance);        // move elements back

            for (auto i = p; first != last; ++i, ++first) {
                construct_at(i, *first);
            }

            return begin() + p;
        }

        constexpr Iter insert(ConstIter pos, std::initializer_list<T> ilist) {
            return insert(pos, ilist.begin(), ilist.end());
        }

        /**
         * @brief 在末尾以复制的方法插入一个元素.
         */
        constexpr void push_back(const T &v) {
            if (len >= cap) {
                extend_space();
            }

            construct_at(len++, v);
        }

        /**
         * @brief 在末尾以移动的方法插入一个元素.
         */
        constexpr void push_back(T &&v) {
            if (len >= cap) {
                extend_space();
            }

            construct_at(len++, std::forward<T>(v));
        }

        /**
         * @brief 在末尾构造一个元素.
         */
        template<typename ... Args>
        constexpr T &emplace_back(Args &&... args) {
            if (len >= cap) {
                extend_space();
            }

            construct_at(len++, std::forward<Args>(args)...);
            return back_unchecked();
        }

        /**
         * @brief 改变Vector的大小.
         *
         * 若count < `size()`, 则缩小Vector到count, 并销毁多余的元素; 否则, 扩大Vector到count, 并在尾部填充默认构造的元素.
         */
         constexpr void resize(usize count) {
            if (count < len) {
                usize d = len - count;
                while (d > 0) {
                    pop_back();
                    d--;
                }
            } else if (count > len) {
                if (count > cap) {
                    reserve(count);
                }
                usize d = count - len;
                while (d > 0) {
                    emplace_back();
                    d--;
                }
            }
        }

        /**
         * @brief 改变Vector的大小.
         *
         * 若count < `size()`, 则缩小Vector到count, 并销毁多余的元素; 否则, 扩大Vector到count, 并在尾部以复制的方法填充r.
         */
        constexpr void resize(usize count, const T& r) {
            if (count < len) {
                usize d = len - count;
                while (d > 0) {
                    pop_back();
                    d--;
                }
            } else if (count > len) {
                if (count > cap) {
                    reserve(count);
                }
                usize d = count - len;
                while (d > 0) {
                    push_back(r);
                    d--;
                }
            }
        }

        /**
         * @brief 从尾部删除一个元素.
         */
        constexpr void pop_back() noexcept {
            destroy_at(len - 1);
            len--;
        }

        /**
         * @brief 交换两个Vector中储存的元素, 同时交换两者的Allocator.
         * ## Example
         * @code
         *      Vector<int> a = {1, 2, 3}, b = {4, 5, 6};
         *      a.swap(b);
         *      assert(to_string(a) == "Vec [4, 5, 6]");
         *      assert(to_string(b) == "Vec [1, 2, 3]");
         * @endcode
         */
        constexpr void swap(Vector &o) noexcept {
            std::swap(len, o.len);
            std::swap(cap, o.cap);

            std::swap(beginPtr, o.beginPtr);

            std::swap(alloc, o.alloc);
        }

    private:
        usize len{};
        usize cap{};

        T *beginPtr = nullptr;

        A alloc;

    private:
        // Deallocate memory without destroy any element.
        constexpr void deallocate() noexcept {
            alloc.template deallocate(beginPtr, cap);
            beginPtr = nullptr;
            len = cap = 0;
        }

        // Allocate raw space.
        constexpr void allocate(usize size) noexcept {
            len = 0;
            cap = size;

            beginPtr = alloc.template allocate<T>(size);
        }

        constexpr void allocate_reserve(usize size) noexcept {
            auto nArr = alloc.template allocate<T>(size);  // Alloc
            for (usize i = 0; i < len; i++) {
                std::construct_at(nArr + i, std::move(beginPtr[i]));
            }
            usize oLen = len;

            clear();
            len = oLen;
            cap = size;
            beginPtr = nArr;
        }

        template<typename ...Args>
        constexpr void construct_at(usize pos, Args &&...args) {
            std::construct_at(beginPtr + pos, std::forward<Args>(args)...);
        }

        constexpr void destroy_at(usize pos) {
            std::destroy_at(beginPtr + pos);
        }

        constexpr void move_impl(Vector &&r) noexcept {
            len = r.len;
            r.len = 0;
            cap = r.cap;
            r.cap = 0;

            beginPtr = r.beginPtr;
            r.beginPtr = nullptr;
            alloc = std::move(r.alloc);
        }

        constexpr void copy_impl(const Vector &r) {
            allocate(r.len);
            len = r.len;

            for (usize i = 0; i < len; i++) {
                construct_at(i, r[i]);
            }
        }

        constexpr void copy_impl(const std::initializer_list<T> &list) {
            usize l = list.size();

            allocate(l);
            len = l;
            usize i = 0;
            for (auto &ele: list) {
                construct_at(i, ele);
                i++;
            }
        }

        // 把元素向后移动n个位置, 改变len
        // 这将导致[pos, pos + count)范围内的元素为无效元素(垂悬引用)
        constexpr void move_elements_back(usize pos, usize count) {
            if (len == 0) {
                return;
            }
            if (len + count > cap) {
                reserve(len + count);
            }
            auto hi = len - 1;
            while (hi >= pos) {
                construct_at(hi + count, std::move(beginPtr[hi]));
                destroy_at(hi);
                hi--;
            }
            len += count;
        }

        constexpr void extend_space() {
            usize newCap;
            if (cap == 0) {
                newCap = 2;
            } else {
                newCap = 2 * cap;
            }
            reserve(newCap);
        }
    };

    template<typename T>
    class VectorIter {
    public:
        using Item = T&;
        using value_type = T;

        constexpr VectorIter(T *beg, T *end) : beg(beg), cur(beg), end(end) {}

        constexpr VectorIter(T *beg, T *end, T *cur) : beg(beg), cur(cur), end(end) {}

        constexpr VectorIter(const VectorIter& r) : beg(r.beg), cur(r.cur), end(r.end) {}

        constexpr operator VectorIter<const T> () {
            return {beg, end, cur};
        }

        constexpr Option<Item> next() {
            if (cur != end) {
                return Option<Item>::some(*cur++);
            } else {
                return Option<Item>::none();
            }
        }

        constexpr T &operator*() {
            return *cur;
        }

        constexpr const T &operator*() const {
            return *cur;
        }

        constexpr VectorIter &operator++() {
            cur++;
            return *this;
        }

        constexpr VectorIter operator++(int) {
            auto tmp = *this;
            cur++;
            return tmp;
        }

        constexpr VectorIter &operator--() {
            cur--;
            return *this;
        }

        constexpr VectorIter operator--(int) {
            auto tmp = *this;
            cur--;
            return tmp;
        }

        constexpr VectorIter operator+(usize i) {
            auto tmp = *this;
            tmp.cur += i;
            return tmp;
        }

        constexpr VectorIter operator-(usize i) {
            auto tmp = *this;
            tmp.cur -= i;
            return tmp;
        }

        constexpr usize operator-(VectorIter i) {
            return cur - i.cur;
        }

        constexpr VectorIter& operator+=(usize i) {
            cur += i;
            return *this;
        }

        constexpr VectorIter& operator-=(usize i) {
            cur -= i;
            return *this;
        }

        constexpr bool operator==(const VectorIter &rhs) const {
            return beg == rhs.beg &&
                   cur == rhs.cur &&
                   end == rhs.end;
        }

        constexpr std::weak_ordering operator<=>(const VectorIter& rhs) const {
            return { cur <=> rhs.cur };
        }

        constexpr usize pos() const {
            return cur - beg;
        }
    private:
        T *const beg = nullptr;
        T *cur = nullptr;
        T *const end = nullptr;
    };

    template<typename T, mstl::memory::concepts::Allocator A>
    class VectorIntoIter<T, A, false> {
    public:
        using Item = T;
        using VecType = Vector<Item, A>;
        VectorIntoIter(VecType && v) : vec{std::forward<VecType&&>(v)}, len(vec.size()){}
        VectorIntoIter(const VectorIntoIter& v) = delete;
        VectorIntoIter(VectorIntoIter&& v)  noexcept {
            vec = std::move(v.vec);
            pos = v.pos;
            v.pos = 0;
            len = vec.size();
        }

    public:
        Option<Item> next() {
            if (pos < len) {
                auto n = Option<Item>::some(Item{std::move(vec[pos])});
                pos++;
                if (pos == len) {
                    vec.clear();
                }
                return n;
            } else {
                return Option<Item>::none();
            }
        }

    private:
        Vector<Item, A> vec;
        usize pos = 0;
        usize len;
    };

    template<typename T, mstl::memory::concepts::Allocator A>
    class VectorIntoIter<T, A, true> {
    public:
        using Item = T;
        using VecType = Vector<Item, A>;

        explicit VectorIntoIter(VecType && v) : vec{std::forward<VecType&&>(v)}{}
        VectorIntoIter(const VectorIntoIter& v) = delete;
        explicit VectorIntoIter(VectorIntoIter&& v)  noexcept {
            vec = std::move(v.vec);
            v.pos = 0;
        }

    public:
        Option<Item> next() {
            if (!vec.empty()) {
                auto n = Option<Item>::some(Item{
                    std::move(vec.back_unchecked())
                });
                vec.pop_back();
                return n;
            } else {
                return Option<Item>::none();
            }
        }

    private:
        Vector<Item, A> vec;
    };

    template<typename T, typename U, memory::concepts::Allocator A, memory::concepts::Allocator B>
    requires ops::Eq<T, U>
    bool operator==(const Vector<T, A> &lhs, const Vector<U, B> &rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        } else {
            for (usize i = 0; i < lhs.size(); i++) {
                if (lhs[i] != rhs[i]) {
                    return false;
                }
            }
        }
        return true;
    }

    template<mstl::basic::Printable T, memory::concepts::Allocator A>
    std::ostream &operator<<(std::ostream &os, const Vector<T, A> &vector) {
        os << "Vec [";
        for (usize i = 0; i < vector.size(); i++) {
            os << vector[i];
            if (i != vector.size() - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }

    template<typename T, typename U, memory::concepts::Allocator A, memory::concepts::Allocator B>
    auto operator<=>(const Vector<T, A>& lhs, const Vector<U, B>& rhs)
    requires requires (T t, U u, usize len){
        requires std::three_way_comparable_with<T, U, std::partial_ordering>;
        { t <=> u } -> std::constructible_from<decltype(len <=> len)>;
    } {
        auto lenL = lhs.size();
        auto lenR = rhs.size();

        for (usize i = 0, j = 0; i < lenL && j < lenR; i++, j++) {
            auto p = lhs[i] <=> rhs[j];
            if (p == 0) {
                continue;
            } else {
                return p;
            }
        }

        using Order = decltype(std::declval<T>() <=> std::declval<U>());

        return Order{lenL <=> lenR};
    }

}
template<typename T>
struct std::iterator_traits<mstl::collection::VectorIter<T>>
{
    typedef random_access_iterator_tag iterator_category;
    typedef T                          value_type;
    typedef ptrdiff_t                  difference_type;
    typedef T*                         pointer;
    typedef T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::VectorIter<const T>>
{
    typedef random_access_iterator_tag iterator_category;
    typedef T                          value_type;
    typedef ptrdiff_t                  difference_type;
    typedef const T*                   pointer;
    typedef const T&                   reference;
};

#endif //MODERN_STL_VECTOR_H
