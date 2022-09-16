//
// Created by Shiroan on 2022/9/14.
//

#ifndef MODERN_STL_VECTOR_H
#define MODERN_STL_VECTOR_H

#include <initializer_list>
#include <algorithm>
#include <ostream>
#include <compare>

#include <global.h>
#include <iter/iter_concepts.h>
#include <option/option.h>
#include <memory/allocators/allocator_concept.h>

using mstl::memory::allocators::Allocator;

namespace mstl::collection {

    template<typename T>
    class VectorIter;

    template<typename T, Allocator A = std::allocator<T>>
    class Vector {
    public:
        using Item = T;
        using IntoIter = VectorIter<T>;
        using AllocatorType = A;

        constexpr Vector(std::initializer_list<T> list, const A &allocator = A{}) : len{0}, cap{0}, alloc(allocator) {
            copy_impl(list);
        }

        constexpr Vector() : alloc{} {
            len = 0;
            cap = 0;
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

        constexpr ~Vector() {
            clear();
        }

        constexpr Vector &operator=(const Vector &other) {
            if (&other == this) {
                return *this;
            }
            clear();

            copy_impl(other);

            return *this;
        }

        constexpr Vector &operator=(Vector &&r) noexcept {
            clear();

            move_impl(std::forward<Vector&&>(r));
            return *this;
        }

        Vector &operator=(std::initializer_list<T> list) {
            clear();

            copy_impl(list);
            return *this;
        }

        constexpr T &operator[](usize pos) {
            return beginPtr[pos];
        }

        constexpr const T &operator[](usize pos) const {
            return beginPtr[pos];
        }

        constexpr AllocatorType get_allocator() const noexcept {
            return alloc;
        }

    public:
        constexpr void assign(usize count, const T &val) {
            clear();

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
         * 返回Vector位于pos处的元素的引用. 若数组越界, 则返回Option::none().
         * */
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

        constexpr T &at_unchecked(usize pos) {
            return beginPtr[pos];
        }

        constexpr const T &at_unchecked(usize pos) const {
            return beginPtr[pos];
        }

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

        constexpr T &front_unchecked() {
            return beginPtr[0];
        }

        constexpr const T &front_unchecked() const {
            return beginPtr[0];
        }

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

        constexpr T &back_unchecked() {
            return beginPtr[len - 1];
        }

        constexpr const T &back_unchecked() const {
            return beginPtr[len - 1];
        }

        constexpr T *data() noexcept {
            return beginPtr;
        }

        constexpr const T *data() const noexcept {
            return beginPtr;
        }

    public:
        IntoIter into_iter() {
            return IntoIter{beginPtr, endPtr};
        }

        IntoIter begin() {
            return IntoIter{beginPtr, endPtr, beginPtr};
        }

        IntoIter end() {
            return IntoIter{beginPtr, endPtr, endPtr};
        }

        template<iter::Iterator Iter>
        static decltype(auto) from_iter(Iter iter) {
            Vector v;
            auto val = iter.next();
            while (val.is_some()) {
                v.push_back(val.unwrap_uncheck());
                val = iter.next();
            }
            return v;
        }

    public:
        constexpr bool empty() const {
            return len == 0;
        }

        constexpr usize size() const {
            return len;
        }

        constexpr void reserve(usize newCap) {
            if (newCap > cap) {
                allocate_reserve(newCap);
            }
        }

        constexpr usize capacity() const {
            return cap;
        }

    public:
        // Destroy all elements and deallocate reserved space
        constexpr void clear() {
            for (usize i = 0; i < len; i++) {
                destroy_at(i);
            }
            deallocate();
        }

//        todo constexpr iter insert(iter pos, const T& value);

//        todo template<typename ...Args> void emplace(const_iter, ...);

//        todo erase(iter)

        constexpr void push_back(const T &v) {
            if (len >= cap) {
                extend_space();
            }

            construct_at(len++, v);
            endPtr++;
        }

        constexpr void push_back(T &&v) {
            if (len >= cap) {
                extend_space();
            }

            construct_at(len++, std::forward<T &&>(v));
            endPtr++;
        }

        template<typename ... Args>
        constexpr T &emplace_back(Args &&... args) {
            if (len >= cap) {
                extend_space();
            }

            construct_at(len++, std::forward<Args>(args)...);
            endPtr++;
        }

        // todo resize()

        constexpr void pop_back() noexcept {
            destroy_at(len - 1);
            len--;
            endPtr--;
        }

        constexpr void swap(Vector &o) noexcept {
            std::swap(len, o.len);
            std::swap(cap, o.cap);
            std::swap(alloc, o.alloc);

            std::swap(beginPtr, o.beginPtr);
            std::swap(endPtr, o.endPtr);

            if constexpr (std::allocator_traits<AllocatorType>::propagate_on_container_swap::value) {  // ?
                std::swap(capEndPtr, o.capEndPtr);
            }
        }

    private:
        usize len{};
        usize cap{};

        T *beginPtr = nullptr;
        T *endPtr = nullptr;
        T *capEndPtr = nullptr;

        A alloc;

    private:
        // Deallocate memory without destroy any element.
        constexpr void deallocate() noexcept {
            alloc.deallocate(beginPtr, cap);

            beginPtr = endPtr = capEndPtr = nullptr;
            len = cap = 0;
        }

        // Allocate raw space.
        constexpr void allocate(usize size) noexcept {
            len = 0;
            cap = size;

            beginPtr = alloc.allocate(size);
            endPtr = beginPtr;
            capEndPtr = beginPtr + cap;
        }

        constexpr void allocate_reserve(usize size) noexcept {
            auto nArr = alloc.allocate(size);  // Alloc
            for (usize i = 0; i < len; i++) {
                std::construct_at(nArr + i, beginPtr[i]);
            }
            usize oLen = len;

            clear();
            len = oLen;
            cap = size;
            beginPtr = nArr;
            endPtr = beginPtr + len;
            capEndPtr = beginPtr + cap;
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
            endPtr = r.endPtr;
            r.endPtr = nullptr;
            capEndPtr = r.capEndPtr;
            r.capEndPtr = nullptr;
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
            usize _len = list.size();

            allocate(_len);
            len = _len;
            usize i = 0;
            for (auto &ele: list) {
                construct_at(i, ele);
                i++;
            }
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
        // TODO 移动入迭代器
    public:
        using Item = T;

        VectorIter(Item *beg, Item *end) : beg(beg), cur(beg), end(end) {}

        VectorIter(Item *beg, Item *end, Item *cur) : beg(beg), cur(cur), end(end) {}

        Option<T> next() {
            if (cur != end) {
                return Option<T>::some(*cur++);
            } else {
                return Option<T>::none();
            }
        }

        T &operator*() {
            return *cur;
        }

        const T &operator*() const {
            return *cur;
        }

        VectorIter &operator++() {
            cur++;
            return *this;
        }

        VectorIter operator++(int) {
            auto tmp = *this;
            cur++;
            return tmp;
        }

        bool operator==(const VectorIter &rhs) const {
            return beg == rhs.beg &&
                   cur == rhs.cur &&
                   end == rhs.end;
        }

    private:
        Item *const beg = nullptr;
        Item *cur = nullptr;
        Item *const end = nullptr;
    };

    template<typename T, typename U>
    requires std::equality_comparable_with<T, U>
    bool operator==(const Vector<T> &lhs, const Vector<U> &rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        } else {
            for (int i = 0; i < lhs.size(); i++) {
                if (lhs[i] != rhs[i]) {
                    return false;
                }
            }
        }
        return true;
    }

    template<mstl::basic::Printable T>
    std::ostream &operator<<(std::ostream &os, const Vector<T> &vector) {
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

    template<typename T, typename U>
    auto operator<=>(const Vector<T>& lhs, const Vector<U>& rhs)
    requires requires (T t, U u, usize len){
        std::three_way_comparable_with<T, U, std::partial_ordering>;
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


#endif //MODERN_STL_VECTOR_H
