//
// Created by Shiroan on 2022/9/22.
//

// fixme 目前, 当尾节点变更时, 迭代器将会失效 考虑使用虚尾节点

#ifndef MODERN_STL_LINKED_LIST_H
#define MODERN_STL_LINKED_LIST_H

#include <concepts>
#include <global.h>
#include <option/option.h>
#include <basic_concepts.h>
#include <iter/iterator.h>
#include <memory/allocators/allocator_concept.h>
#include <ops/cmp.h>
#include <ops/callable.h>

namespace mstl::collection {
    namespace concepts {
        template <typename T>
        concept ForwardNode = requires (T node, T* node_ptr){
            { node.next } -> std::convertible_to<T*>;
            *node.data;
            node.set_next(node_ptr);
            typename T::Item;
        };

        template <typename T>
        concept Node = ForwardNode<T> && requires (T node) {
            { node.prev } -> std::convertible_to<T*>;
        };
    }

    template<typename T>
    requires (!basic::RefType<T>)
    struct ForwardListNode {
        using Item = T;

        ForwardListNode* next;
        T* data;

        template<typename ...Args>
        void construct(Args&& ...args) {
            MSTL_DEBUG_ASSERT(data != nullptr, "Trying to construct an object at nullptr.");
            std::construct_at(data, std::forward<Args>(args)...);
        }

        void destroy() {
            if (data != nullptr)
                std::destroy_at(data);
        }

        void set_next(ForwardListNode* n) {
            next = n;
        }
    };

    template<typename T>
    requires (!basic::RefType<T>)
    struct ListNode {
        using Item = T;

        ListNode* prev;
        ListNode* next;
        T* data;

        template<typename ...Args>
        void construct(Args&& ...args) {
            MSTL_DEBUG_ASSERT(data != nullptr, "Trying to construct an object at nullptr.");
            std::construct_at(data, std::forward<Args>(args)...);
        }

        void destroy() {
            if (data != nullptr)
                std::destroy_at(data);
        }

        // set the next node to n, and set the prev node of n to this, if possible
        void set_next(ListNode* n) {
            next = n;
            if (n != nullptr) {
                n->prev = this;
            }
        }
    };
    static_assert(concepts::ForwardNode<ForwardListNode<int>>);
    static_assert(!concepts::Node<ForwardListNode<int>>);
    static_assert(concepts::Node<ListNode<int>>);
//
//    template<typename T,
//            concepts::ForwardNode Node,
//            mstl::memory::concepts::Allocator A>
//    requires (!basic::RefType<T>)
//    class BaseList;

    template<typename T,
            concepts::ForwardNode Node,
            bool Reversed=false>
    requires (!basic::RefType<T>) && (!Reversed || (Reversed && concepts::Node<Node>))
    class ListIter {
        Node* head;
        Node* cur;
        Node* tail;

        template<typename T_,
                concepts::ForwardNode Node_,
                mstl::memory::concepts::Allocator A>
        requires (!basic::RefType<T_>)
        friend class BaseList;

    public:
        using Item = T&;
        using value_type = T;

        ListIter(Node *head, Node *cur, Node* tail) : head(head), cur(cur), tail(tail) {}
        ListIter(const ListIter& r): head(r.head), cur(r.cur), tail(r.tail) {}

        ListIter& operator=(const ListIter& r) {
            if (&r == this) {
                return *this;
            }
            head = r.head;
            cur = r.cur;
            return *this;
        }
        operator ListIter<const T, Node> () {
            return ListIter<const T, Node>{head, cur, tail};
        }

        Option<Item> next() {
            if (cur != nullptr && cur != head) {
                auto res = Option<Item>::some(*cur->data);
                cur = cur->next;
                return res;
            } else {
                return Option<Item>::none();
            }
        }

        T& operator*() {
            return *cur->data;
        }

        const T& operator*() const {
            return *cur->data;
        }

        ListIter& operator++() {
            cur = cur->next;
            return *this;
        }

        ListIter operator++(int) {
            auto tmp = *this;
            cur = cur->next;
            return tmp;
        }

        ListIter& operator++() requires Reversed {
            return operator--();
        }

        ListIter operator++(int) requires Reversed {
            return operator--(0);
        }

        ListIter& operator--()
        requires concepts::Node<Node> && Reversed {
            return operator++();
        }

        ListIter operator--(int)
        requires concepts::Node<Node> && Reversed {
            return operator++(0);
        }

        ListIter& operator--()
        requires concepts::Node<Node> {
            if (cur == nullptr) {
                cur = tail;
            } else {
                cur = cur->prev;
            }
            return *this;
        }

        ListIter operator--(int)
        requires concepts::Node<Node> {
            auto tmp = *this;
            if (cur == nullptr) {
                cur = tail;
            } else {
                cur = cur->prev;
            }
            return tmp;
        }

        bool operator==(const ListIter &rhs) const {
            return head == rhs.head &&
                   cur == rhs.cur;
        }
    };

    namespace _private {
        template <concepts::ForwardNode Node, typename P>
        requires ops::Predicate<P, typename Node::Item, typename Node::Item>
        Node* merge(Node* a, Node* b, P predicate);

        template <concepts::ForwardNode Node, typename P>
        requires ops::Predicate<P, typename Node::Item, typename Node::Item>
        Node* mergeSort(Node* head, P predicate) {
            if (head == nullptr || head->next == nullptr) {
                return head;
            }

            Node* fast = head;
            Node* slow = head;

            while (fast->next != nullptr && fast->next->next != nullptr) {
                fast = fast->next->next;
                slow = slow->next;
            }
            fast = slow;
            slow = slow->next;
            fast->next = nullptr;
            fast = mergeSort(head, predicate);
            slow = mergeSort(slow, predicate);
            return merge(fast, slow, predicate);
        }

        template <concepts::ForwardNode Node, typename P>
        requires ops::Predicate<P, typename Node::Item, typename Node::Item>
        Node* merge(Node* a, Node* b, P predicate) {
            if (a == nullptr)
                return b;
            if (b == nullptr)
                return a;

            Node* res;
            Node* p;

            if (predicate(*a->data, *b->data)) {
                res = a;
                a = a->next;
            } else {
                res = b;
                b = b->next;
            }
            p = res;

            while (a != nullptr && b != nullptr) {
                if (predicate(*a->data, *b->data)) {
                    p->set_next(a);
                    a = a->next;
                } else {
                    p->set_next(b);
                    b = b->next;
                }
                p = p->next;
            }
            if (a != nullptr) {
                p->set_next(a);
            } else if (b != nullptr){
                p->set_next(b);
            }

            return res;
        }
    }

    template<typename T,
            concepts::ForwardNode Node,
            mstl::memory::concepts::Allocator A=mstl::memory::allocator::Allocator>
    requires (!basic::RefType<T>)
    class BaseList {
        static constexpr bool is_double_linked_list = concepts::Node<Node>;

        Node* head = nullptr;  // head node is always an invalid node
        Node* tail = nullptr;
        usize len = 0;
        A alloc;

    public:
        using Item = T;
        using Pointer = T*;
        using Reference = T&;
        using Iter = ListIter<T, Node>;
        using ConstIter = ListIter<const T, Node>;

        BaseList() noexcept requires std::default_initializable<A>
        : alloc{} {
            init_empty_list();
        }

        explicit BaseList(const A& a) noexcept requires basic::CopyAble<A>
        : alloc(a) {
            init_empty_list();
        }

        BaseList(usize count, const T& value, const A& allocator={})
        requires basic::CopyAble<T> : alloc(allocator) {
            if (count == 0) {
                init_empty_list();
                return;
            }
            Node* h = build_virtual_head_node();
            Node* p = h;
            for (usize i = 0; i < count; i++) {
                Node* tmp = construct_node(value);
                p->set_next(tmp);
                p = tmp;
                len++;
            }

            head = h;
            tail = p;
            tail->set_next(nullptr);
        }

        explicit BaseList(usize count, const A& allocator={})
        requires std::default_initializable<T> : alloc(allocator) {
            if (count == 0) {
                init_empty_list();
                return;
            }
            Node* h = build_virtual_head_node();
            Node* p = h;
            for (usize i = 0; i < count; i++) {
                Node* tmp = construct_node();
                p->set_next(tmp);
                p = tmp;
                len++;
            }

            head = h;
            tail = p;
            tail->set_next(nullptr);
        }

        template<iter::LegacyInputIterator InputIt>
        BaseList(InputIt first, InputIt last, const A& allocator=A{})
        requires basic::CopyAble<T> : alloc(allocator) {
            if (first == last) {
                init_empty_list();
                return;
            }

            Node* h = build_virtual_head_node();
            Node* p = h;
            while (first != last) {
                Node* tmp = construct_node(*first++);
                p->set_next(tmp);
                p = tmp;
                len++;
            }

            head = h;
            tail = p;
            tail->set_next(nullptr);
        }

        BaseList(BaseList&& list) noexcept
                : head(list.head), tail(list.tail), len(list.len), alloc(std::move(list.alloc)) {
            list.head = nullptr;
            list.tail = nullptr;
            list.len = 0;
        }

        BaseList(BaseList&& list, const A& allocator) noexcept {
            MSTL_DEBUG_ASSERT(list.head != nullptr, "The list has been moved.");
            alloc = allocator;
            if (allocator == list.alloc) {
                head = list.head;
                tail = list.tail;
                len = list.len;
            } else {
                len = 0;
                if (list.empty()) {
                    init_empty_list();
                    list.destroy_node(list.head);
                    list.head = nullptr;
                    list.tail = nullptr;
                    list.len = 0;
                    return;
                }

                Node* rp = list.head->next;
                Node* h = build_virtual_head_node();
                Node* p = h;

                while (rp != nullptr) {
                    Node* n = construct_node(std::move(*rp->data));  // new node
                    Node* tmp = rp;
                    rp = rp->next;
                    p->set_next(n);
                    p = n;
                    len++;
                    list.destroy_node(tmp);
                }

                head = h;
                tail = p;
                p->set_next(nullptr);
                list.destroy_node(list.head);
            }

            list.head = nullptr;
            list.tail = nullptr;
            list.len = 0;
        }

        BaseList(const BaseList& o)
        requires basic::CopyAble<T>: alloc(o.alloc) {
            copy_impl(o);
        }

        BaseList(const BaseList& o, const A& allocator)
        requires basic::CopyAble<T>: alloc(allocator) {
            copy_impl(o);
        }

        BaseList(std::initializer_list<T> init, const A& allocator=A{}): BaseList(init.begin(), init.end(), allocator) {}

        ~BaseList() {
            if (head == nullptr) {  // head should not be nullptr
                return;             // unless the list has been moved
            } else {
                clear();              // clear all elements int the list
                destroy_node(head);  // and finally release the virtual head node.
            }
        }

        BaseList& operator=(const BaseList& rhs)
        requires basic::CopyAble<T> {
            if (&rhs == this) {
                return *this;
            }
            clear();
            destroy_node(head);
            alloc = rhs.alloc;
            copy_impl(rhs);
            return *this;
        }

        BaseList& operator=(BaseList&& rhs) noexcept {
            if (&rhs == this) {
                return *this;
            }
            clear();
            destroy_node(head);
            alloc = std::move(rhs.alloc);
            head = rhs.head;
            rhs.head = nullptr;
            tail = rhs.tail;
            rhs.tail = nullptr;
            len = rhs.len;
            rhs.len = 0;
            return *this;
        }

        BaseList& operator=(std::initializer_list<T> list) {
            clear();
            Node* h = head;
            Node* p = h;
            auto first = list.begin();
            auto last = list.end();
            while (first != last) {
                Node* tmp = construct_node(*first++);
                p->set_next(tmp);
                p = tmp;
                len++;
            }

            tail = p;
            tail->set_next(nullptr);
            return *this;
        }

        A get_allocator() const noexcept {
            return alloc;
        }

        bool moved() const noexcept {
            return head == nullptr;
        }

    public:  // visitor
        Option<T&> front() noexcept {
            if (empty()) {
                return Option<T&>::none();
            } else {
                T* data = head->next->data;
                return Option<T&>::some(*data);
            }
        }

        Option<const T&> front() const noexcept {
            if (empty()) {
                return Option<const T&>::none();
            } else {
                const T* data = head->next->data;
                return Option<const T&>::some(*data);
            }
        }

        Option<T&> back() noexcept {
            if (empty()) {
                return Option<T&>::none();
            } else {
                T* data = tail->data;
                return Option<T&>::some(*data);
            }
        }

        Option<const T&> back() const noexcept {
            if (empty()) {
                return Option<const T&>::none();
            } else {
                const T* data = tail->data;
                return Option<const T&>::some(*data);
            }
        }
    public:  // iter
        Iter iter() noexcept {
            return begin();
        }

        ConstIter iter() const noexcept {
            return cbegin();
        }

        Iter before_begin() noexcept {
            return ListIter<T, Node>(head, head, tail);
        }

        ConstIter before_begin() const noexcept {
            return cbefore_begin();
        }

        ConstIter cbefore_begin() const noexcept {
            return ListIter<const T, Node>(head, head, tail);
        }

        Iter begin() noexcept{
            return ListIter<T, Node>(head, head->next, tail);
        }

        ConstIter begin() const noexcept {
            return cbegin();
        }

        ConstIter cbegin() const noexcept{
            return ListIter<const T, Node>(head, head->next, tail);
        }

        Iter end() noexcept{
            return ListIter<T, Node>(head, nullptr, tail);
        }

        ConstIter end() const noexcept{
            return cend();
        }

        ConstIter cend() const noexcept{
            return ListIter<const T, Node>(head, nullptr, tail);
        }

        decltype(auto) rbegin() noexcept
        requires is_double_linked_list {
            return ListIter<T, Node, true>(head, tail, tail);
        }

        decltype(auto) rbegin() const noexcept
        requires is_double_linked_list {
            return crbegin();
        }

        decltype(auto) crbegin() const noexcept
        requires is_double_linked_list {
            return ListIter<const T, Node, true>(head, tail, tail);
        }

        decltype(auto) rend() noexcept
        requires is_double_linked_list {
            return ListIter<T, Node, true>(head, head, tail);
        }

        decltype(auto) rend() const noexcept
        requires is_double_linked_list {
            return crend();
        }

        decltype(auto) crend() const noexcept
        requires is_double_linked_list {
            return ListIter<const T, Node, true>(head, head, tail);
        }

    public:  // capacity
        inline bool empty() const noexcept {
            return len == 0;
        }

        inline usize size() const noexcept {
            return len;
        }

    public:  // editors
        void clear() noexcept {
            Node* cur = head->next;
            while (cur != nullptr) {
                Node* tmp = cur;
                cur = cur->next;

                destroy_node(tmp);
            }
            len = 0;
            head->set_next(nullptr);
            tail = head;
        }

        Iter insert_after(ConstIter pos, const T& val)
        requires basic::CopyAble<T> {
            return emplace_after(pos, val);
        }

        Iter insert_after(ConstIter pos, T&& val)
        {
            return emplace_after(pos, std::forward<T>(val));
        }

        Iter insert_after(ConstIter pos, usize count, const T& val)
        requires basic::CopyAble<T> {
            if (count == 0) {
                return Iter(pos.head, pos.cur, pos.tail);
            }
            Node* p = pos.cur;
            Node* r = p;
            Node* t = pos.cur->next;
            for (usize i = 0; i < count; i++) {
                Node* tmp = construct_node(val);
                p->set_next(tmp);
                p = tmp;
            }
            len += count;
            p->set_next(t);
            if (t == nullptr) {
                tail = p;
            }
            return {head, r->next, tail};
        }

        template<iter::LegacyInputIterator InputIt>
        Iter insert_after(ConstIter pos, InputIt first, InputIt last) {
            if (first == last) {
                return {pos.head, pos.cur, pos.tail};
            }
            Node* p = pos.cur;
            Node* r = p;
            Node* t = pos.cur->next;
            while (first != last) {
                Node* tmp = construct_node(*first);
                ++first;
                len++;
                p->set_next(tmp);
                p = tmp;
            }
            p->set_next(t);
            if (t == nullptr) {
                tail = p;
            }
            return {head, r->next, tail};
        }

        Iter insert_after(ConstIter pos, std::initializer_list<T> ilist) {
            return insert_after(pos, ilist.begin(), ilist.end());
        }

        template<typename ...Args>
        Iter emplace_after(ConstIter pos, Args&& ...args) {
            Node* tmp = construct_node(std::forward<Args>(args)...);
            tmp->set_next(pos.cur->next);
            pos.cur->set_next(tmp);
            len++;
            if (tmp->next == nullptr) {
                tail = tmp;
            }
            return ListIter<T, Node>(head, tmp, tail);
        }

        Iter erase_after(ConstIter pos) {
            if (pos.cur->next == nullptr) {
                return {pos.head, pos.cur, pos.tail};
            } else {
                Node* tmp = pos.cur->next;
                pos.cur->set_next(tmp->next);
                if (tail == tmp) {
                    tail = (pos.cur != nullptr) ? pos.cur : head;
                }
                destroy_node(tmp);
                len--;
                return {pos.head, pos.cur->next, pos.tail};
            }
        }

        Iter erase_after(ConstIter first, ConstIter last) {
            while (true) {
                if (first == last || first.cur->next == last.cur)
                    break;
                erase_after(first);
            }
            return {last.head, last.cur, last.tail};
        }

        template<class ...Args>
        Iter emplace(ConstIter pos, Args&& ...args)
        requires is_double_linked_list {
            Node* tmp = construct_node(std::forward<Args>(args)...);
            Node* prev = pos.cur->prev;

            prev->set_next(tmp);
            tmp->set_next(pos.cur);
            len++;
            return ListIter<T, Node>(head, tmp, tail);
        }

        Iter insert(ConstIter pos, const T& val)
        requires basic::CopyAble<T> && is_double_linked_list {
            return emplace(pos, val);
        }

        Iter insert(ConstIter pos, T&& val)
            requires is_double_linked_list {
            return emplace(pos, std::forward<T>(val));
        }

        Iter insert(ConstIter pos, usize count, const T& val)
        requires basic::CopyAble<T> && is_double_linked_list {
            if (count == 0) {
                return Iter(pos.head, pos.cur, pos.tail);
            }
            Node* p = pos.cur->prev;
            Node* t = pos.cur;
            for (usize i = 0; i < count; i++) {
                Node* tmp = construct_node(val);
                p->set_next(tmp);
                p = tmp;
            }
            len += count;
            p->set_next(t);
            return {head, p, tail};
        }

        template<iter::LegacyInputIterator InputIt>
        Iter insert(ConstIter pos, InputIt first, InputIt last)
        requires is_double_linked_list {
            if (first == last) {
                return {pos.head, pos.cur, pos.tail};
            }
            Node* p = pos.cur->prev;
            Node* r = p;
            Node* t = pos.cur;
            while (first != last) {
                Node* tmp = construct_node(*first);
                ++first;
                len++;
                p->set_next(tmp);
                p = tmp;
            }
            p->set_next(t);
            return {head, r->next, tail};
        }

        Iter insert(ConstIter pos, std::initializer_list<T> ilist) {
            return insert(pos, ilist.begin(), ilist.end());
        }

        Iter erase(ConstIter pos)
        requires is_double_linked_list {
            if (pos.cur == nullptr || pos.cur == head) {
                return {pos.head, pos.cur, pos.tail};
            } else {
                Node* tmp = pos.cur;
                pos.cur->prev->set_next(tmp->next);
                Node* r = tmp->next;
                destroy_node(tmp);
                len--;
                return {pos.head, r, pos.tail};
            }
        }

        Iter erase(ConstIter first, ConstIter last)
        requires is_double_linked_list {
            if (first == last) {
                return {last.head, last.cur, last.tail};
            }
            Node* h = first.cur->prev;
            h->set_next(last.cur);
            while (first != last) {
                Node* tmp = first.cur;
                ++first;
                destroy_node(tmp);
                len--;
            }
            return {last.head, last.cur, last.tail};
        }


        void push_front(T&& value) noexcept
        {
            emplace_front(std::forward<T>(value));
        }

        void push_front(const T& value) noexcept
        requires basic::CopyAble<T> {
            emplace_front(value);
        }

        template<typename ...Args>
        Reference emplace_front(Args&& ...args) noexcept {
            Node* node = construct_node(std::forward<Args>(args)...);

            node->set_next(head->next);
            head->set_next(node);
            if (tail == head) {
                tail = node;
            }

            len++;
            return *node->data;
        }

        void pop_front() noexcept {
            Node* tmp = head->next;
            MSTL_DEBUG_ASSERT(tmp != nullptr, "Trying to pop front at an empty list.");
            head->set_next(tmp->next);
            if (tail == tmp) {
                tail = head;
            }
            destroy_node(tmp);
            len--;
        }

        void push_back(T&& val) noexcept
        requires is_double_linked_list {
            emplace_back(std::forward<T>(val));
        }

        void push_back(const T& val) noexcept
        requires basic::CopyAble<T> && is_double_linked_list {
            emplace_back(val);
        }

        template<typename ...Args>
        Reference emplace_back(Args... args) noexcept
        requires is_double_linked_list {
            Node* node = construct_node(std::forward<Args>(args)...);

            node->set_next(nullptr);
            tail->set_next(node);
            tail = node;

            len++;
            return *node->data;
        }

        void pop_back() noexcept
        requires is_double_linked_list {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            MSTL_DEBUG_ASSERT(!empty(), "Trying to pop back at an empty list.");
            Node* tmp = tail;
            tmp->prev->set_next(nullptr);
            tail = tmp->prev;
            len--;
            destroy_node(tmp);
        }

        void resize(usize count) noexcept {
            if (count == 0) {
                clear();
            } else if (count == len) {
                return;
            } else if (count > len) {
                lengthen(count);
            } else {
                shorten(count);
            }
        }

        void resize(usize count, const T& val) noexcept {
            if (count == 0) {
                clear();
            } else if (count == len) {
                return;
            } else if (count > len) {
                lengthen(count, val);
            } else {
                shorten(count);
            }
        }

        void swap(BaseList& list) noexcept {
            std::swap(alloc, list.alloc);
            std::swap(head, list.head);
            std::swap(tail, list.tail);
            std::swap(len, list.len);
        }

    public:  // Operations
        void merge(BaseList& other) {
            merge(other, std::less<T>{});
        }

        template<class Compare>
        void merge(BaseList& other, Compare p)
        requires ops::Predicate<Compare, T, T> {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            MSTL_DEBUG_ASSERT(other.head != nullptr, "The list has been moved.");

            if (&other == this || other.empty()) {
                return;
            }

            Node* a = head->next;
            Node* b = other.head->next;

            Node* tailA = tail;
            Node* tailB = other.tail;
            Node* tail_;
            if (tailA == head || p(*tailA->data, *tailB->data)) {
                tail_ = tailB;
            } else {
                tail_ = tailA;
            }

            other.head->set_next(nullptr);
            len += other.len;
            other.len = 0;
            other.tail = other.head;

            Node* r = _private::merge(a, b, p);
            head->set_next(r);
            tail = tail_;
        }

        void merge(BaseList&& other) {
            merge(std::forward<BaseList>(other), std::less<T>{});
        }

        template<class Compare>
        void merge(BaseList&& other, Compare p)
        requires ops::Predicate<Compare, T, T> {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            MSTL_DEBUG_ASSERT(other.head != nullptr, "The list has been moved.");

            if (&other == this || other.empty()) {
                return;
            }

            Node* a = head->next;
            Node* b = other.head->next;

            Node* tailA = tail;
            Node* tailB = other.tail;
            Node* tail_;
            if (tailA == head || p(tailA, tailB)) {
                tail_ = tailB;
            } else {
                tail_ = tailA;
            }

            len += other.len;
            other.len = 0;
            other.destroy_node(other.head);
            other.head = nullptr;
            other.tail = nullptr;

            Node* r = _private::merge(a, b, p);
            head->set_next(r);
            tail = tail_;
        }

        // todo splice_after
        void splice_after(ConstIter pos, BaseList& other) noexcept {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = pos.cur->next;
            pos.cur->set_next(h);
            if (t == nullptr) {
                tail = other.tail;
            } else {
                other.tail->set_next(t);
            }
            len += other.len;
            other.head->set_next(nullptr);
            other.tail = head;
            other.len = 0;
        }

        void splice(ConstIter pos, BaseList& other) noexcept
        requires is_double_linked_list {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = pos.cur;
            pos.cur->prev->set_next(h);
            if (t == nullptr) {
                tail = other.tail;
            } else {
                other.tail->set_next(t);
            }
            len += other.len;
            other.head->set_next(nullptr);
            other.tail = head;
            other.len = 0;
        }

        void splice_after(ConstIter pos, BaseList&& other) noexcept {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = pos.cur->next;
            pos.cur->set_next(h);
            if (t == nullptr) {
                tail = other.tail;
            } else {
                other.tail->set_next(t);
            }
            len += other.len;
            other.destroy_node(other.head);
            other.head = other.tail = nullptr;
            other.len = 0;
        }

        void splice(ConstIter pos, BaseList&& other) noexcept
        requires is_double_linked_list {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = pos.cur;
            pos.cur->prev->set_next(h);
            if (t == nullptr) {
                tail = other.tail;
            } else {
                other.tail->set_next(t);
            }
            len += other.len;
            other.destroy_node(other.head);
            other.head = other.tail = nullptr;
            other.len = 0;
        }

        usize remove(const T& val) noexcept requires ops::Eq<T, T> {
            return remove_if([&](const T& val1) {
                return val == val1;
            });
        }

        template<typename P>
        usize remove_if(P predicate) noexcept
        requires ops::Predicate<P, T>{
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            if (empty()) {
                return 0;
            }

            usize cnt = 0;
            Node* cur = head->next;
            Node* last = head;

            while (cur != nullptr) {
                if (predicate(*cur->data)) {
                    last->set_next(cur->next);
                    if (cur == tail) {  // dropping the tail node
                        tail = last;
                    }
                    destroy_node(cur);  // drop the node
                    len--;
                    cnt++;
                    cur = last->next;

                } else {
                    cur = cur->next;
                    last = last->next;
                }
            }
            return cnt;
        }

        void reverse() requires (!is_double_linked_list) {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            if (len < 2) {
                return;
            }

            Node* p1 = head->next;
            Node* p2 = p1->next;
            Node* p3 = p2->next;
            Node* h = head->next;
            Node* t = tail;
            while (p2 != nullptr) {
                p2->set_next(p1);
                p1 = p2;
                p2 = p3;
                if (p3 != nullptr)
                    p3 = p3->next;
            }

            head->set_next(t);
            tail = h;
            h->set_next(nullptr);
        }

        void reverse() requires is_double_linked_list {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            Node* h = head->next;
            Node* bp = head;
            Node* p = h;
            while (p != nullptr) {
                bp = p;
                p = p->next;
                std::swap(bp->next, bp->prev);
            }
            head->set_next(bp);
            tail = h;
            h->set_next(nullptr);
        }

        usize unique() requires ops::Eq<T, T> {
            return unique(std::equal_to<T>());
        }

        template<class BinaryPredicate>
        usize unique(BinaryPredicate p) requires ops::Predicate<BinaryPredicate, const T&, const T&> {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            if (len < 2) {
                return 0;
            }
            usize cnt = 0;
            Node* cur = head->next->next;
            Node* before_cur = head->next;
            Node* last = head->next;
            while (cur != nullptr) {
                if (p(*cur->data, *last->data)) {
                    before_cur->set_next(cur->next);
                    destroy_node(cur);  // drop the node
                    len--;
                    cnt++;
                    cur = before_cur->next;
                } else {
                    last = cur;
                    before_cur = cur;
                    cur = cur->next;
                }
            }
            return cnt;
        }

        void sort() {
            sort(std::less<T>());
        }

        template<typename P>
        void sort(P predicate)
        requires ops::Predicate<P, T, T> {
            // Must I use this shit?
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            Node* h = _private::mergeSort(head->next, predicate);
            head->set_next(h);
            reset_tail();
        }

    private:
        constexpr static memory::Layout get_node_layout() {
            return memory::Layout::from_type<Node>();
        }

        constexpr static memory::Layout get_layout() {
            return memory::Layout::from_type<T>();
        }

        // allocate data node without initialize it
        Node* alloc_node() {
            Node* node = static_cast<Node*>(alloc.allocate(get_node_layout(), 1));
            return node;
        }

        Node* build_virtual_head_node() {
            Node* r = alloc_node();
            r->next = nullptr;
            r->data = nullptr;
            if constexpr (is_double_linked_list) {
                r->prev = nullptr;
            }
            return r;
        }

        // deallocate the node without destroy it.
        void dealloc_node(Node* node) {
            alloc.deallocate(node, get_node_layout(), 1);
        }

        T* alloc_data() {
            T* data = static_cast<T*>(alloc.allocate(get_layout(), 1));
            return data;
        }

        void dealloc_data(T* data) {
            alloc.deallocate(data, get_layout(), 1);
        }

        template<typename ...Args>
        Node* construct_node(Args&& ...args) {
            Node* n = alloc_node();
            n->data = alloc_data();
            n->construct(std::forward<Args>(args)...);
            return n;
        }

        // Destroy data and the node
        void destroy_node(Node* node) {
            node->destroy();           // destroy data that the node holds
            dealloc_data(node->data);  // deallocate the memory of the data
            dealloc_node(node);        // deallocate the memory of the node
        }

        // DO NOT INVOKE when the list is not empty
        void init_empty_list() {
            Node* node = build_virtual_head_node();
            head = node;
            tail = node;
        }

        void copy_impl(const BaseList& other)
        requires basic::CopyAble<T>{
            MSTL_DEBUG_ASSERT(other.head != nullptr, "The list has been moved.");
            len = 0;

            if (other.empty()) {
                init_empty_list();
                return;
            }

            Node* rp = other.head->next;
            Node* h = build_virtual_head_node();
            Node* p = h;

            while (rp != nullptr) {
                Node* tmp = construct_node(*static_cast<const T*>(rp->data));
                rp = rp->next;
                p->set_next(tmp);
                p = tmp;
                len++;
            }

            head = h;
            tail = p;
            p->set_next(nullptr);
        }

        void reset_tail() {
            // todo Improve
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            Node* p = head;
            while (p->next != nullptr) {
                p = p->next;
            }
            tail = p;
        }

        void shorten(usize count) {
            Node* t;
            usize d = len - count;
            if constexpr (is_double_linked_list) {
                t = tail;
                for (usize i = 0; i < d; i++) {
                    t = t->prev;
                }
            } else {
                t = head;
                for (usize i = 0; i < count; i++) {
                    t = t->next;
                }
            }
            tail = t;
            t = t->next;
            tail->set_next(nullptr);

            while (t != nullptr) {
                Node* tmp = t;
                t = t->next;
                destroy_node(tmp);
            }
            len = count;
        }

        void lengthen(usize count) {
            usize d = count - len;
            Node* p = tail;
            for (usize i = 0; i < d; i++) {
                Node* tmp = construct_node();
                p->set_next(tmp);
                p = tmp;
            }
            tail = p;
            p->set_next(nullptr);
            len =  count;
        }

        void lengthen(usize count, const T& val) {
            usize d = count - len;
            Node* p = tail;
            for (usize i = 0; i < d; i++) {
                Node* tmp = construct_node(val);
                p->set_next(tmp);
                p = tmp;
            }
            tail = p;
            p->set_next(nullptr);
            len =  count;
        }
    };

    template <typename T, memory::concepts::Allocator A=memory::allocator::Allocator>
    using List = BaseList<T, ListNode<T>, A>;

    template <typename T, memory::concepts::Allocator A=memory::allocator::Allocator>
    using ForwardList = BaseList<T, ForwardListNode<T>, A>;

    template <basic::Printable T, memory::concepts::Allocator A>
    std::ostream &operator<<(std::ostream& os, const List<T, A>& list) {
        os << "List [";
        usize size = list.size();
        usize i = 0;
        auto lo = list.begin(), hi = list.end();
        while (lo != hi) {
            os << *lo;
            if (i != size - 1) {
                os << ", ";
            }
            lo++;
            i++;
        }
        os << "]";
        return os;
    }

    template <basic::Printable T, memory::concepts::Allocator A>
    std::ostream &operator<<(std::ostream& os, const ForwardList<T, A>& list) {
        os << "ForwardList [";
        usize size = list.size();
        usize i = 0;
        auto lo = list.begin(), hi = list.end();
        while (lo != hi) {
            os << *lo;
            if (i != size - 1) {
                os << ", ";
            }
            lo++;
            i++;
        }
        os << "]";
        return os;
    }
}

template<typename T>
struct std::iterator_traits<mstl::collection::ListIter<T, mstl::collection::ForwardListNode<T>>>
{
    typedef forward_iterator_tag       iterator_category;
    typedef T                          value_type;
    typedef void                       difference_type;
    typedef T*                         pointer;
    typedef T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::ListIter<const T, mstl::collection::ForwardListNode<T>>>
{
    typedef forward_iterator_tag             iterator_category;
    typedef T                                value_type;
    typedef void                             difference_type;
    typedef const T*                         pointer;
    typedef const T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::ListIter<T, mstl::collection::ListNode<T>>>
{
    typedef bidirectional_iterator_tag iterator_category;
    typedef T                          value_type;
    typedef mstl::usize                difference_type;
    typedef T*                         pointer;
    typedef T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::ListIter<const T, mstl::collection::ListNode<T>>>
{
    typedef bidirectional_iterator_tag  iterator_category;
    typedef T                           value_type;
    typedef mstl::usize                 difference_type;
    typedef const T*                    pointer;
    typedef const T&                    reference;
};

#endif //MODERN_STL_LINKED_LIST_H
