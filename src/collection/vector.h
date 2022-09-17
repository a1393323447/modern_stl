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

    template<typename T, Allocator A, bool Reversed>
    class VectorIntoIter;

    template<typename T, Allocator A>
    class Vector;

    namespace _private {
        template<typename T, Allocator A>
        VectorIntoIter<T, A, false> vec_into_iter(Vector<T, A>&& v) {
            return VectorIntoIter<T, A, false>{std::forward<Vector<T, A>&&>(v)};
        }

        template<typename T, Allocator A>
        VectorIntoIter<T, A, true> vec_into_iter_reversed(Vector<T, A>&& v) {
            return VectorIntoIter<T, A, true>{std::forward<Vector<T, A>&&>(v)};
        }
    }

    template<typename T, Allocator A = std::allocator<T>>
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
            return _private::vec_into_iter(std::move(*this));
        }

        IntoIterReversed into_iter_reversed() {
            return _private::vec_into_iter_reversed(std::move(*this));
        }

        Iter iter() {
            return Iter{beginPtr, beginPtr + len};
        }

        ConstIter citer() const {
            return ConstIter{beginPtr, beginPtr + len};
        }

        Iter begin() {
            return Iter{beginPtr, beginPtr + len, beginPtr};
        }

        Iter end() {
            return Iter{beginPtr, beginPtr + len, beginPtr + len};
        }

        ConstIter cbegin() const {
            return ConstIter{beginPtr, beginPtr + len, beginPtr};
        }

        ConstIter cend() const {
            return ConstIter{beginPtr, beginPtr + len, beginPtr + len};
        }

        ConstIter begin() const {
            return cbegin();
        }

        ConstIter end() const {
            return cend();
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

        template<typename ...Args>
        void emplace(ConstIter pos, Args...vs) {
            if (len == cap) {
                extend_space();
            }

            auto hi = len;
            auto lo = pos.pos();

            while (hi != lo) {  // Move every element to the next position
                construct_at(hi, std::move(beginPtr[hi - 1]));
                hi--;
                destroy_at(hi);
            }

            construct_at(lo, std::forward<Args>(vs)...);
            len++;
        }

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

        constexpr void push_back(const T &v) {
            if (len >= cap) {
                extend_space();
            }

            construct_at(len++, v);
        }

        constexpr void push_back(T &&v) {
            if (len >= cap) {
                extend_space();
            }

            construct_at(len++, std::forward<T &&>(v));
        }

        template<typename ... Args>
        constexpr T &emplace_back(Args &&... args) {
            if (len >= cap) {
                extend_space();
            }

            construct_at(len++, std::forward<Args>(args)...);
        }

         void resize(usize count) {
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
                    push_back(T{});
                    d--;
                }
            }
        }

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

        constexpr void pop_back() noexcept {
            destroy_at(len - 1);
            len--;
        }

        constexpr void swap(Vector &o) noexcept {
            std::swap(len, o.len);
            std::swap(cap, o.cap);

            std::swap(beginPtr, o.beginPtr);

            if constexpr (std::allocator_traits<AllocatorType>::propagate_on_container_swap::value) {  // ?
                std::swap(alloc, o.alloc);
            }
        }

    private:
        usize len{};
        usize cap{};

        T *beginPtr = nullptr;

        A alloc;

    private:
        // Deallocate memory without destroy any element.
        constexpr void deallocate() noexcept {
            alloc.deallocate(beginPtr, cap);

            beginPtr = nullptr;
            len = cap = 0;
        }

        // Allocate raw space.
        constexpr void allocate(usize size) noexcept {
            len = 0;
            cap = size;

            beginPtr = alloc.allocate(size);
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
    public:
        using Item = T&;
        using value_type = T;

        VectorIter(T *beg, T *end) : beg(beg), cur(beg), end(end) {}

        VectorIter(T *beg, T *end, T *cur) : beg(beg), cur(cur), end(end) {}

        VectorIter(const VectorIter& r) : beg(r.beg), cur(r.cur), end(r.end) {}

        operator VectorIter<const T> () {
            return {beg, end, cur};
        }

        Option<Item> next() {
            if (cur != end) {
                return Option<Item>::some(*cur++);
            } else {
                return Option<Item>::none();
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

        VectorIter &operator--() {
            cur--;
            return *this;
        }

        VectorIter operator--(int) {
            auto tmp = *this;
            cur--;
            return tmp;
        }

        VectorIter operator+(usize i) {
            auto tmp = *this;
            tmp.cur += i;
            return tmp;
        }

        VectorIter operator-(usize i) {
            auto tmp = *this;
            tmp.cur -= i;
            return tmp;
        }

        VectorIter& operator+=(usize i) {
            cur += i;
            return *this;
        }

        VectorIter& operator-=(usize i) {
            cur -= i;
            return *this;
        }

        bool operator==(const VectorIter &rhs) const {
            return beg == rhs.beg &&
                   cur == rhs.cur &&
                   end == rhs.end;
        }

        std::weak_ordering operator<=>(const VectorIter& rhs) const {
            return { cur <=> rhs.cur };
        }

        usize pos() const {
            return cur - beg;
        }
    private:
        T *const beg = nullptr;
        T *cur = nullptr;
        T *const end = nullptr;
    };

    template<typename T, Allocator A>
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

    template<typename T, Allocator A>
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
