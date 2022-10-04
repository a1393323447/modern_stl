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

        RangeCursor(const RangeCursor& other)
        requires basic::CopyAble<Idx> {
            pos = other.pos;
        }

        RangeCursor& operator++() requires Inc<Idx> {
            ++pos;
            return *this;
        }

        RangeCursor operator++(int) requires Inc<Idx> {
            RangeCursor old = *this;
            ++pos;
            return old;
        }

        RangeCursor operator+(RangeCursor rhs) requires Add<Idx> {
            return { pos + rhs.pos };
        }

        RangeCursor& operator--() requires Dec<Idx> {
            --pos;
            return *this;
        }

        RangeCursor operator--(int) requires Dec<Idx> {
            RangeCursor old = *this;
            --pos;
            return old;
        }

        RangeCursor operator-(RangeCursor rhs) requires Sub<Idx> {
            return { pos - rhs.pos };
        }

        Idx operator*() const
        requires Inc<Idx> {
            return pos;
        }

        template<typename I>
        requires PartialOrd<I, Idx> && (!StrongOrd<I, Idx>) && (!WeakOrd<I, Idx>)
        std::partial_ordering operator<=>(const RangeCursor<I>& rhs) const {
            return pos <=> rhs.pos;
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
    requires PartialOrd<Idx, Idx> &&
             basic::CopyAble<Idx>
    struct Range {
        using Item = Idx;

        Range(Idx low, Idx high): low(low), high(high) {}

        Range(const Range&) = default;
        Range& operator=(const Range&) = default;

        template<typename U>
        requires PartialOrd<Idx, U> && PartialOrd<U, Idx>
        bool contains(U&& item) {
            return low <= item && item < high;
        }

        Option<Idx> next() requires Inc<Idx> {
            if (low == high) {
                return Option<Item>::none();
            } else {
                auto value = Option<Item>::some(low);
                low++;
                return value;
            }
        }

        Option<Idx> prev() requires Dec<Idx> {
            if (low == high) {
                return Option<Item>::none();
            } else {
                auto value = Option<Item>::some(high);
                high--;
                return value;
            }
        }

        MSTL_INLINE constexpr
        bool is_empty() {
            // partial order
            return !(low < high);
        }

        RangeCursor<Idx> begin()
        requires Inc<Idx> && Dec<Idx> {
            return { low };
        }

        RangeCursor<Idx> end()
        requires Inc<Idx> && Dec<Idx> {
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
