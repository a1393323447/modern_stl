//
// Created by Shiroan on 2022/9/14.
//

#ifndef MODERN_STL_VECTOR_H
#define MODERN_STL_VECTOR_H

#include <initializer_list>
#include <algorithm>
#include <ostream>

#include <global.h>
#include <iter/iter_concepts.h>
#include <option/option.h>
#include <memory/allocators/allocator_concept.h>

using mstl::memory::allocators::Allocator;

namespace mstl::collection {

    template<typename T>
    class VectorIter;

    template <typename T, Allocator A = std::allocator<T>>
    class Vector {
    public:
        using Item = T;
        using IntoIter = VectorIter<T>;
        using AllocatorType = A;

        constexpr Vector(std::initializer_list<T> list, const A& allocator = A{}): len{0}, cap{0}, alloc(allocator) {
            copy_impl(list);
        }

        constexpr Vector(): alloc{} {
            len = 0;
            cap = 0;
        }

        constexpr Vector(const A& allocator): alloc(allocator), len(0), cap(0) {

        }

        constexpr Vector(usize count, const T& val, const A& allocator={}): len{0}, cap{0}, alloc(allocator) {
            assign(count, val);
        }

        constexpr Vector(const Vector& r): len{}, cap{}, alloc{} {
            copy_impl(r);
        }

        constexpr Vector(const Vector& r, const A& allocator): len{}, cap{}, alloc(allocator) {
            copy_impl(r);
        }

        constexpr Vector(Vector&& r)  noexcept: len{0}, cap{0}, alloc{} {
            move_impl(std::forward<Vector&&>(r));
        }

        constexpr Vector(Vector&& r, const A& allocator)  noexcept: len{0}, cap{0}, alloc(allocator) {
            move_impl(r);
        }

        constexpr ~Vector() {
            delete[] beginPtr;
        }

        constexpr Vector& operator=( const Vector& other ) {
            if (&other == this) {
                return *this;
            }
            delete[] beginPtr;

            copy_impl(other);

            return *this;
        }

        constexpr Vector& operator=( Vector&& r ) noexcept {
            delete[] beginPtr;

            move_impl(r);
            return *this;
        }

        Vector& operator=( std::initializer_list<T> list ) {
            delete[] beginPtr;

            copy_impl(list);
            return *this;
        }

        constexpr T& operator[](usize pos) {
            return beginPtr[pos];
        }

        constexpr const T& operator[](usize pos) const {
            return beginPtr[pos];
        }

        constexpr AllocatorType get_allocator() const noexcept {
            return alloc;
        }

    public:
        constexpr void assign(usize count, const T& val) {
            delete[] beginPtr;

            len = count;
            cap = count;

            beginPtr = new T[cap];
            endPtr = beginPtr + len;
            capEndPtr = beginPtr + cap;

            for (int i = 0; i < count; i++) {
                beginPtr[i] = {val};
            }
        }

        constexpr void assign( std::initializer_list<T> list ) {
            delete[] beginPtr;

            copy_impl(list);
        }

        /**
         * 返回Vector位于pos处的元素的引用. 若数组越界, 则返回Option::none().
         * */
        constexpr Option<T&> at(usize pos) {
            if (pos < len) {
                return Option<T&>::some(beginPtr[pos]);
            } else {
                return Option<T&>::none();
            }
        }

        constexpr Option<const T&> at(usize pos) const {
            if (pos < len) {
                return Option<T&>::some(beginPtr[pos]);
            } else {
                return Option<T&>::none();
            }
        }

        constexpr T& at_unchecked(usize pos){
            return beginPtr[pos];
        }

        constexpr const T& at_unchecked(usize pos) const{
            return beginPtr[pos];
        }

        constexpr Option<T&> front() {
            if (len != 0) {
                return Option<T&>::some(beginPtr[0]);
            } else {
                return Option<T&>::none();
            }
        }

        constexpr Option<const T&> front() const {
            if (len != 0) {
                return Option<T&>::some(beginPtr[0]);
            } else {
                return Option<T&>::none();
            }
        }

        constexpr T& front_unchecked() {
            return beginPtr[0];
        }

        constexpr const T& front_unchecked() const {
            return beginPtr[0];
        }

        constexpr Option<T&> back() {
            if (len != 0) {
                return Option<T&>::some(beginPtr[len - 1]);
            } else {
                return Option<T&>::none();
            }
        }

        constexpr Option<const T&> back() const {
            if (len != 0) {
                return Option<T&>::some(beginPtr[len - 1]);
            } else {
                return Option<T&>::none();
            }
        }

        constexpr T& back_unchecked() {
            return beginPtr[len - 1];
        }

        constexpr const T& back_unchecked() const {
            return beginPtr[len - 1];
        }

        constexpr T* data() noexcept{
            return beginPtr;
        }

        constexpr const T* data() const noexcept {
            return beginPtr;
        }

    public:
        IntoIter into_iter() {
            return IntoIter {beginPtr, endPtr};
        }

        IntoIter begin() {
            return IntoIter {beginPtr, endPtr, beginPtr};
        }

        IntoIter end() {
            return IntoIter {beginPtr, endPtr, endPtr};
        }

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
        constexpr bool empty() const {
            return len == 0;
        }

        constexpr usize size() const {
            return len;
        }

        constexpr void reserve(usize newCap) {
            if (newCap > cap) {
                cap = newCap;
                auto newSpace = new Item[newCap];
                if (len != 0) {
                    std::copy(beginPtr, endPtr, newSpace);
                }
                delete[] beginPtr;

                beginPtr = newSpace;
                endPtr = beginPtr + len;
                capEndPtr = beginPtr + cap;
            }
        }

        constexpr usize capacity() const{
            return cap;
        }

    public:
        constexpr void clear() {
            delete[] beginPtr;
            beginPtr = endPtr = capEndPtr = nullptr;
            len = cap = 0;
        }

//        todo constexpr iter insert(iter pos, const T& value);

//        todo template<typename ...Args> void emplace(const_iter, ...);

//        todo erase(iter)

//        FIXME Vector仍然有严重问题, 如要求T必须可默认构造.

        constexpr void push_back(const T& v) {
            if (len >= cap) {
                extend_space();
            }

            beginPtr[len++] = v;
        }

        constexpr void push_back(T&& v) {
            if (len >= cap) {
                extend_space();
            }

            beginPtr[len++] = v;
        }

        template< typename ... Args >
        constexpr T& emplace_back( Args&&... args ) {
            if (len >= cap) {
                extend_space();
            }

            beginPtr[len++] = T{std::forward<Args>(args)...};
        }

        // todo resize()

        constexpr void pop_back() noexcept {
            // FIXME 如何对尾元素进行销毁?: With allocator and "destroy_at"
            len--;
            endPtr--;
        }

        constexpr void swap(Vector& o) noexcept {
            std::swap(len, o.len);
            std::swap(cap, o.cap);
            std::swap(alloc, o.alloc);

            std::swap(beginPtr, o.beginPtr);
            std::swap(endPtr, o.endPtr);

            if constexpr ( std::allocator_traits<AllocatorType>::propagate_on_container_swap::value) {  // ?
                std::swap(capEndPtr, o.capEndPtr);
            }
        }

    private:
        usize len;
        usize cap;

        T* beginPtr = nullptr;
        T* endPtr = nullptr;
        T* capEndPtr = nullptr;

        A alloc;  // TODO Reimplement the vector with alloc

    private:
        constexpr void move_impl(Vector&& r) noexcept {
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
        }

        constexpr void copy_impl(const Vector& r) {
            len = r.len;
            cap = r.cap;
            beginPtr = new T[cap];
            endPtr = beginPtr + len;
            capEndPtr = beginPtr + cap;

            std::copy(r.beginPtr, r.endPtr, beginPtr);
        }

        constexpr void copy_impl(const std::initializer_list<T>& list) {
            usize _len = list.size();

            len = _len;
            cap = _len;

            beginPtr = new T[cap];
            endPtr = beginPtr + len;
            capEndPtr = beginPtr + cap;

            std::copy(list.begin(), list.end(), beginPtr);
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
        VectorIter(Item* beg, Item* end): beg(beg), cur(beg), end(end) {}
        VectorIter(Item* beg, Item* end, Item* cur): beg(beg), cur(cur), end(end) {}

        Option<T> next() {
            if (cur != end) {
                return Option<T>::some(*cur++);
            } else {
                return Option<T>::none();
            }
        }

        T& operator*() {
            return *cur;
        }

        const T& operator*() const {
            return *cur;
        }

        VectorIter& operator++() {
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
        Item* const beg = nullptr;
        Item* cur = nullptr;
        Item* const end = nullptr;
    };

    template <typename T, typename U>
    requires std::equality_comparable_with<T, U>
    bool operator==(const Vector<T>& lhs, const Vector<U>& rhs) {
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

//    template<mstl::basic::Printable T>
//    std::ostream &operator<<(std::ostream &os, Vector<T> &vector) {
//        for (auto& i : vector) {
//            os << i;  // todo Reimplement with iterator?
//        }
//        return os;
//    }
}



#endif //MODERN_STL_VECTOR_H
