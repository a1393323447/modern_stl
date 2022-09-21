//
// Created by 朕与将军解战袍 on 2022/9/20.
//

#ifndef __MODERN_STL_RANGE_H__
#define __MODERN_STL_RANGE_H__

#include <ops/cmp.h>
#include <ops/arithmetical.h>
#include <basic_concepts.h>
#include <option/option.h>

namespace mstl::ops {
    template<typename Idx>
    class RangeCursor {
    public:
        RangeCursor(Idx p): pos(p) {}

        template<typename I = Idx>
        requires basic::CopyAble<I> &&
                 std::same_as<I, Idx>
        RangeCursor(const RangeCursor<int>& other) {
            pos = other.pos;
        }

        template<typename I = Idx>
        requires Inc<I> &&
                 std::same_as<I, Idx>
        RangeCursor<I>& operator++() {
            ++pos;
            return *this;
        }

        template<typename I = Idx>
        requires Inc<I> &&
                 std::same_as<I, Idx>
        RangeCursor<I> operator++(int) {
            RangeCursor<I> old = *this;
            ++pos;
            return old;
        }

        template<typename I = Idx>
        requires Add<I> &&
                 std::same_as<I, Idx>
        RangeCursor<I> operator+(RangeCursor<I> rhs) {
            return { pos + rhs.pos };
        }

        template<typename I = Idx>
        requires Inc<I> &&
                 std::same_as<I, Idx>
        RangeCursor<I>& operator--() {
            --pos;
            return *this;
        }

        template<typename I = Idx>
        requires Inc<I> &&
                 std::same_as<I, Idx>
        RangeCursor<I> operator--(int) {
            RangeCursor<I> old = *this;
            --pos;
            return old;
        }

        template<typename I = Idx>
        requires Sub<I> &&
                 std::same_as<I, Idx>
        RangeCursor<I> operator-(RangeCursor<I> rhs) {
            return { pos - rhs.pos };
        }

        template<typename I = Idx>
        requires Inc<I> &&
                 std::same_as<I, Idx>
        I operator*() const {
            return pos;
        }

        template<typename I>
        requires WeakOrd<I, Idx> && (!StrongOrd<I, Idx>)
        std::weak_ordering operator<=>(const RangeCursor<I>& rhs) const {
            return pos <=> rhs.pos;
        }

        template<typename I>
        requires WeakOrd<I, Idx> && (!StrongOrd<I, Idx>)
        bool operator==(const RangeCursor<I>& rhs) const {
            return pos == rhs.pos;
        }

        template<typename I>
        requires StrongOrd<I, Idx>
        std::strong_ordering operator<=>(const RangeCursor<I>& rhs) const {
            return pos <=> rhs.pos;
        }

        template<typename I>
        requires StrongOrd<I, Idx>
        bool operator==(const RangeCursor<I>& rhs) const {
            return pos == rhs.pos;
        }

    private:
        Idx pos;
    };

    template<typename Idx>
    requires WeakOrd<Idx, Idx> &&
             basic::CopyAble<Idx>
    struct Range {
        using Item = Idx;

        Range(Idx low, Idx high): low(low), high(high) {}

        Range(Range<Idx>&) = default;
        Range& operator=(const Range<Idx>&) = default;

        template<typename U>
        requires WeakOrd<Idx, U> && WeakOrd<U, Idx>
        bool contains(U&& item) {
            return low <= item && item < high;
        }

        template<typename I = Idx>
        requires Inc<I> &&
                 std::same_as<I, Idx>
        Option<I> next() {
            if (low == high) {
                return Option<I>::none();
            } else {
                auto value = Option<I>::some(low);
                low++;
                return value;
            }
        }

        template<typename I = Idx>
        requires Inc<I> && Dec<I> &&
                 std::same_as<I, Idx>
        RangeCursor<I> begin() {
            return { low };
        }

        template<typename I = Idx>
        requires Inc<I> && Dec<I> &&
                 std::same_as<I, Idx>
        RangeCursor<I> end() {
            return { high };
        }

        template<typename U>
        requires Eq<Idx, U>
        bool operator==(const Range<U>& rhs) {
            return low == rhs.low && high == rhs.high;
        }

        Idx low;
        Idx high;
    };
}

template<typename T>
requires mstl::ops::Inc<T>
struct std::iterator_traits<mstl::ops::RangeCursor<T>>
{
    typedef std::forward_iterator_tag iterator_category;
    typedef T                         value_type;
    typedef ptrdiff_t                 difference_type;
    typedef T*                        pointer;
    typedef T&                        reference;
};


template<typename T>
requires mstl::ops::Inc<T> && mstl::ops::Dec<T>
struct std::iterator_traits<mstl::ops::RangeCursor<T>>
{
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef T                                value_type;
    typedef ptrdiff_t                        difference_type;
    typedef T*                               pointer;
    typedef T&                               reference;
};


template<typename T>
requires mstl::ops::Inc<T> && mstl::ops::Dec<T> &&
         mstl::ops::Add<T> && mstl::ops::Sub<T>
struct std::iterator_traits<mstl::ops::RangeCursor<T>>
{
    typedef std::random_access_iterator_tag iterator_category;
    typedef T                               value_type;
    typedef ptrdiff_t                       difference_type;
    typedef T*                              pointer;
    typedef T&                              reference;
};

#endif //__MODERN_STL_RANGE_H__
