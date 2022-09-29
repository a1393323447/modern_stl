//
// Created by Shiroan on 2022/9/22.
//

#ifndef MODERN_STL_LINKED_LIST_H
#define MODERN_STL_LINKED_LIST_H

#include <concepts>
#include <global.h>
#include <utility/tuple.h>
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

        // cannot be null
        template<typename ...Args>
        void construct(Args&& ...args) {
            MSTL_DEBUG_ASSERT(data != nullptr, "Trying to construct an object at nullptr.");
            std::construct_at(data, std::forward<Args>(args)...);
        }

        // cannot be null
        void destroy() {
            MSTL_DEBUG_ASSERT(data != nullptr, "Trying to destroy an object at nullptr.");
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

    template<typename T,
            concepts::ForwardNode Node>
    requires (!basic::RefType<T>)
    class ListIter{
        Node* start;        // must be init to head->next
        Node* end;          // must be init to tail

    public:
        using Item = T&;

        ListIter(Node *start, Node *anEnd) : start(start), end(anEnd) {}

        ListIter(const ListIter&) = default;

        Option<Item> next() {
            if (start != end) {
                auto res = Option<T&>::some(*start->data);
                start  = start->next;
                return res;
            } else {
                return Option<T&>::none();
            }
        }

        Option<Item> prev()
        requires concepts::Node<Node> {
            if (end != start) {
                end = end->prev;
                return Option<T&>::some(*end->data);
            } else {
                return Option<T&>::none();
            }
        }
    };

    static_assert(mstl::iter::Iterator<ListIter<int, ForwardListNode<int>>>);
    static_assert(mstl::iter::DoubleEndedIterator<ListIter<int, ListNode<int>>>);

    template<typename T,
            concepts::ForwardNode Node,
            bool Reversed=false>
    requires (!basic::RefType<T>) && (!Reversed || (Reversed && concepts::Node<Node>))
    class ListIterSTL {
        Node* cur;

        template<typename T_,
                concepts::ForwardNode Node_,
                mstl::memory::concepts::Allocator A>
        requires (!basic::RefType<T_>)
        friend class BaseList;

    public:
        using value_type = T;

        ListIterSTL(Node *cur) : cur(cur) {}
        ListIterSTL(const ListIterSTL& r): cur(r.cur) {}

        ListIterSTL& operator=(const ListIterSTL& r) {
            if (&r == this) {
                return *this;
            }
            cur = r.cur;
            return *this;
        }
        operator ListIterSTL<const T, Node> () {
            return ListIterSTL<const T, Node>{cur};
        }

        T& operator*() {
            return *cur->data;
        }

        const T& operator*() const {
            return *cur->data;
        }

        ListIterSTL& operator++() {
            cur = cur->next;
            return *this;
        }

        ListIterSTL operator++(int) {
            auto tmp = *this;
            cur = cur->next;
            return tmp;
        }

        ListIterSTL& operator++() requires Reversed {
            return operator--();
        }

        ListIterSTL operator++(int) requires Reversed {
            return operator--(0);
        }

        ListIterSTL& operator--()
        requires concepts::Node<Node> && Reversed {
            return operator++();
        }

        ListIterSTL operator--(int)
        requires concepts::Node<Node> && Reversed {
            return operator++(0);
        }

        ListIterSTL& operator--()
        requires concepts::Node<Node> {
            cur = cur->prev;
            return *this;
        }

        ListIterSTL operator--(int)
        requires concepts::Node<Node> {
            auto tmp = *this;
            cur = cur->prev;
            return tmp;
        }

        bool operator==(const ListIterSTL &rhs) const {
            return cur == rhs.cur;
        }
    };

    namespace _private {
        template <concepts::ForwardNode Node, typename P>
        requires ops::Predicate<P, typename Node::Item, typename Node::Item>
        utility::Pair<Node*, Node*> merge(Node* a, Node* b, P predicate);

        template <concepts::ForwardNode Node, typename P>
        requires ops::Predicate<P, typename Node::Item, typename Node::Item>
        utility::Pair<Node*, Node*> mergeSort(Node* head, P predicate) {
            if (head == nullptr || head->next == nullptr) {
                return utility::make_pair(head, (Node*)nullptr);
            }

            static auto is_end = [](Node* n) -> bool {                      // 检测n是否为实尾节点
                return (n->next == nullptr || n->next->data == nullptr);
            };

            Node* fast = head;
            Node* slow = head;

            while (!is_end(fast) &&
                   !is_end(fast->next)) {  // after the loop, "slow" will be the mid-node.
                fast = fast->next->next;
                slow = slow->next;
            }
            fast = slow;
            slow = slow->next;
            fast->next = nullptr;
            auto group1 = mergeSort(head, predicate);
            auto group2 = mergeSort(slow, predicate);
            return merge(group1.first(), group2.first(), predicate);
        }

        template <concepts::ForwardNode Node, typename P>
        requires ops::Predicate<P, typename Node::Item, typename Node::Item>
        utility::Pair<Node*, Node*> merge(Node* a, Node* b, P predicate) {
            if (a == nullptr || a->data == nullptr)
                return utility::make_pair(b, (Node*)nullptr);
            if (b == nullptr || b->data == nullptr)
                return utility::make_pair(a, (Node*)nullptr);

            static auto is_end = [](Node* n) -> bool {          // 检测n是否为虚尾节点(含空节点)
                return (n == nullptr || n->data == nullptr);
            };

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

            while (!is_end(a) && !is_end(b)) {
                if (predicate(*a->data, *b->data)) {
                    p->set_next(a);
                    a = a->next;
                } else {
                    p->set_next(b);
                    b = b->next;
                }
                p = p->next;
            }

            if (!is_end(a)) {
                p->set_next(a);
            } else if (!is_end(b)){
                p->set_next(b);
            }
            while (!is_end(p->next)) {
                p = p->next;
            }
            return utility::make_pair(res, p);
        }

        template<typename T, concepts::ForwardNode Node>
        struct NodeAllocHelper {
            Node* next;
            T* data;
            T _d;

        public:
            NodeAllocHelper() = delete;
        };

        template<typename T, concepts::Node Node>
        struct NodeAllocHelper<T, Node> {
            Node* prev;
            Node* next;
            T* data;
            T _d;

        public:
            NodeAllocHelper() = delete;
        };

        template<typename T, concepts::ForwardNode Node>
        static constexpr memory::Layout get_layout() {
            return memory::Layout::from_type<NodeAllocHelper<T, Node>>();
        }

        template<typename T, concepts::ForwardNode Node>
        static constexpr usize get_offset() {
            return (usize)(&((NodeAllocHelper<T, Node>*)0)->_d);
        }
    }

    template<typename T,
            concepts::ForwardNode Node,
            mstl::memory::concepts::Allocator A>
    requires (!basic::RefType<T>)
    class ListIntoIter;

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
        using IntoIter = ListIntoIter<T, Node, A>;
        using iterator = ListIterSTL<T, Node>;
        using const_iterator = ListIterSTL<const T, Node>;

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
            Node* h = build_virtual_node();
            Node* t = build_virtual_node();
            Node* p = h;
            for (usize i = 0; i < count; i++) {
                Node* tmp = construct_node(value);
                p->set_next(tmp);
                p = tmp;
                len++;
            }
            p->set_next(t);
            head = h;
            tail = t;
            tail->set_next(nullptr);
        }

        explicit BaseList(usize count, const A& allocator={})
        requires std::default_initializable<T> : alloc(allocator) {
            if (count == 0) {
                init_empty_list();
                return;
            }
            Node* h = build_virtual_node();
            Node* t = build_virtual_node();
            Node* p = h;
            for (usize i = 0; i < count; i++) {
                Node* tmp = construct_node();
                p->set_next(tmp);
                p = tmp;
                len++;
            }
            p->set_next(t);
            head = h;
            tail = t;
            tail->set_next(nullptr);
        }

        template<iter::LegacyInputIterator InputIt>
        BaseList(InputIt first, InputIt last, const A& allocator=A{})
        requires basic::CopyAble<T> : alloc(allocator) {
            if (first == last) {
                init_empty_list();
                return;
            }

            Node* h = build_virtual_node();
            Node* t = build_virtual_node();
            Node* p = h;
            while (first != last) {
                Node* tmp = construct_node(*first++);
                p->set_next(tmp);
                p = tmp;
                len++;
            }
            p->set_next(t);
            head = h;
            tail = t;
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
                Node* h = build_virtual_node();
                Node* t = build_virtual_node();
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
                p->set_next(t);
                head = h;
                tail = t;
                tail->set_next(nullptr);
                list.destroy_node(list.head);
                list.destroy_node(list.tail);
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
                destroy_node(head);  // and finally release the virtual head node
                destroy_node(tail);  // and the virtual tail node.
            }
        }

        BaseList& operator=(const BaseList& rhs)
        requires basic::CopyAble<T> {
            if (&rhs == this) {
                return *this;
            }
            this->~BaseList();
            alloc = rhs.alloc;
            copy_impl(rhs);
            return *this;
        }

        BaseList& operator=(BaseList&& rhs) noexcept {
            if (&rhs == this) {
                return *this;
            }
            this->~BaseList();
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
            p->set_next(tail);
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

        Option<T&> back() noexcept requires is_double_linked_list {
            if (empty()) {
                return Option<T&>::none();
            } else {
                T* data = tail->prev->data;
                return Option<T&>::some(*data);
            }
        }

        Option<const T&> back() const noexcept requires is_double_linked_list {
            if (empty()) {
                return Option<const T&>::none();
            } else {
                const T* data = tail->prev->data;
                return Option<const T&>::some(*data);
            }
        }
    public:  // iter
        IntoIter into_iter() {
            return IntoIter{std::move(*this)};
        }

        template<iter::Iterator Iter>
        static decltype(auto) from_iter(Iter iter) {
            BaseList res;
            Node* p = res.head;
            auto val = iter.next();
            while (val.is_some()) {
                Node* tmp = res.template construct_node(val.unwrap_uncheck());
                p->set_next(tmp);
                p = tmp;
                val = iter.next();
                res.len++;
            }
            p->set_next(res.tail);
            return res;
        }

        Iter iter() noexcept {
            return {head->next, tail};
        }

        const_iterator iter() const noexcept {
            return cbegin();
        }

        iterator before_begin() noexcept {
            return ListIterSTL<T, Node>(head);
        }

        const_iterator before_begin() const noexcept {
            return cbefore_begin();
        }

        const_iterator cbefore_begin() const noexcept {
            return ListIterSTL<const T, Node>(head);
        }

        iterator begin() noexcept{
            return ListIterSTL<T, Node>(head->next);
        }

        const_iterator begin() const noexcept {
            return cbegin();
        }

        const_iterator cbegin() const noexcept{
            return ListIterSTL<const T, Node>(head->next);
        }

        iterator end() noexcept{
            return ListIterSTL<T, Node>(tail);
        }

        const_iterator end() const noexcept{
            return cend();
        }

        const_iterator cend() const noexcept{
            return ListIterSTL<const T, Node>(tail);
        }

        decltype(auto) rbegin() noexcept
        requires is_double_linked_list {
            return ListIterSTL<T, Node, true>(tail->prev);
        }

        decltype(auto) rbegin() const noexcept
        requires is_double_linked_list {
            return crbegin();
        }

        decltype(auto) crbegin() const noexcept
        requires is_double_linked_list {
            return ListIterSTL<const T, Node, true>(tail->prev);
        }

        decltype(auto) rend() noexcept
        requires is_double_linked_list {
            return ListIterSTL<T, Node, true>(head);
        }

        decltype(auto) rend() const noexcept
        requires is_double_linked_list {
            return crend();
        }

        decltype(auto) crend() const noexcept
        requires is_double_linked_list {
            return ListIterSTL<const T, Node, true>(head);
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
            while (cur != tail) {
                Node* tmp = cur;
                cur = cur->next;

                destroy_node(tmp);
            }
            len = 0;
            head->set_next(tail);
        }

        iterator insert_after(const_iterator pos, const T& val)
        requires basic::CopyAble<T> {
            return emplace_after(pos, val);
        }

        iterator insert_after(const_iterator pos, T&& val)
        {
            return emplace_after(pos, std::forward<T>(val));
        }

        iterator insert_after(const_iterator pos, usize count, const T& val)
        requires basic::CopyAble<T> {
            MSTL_DEBUG_ASSERT(pos != end(), "Trying to emplace element after the tail.");
            if (count == 0) {
                return iterator(pos.cur);
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
            return {r->next};
        }

        template<iter::LegacyInputIterator InputIt>
        iterator insert_after(const_iterator pos, InputIt first, InputIt last) {
            MSTL_DEBUG_ASSERT(pos != end(), "Trying to emplace element after the tail.");
            if (first == last) {
                return {pos.cur};
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
            return {r->next};
        }

        iterator insert_after(const_iterator pos, std::initializer_list<T> ilist) {
            return insert_after(pos, ilist.begin(), ilist.end());
        }

        template<typename ...Args>
        iterator emplace_after(const_iterator pos, Args&& ...args) {
            MSTL_DEBUG_ASSERT(pos != end(), "Trying to emplace element after the tail.");
            Node* tmp = construct_node(std::forward<Args>(args)...);
            tmp->set_next(pos.cur->next);
            pos.cur->set_next(tmp);
            len++;
            return ListIterSTL<T, Node>(tmp);
        }

        iterator erase_after(const_iterator pos) {
            MSTL_DEBUG_ASSERT(pos != end(), "Trying to erase element after the tail.");
            if (pos.cur->next == tail) {
                return end();
            } else {
                Node* tmp = pos.cur->next;
                Node* r = tmp->next;
                pos.cur->set_next(r);
                destroy_node(tmp);
                len--;
                return {r};
            }
        }

        iterator erase_after(const_iterator first, const_iterator last) {
            while (true) {
                if (first == last || first.cur->next == last.cur)
                    break;
                erase_after(first);
            }
            return {last.cur};
        }

        template<class ...Args>
        iterator emplace(const_iterator pos, Args&& ...args)
        requires is_double_linked_list {
            MSTL_DEBUG_ASSERT(pos != before_begin(), "Trying to emplace element before the head.");
            Node* tmp = construct_node(std::forward<Args>(args)...);
            Node* prev = pos.cur->prev;

            prev->set_next(tmp);
            tmp->set_next(pos.cur);
            len++;
            return ListIterSTL<T, Node>(tmp);
        }

        iterator insert(const_iterator pos, const T& val)
        requires basic::CopyAble<T> && is_double_linked_list {
            return emplace(pos, val);
        }

        iterator insert(const_iterator pos, T&& val)
            requires is_double_linked_list {
            return emplace(pos, std::forward<T>(val));
        }

        iterator insert(const_iterator pos, usize count, const T& val)
        requires basic::CopyAble<T> && is_double_linked_list {
            MSTL_DEBUG_ASSERT(pos != before_begin(), "Trying to emplace element before the head.");
            if (count == 0) {
                return iterator(pos.cur);
            }
            Node* p = pos.cur->prev;
            Node* r = p;
            Node* t = pos.cur;
            for (usize i = 0; i < count; i++) {
                Node* tmp = construct_node(val);
                p->set_next(tmp);
                p = tmp;
            }
            len += count;
            p->set_next(t);
            return {r->next};
        }

        template<iter::LegacyInputIterator InputIt>
        iterator insert(const_iterator pos, InputIt first, InputIt last)
        requires is_double_linked_list {
            if (first == last) {
                return {pos.cur};
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
            return {r->next};
        }

        iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
            return insert(pos, ilist.begin(), ilist.end());
        }

        iterator erase(const_iterator pos)
        requires is_double_linked_list {
            if (pos.cur == tail || pos.cur == head) {
                return {pos.cur};
            } else {
                Node* tmp = pos.cur;
                pos.cur->prev->set_next(tmp->next);
                Node* r = tmp->next;
                destroy_node(tmp);
                len--;
                return {r};
            }
        }

        iterator erase(const_iterator first, const_iterator last)
        requires is_double_linked_list {
            if (first == last) {
                return {last.cur};
            }
            if (first == before_begin()) {
                ++first;                // skip the head
            }
            Node* h = first.cur->prev;
            h->set_next(last.cur);
            while (first != last) {
                Node* tmp = first.cur;
                ++first;
                destroy_node(tmp);
                len--;
            }
            return {last.cur};
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
            len++;
            return *node->data;
        }

        void pop_front() noexcept {
            Node* tmp = head->next;
            MSTL_DEBUG_ASSERT(tmp != nullptr, "Trying to pop front at an empty list.");
            head->set_next(tmp->next);
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

            tail->prev->set_next(node);
            node->set_next(tail);

            len++;
            return *node->data;
        }

        void pop_back() noexcept
        requires is_double_linked_list {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            MSTL_DEBUG_ASSERT(!empty(), "Trying to pop back at an empty list.");
            Node* tmp = tail->prev;
            tmp->prev->set_next(tail);
            len--;
            destroy_node(tmp);
        }

        void resize(usize count) noexcept {
            if (count == 0) {
                clear();
            } else if (count == len) {
                return;
            } else if (count > len) {
                extend(count);
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
                extend(count, val);
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

            other.head->set_next(other.tail);
            len += other.len;
            other.len = 0;

            auto r = _private::merge(a, b, p);
            head->set_next(r.first());
            if (Node* t = r.second(); t != nullptr) {
                t->set_next(tail);
            }
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

            len += other.len;
            other.len = 0;
            other.destroy_node(other.head);
            other.destroy_node(other.tail);
            other.head = nullptr;
            other.tail = nullptr;

            auto r = _private::merge(a, b, p);
            head->set_next(r.first());
            if (Node* t = r.second(); t != nullptr) {
                t->set_next(tail);
            }
        }

        void splice_after(const_iterator pos, BaseList& other) noexcept {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = other.real_tail();

            t->set_next(pos.cur->next);
            pos.cur->set_next(h);

            other.head->set_next(other.tail);
            len += other.len;
            other.len = 0;
        }

        void splice(const_iterator pos, BaseList& other) noexcept
        requires is_double_linked_list {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = other.real_tail();

            pos.cur->prev->set_next(h);
            t->set_next(pos.cur);

            other.head->set_next(other.tail);
            len += other.len;
            other.len = 0;
        }

        void splice_after(const_iterator pos, BaseList&& other) noexcept {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = other.real_tail();

            t->set_next(pos.cur->next);
            pos.cur->set_next(h);

            len += other.len;
            other.len = 0;
            other.destroy_node(other.head);
            other.destroy_node(other.tail);
            other.head = other.tail = nullptr;
        }

        void splice(const_iterator pos, BaseList&& other) noexcept
        requires is_double_linked_list {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = other.real_tail();

            pos.cur->prev->set_next(h);
            t->set_next(pos.cur);

            len += other.len;
            other.len = 0;
            other.destroy_node(other.head);
            other.destroy_node(other.tail);
            other.head = other.tail = nullptr;
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

            while (cur != tail) {
                if (predicate(*cur->data)) {
                    last->set_next(cur->next);
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

            Node* p1 = head;
            Node* p2 = p1->next;
            Node* p3 = p2->next;
            Node* h = head;  // the former header
            while (p2 != nullptr) {
                p2->set_next(p1);
                p1 = p2;
                p2 = p3;
                if (p3 != nullptr)
                    p3 = p3->next;
            }
            h->set_next(nullptr);
            std::swap(head, tail);
        }

        void reverse() requires is_double_linked_list {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            Node* p = head;
            Node* ap = head->next;
            while (p != nullptr) {
                std::swap(p->next, p->prev);
                p = ap;
                if (ap != nullptr)
                    ap = ap->next;
            }
            std::swap(head, tail);
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
            while (cur != tail) {
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
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            auto r = _private::mergeSort(head->next, predicate);
            head->set_next(r.first());
            if (Node* t = r.second(); t != nullptr) {
                t->set_next(tail);
            }
        }

    private:
        constexpr static memory::Layout get_node_layout() {
            return memory::Layout::from_type<Node>();
        }

        constexpr static memory::Layout get_layout() {
            return memory::Layout::from_type<T>();
        }

        constexpr static memory::Layout get_data_node_layout() {
            return _private::get_layout<T, Node>();
        }

        constexpr static usize get_data_offset() {
            return _private::get_offset<T, Node>();
        }

        // allocate data node without initialize it
        Node* alloc_node() {
            Node* node = static_cast<Node*>(alloc.allocate(get_node_layout(), 1));
            return node;
        }

        // allocate node and data within a continious space
        Node* alloc_node_with_data() {
            Node* node = (Node*)(alloc.allocate(get_data_node_layout(), 1));
            T* data = (T*)(((u8*)node) + get_data_offset());
            node->data = data;
            return node;
        }

        Node* build_virtual_node() {
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

        void dealloc_node_with_data(Node* node) {
            alloc.deallocate(node, get_data_node_layout(), 1);
        }

        template<typename ...Args>
        Node* construct_node(Args&& ...args) {
            Node* n = alloc_node_with_data();
            n->construct(std::forward<Args>(args)...);
            return n;
        }

        // Destroy data and the node
        void destroy_node(Node* node) {
            if (node->data != nullptr) {
                node->destroy();  // destroy data
                dealloc_node_with_data(node);
            } else {
                dealloc_node(node);
            }

        }

        // DO NOT INVOKE when the list is not empty
        void init_empty_list() {
            Node* h = build_virtual_node();
            head = h;
            Node* t = build_virtual_node();
            tail = t;
            head->set_next(tail);
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
            Node* h = build_virtual_node();
            Node* t = build_virtual_node();
            Node* p = h;

            while (rp != other.tail) {
                Node* tmp = construct_node(*static_cast<const T*>(rp->data));
                rp = rp->next;
                p->set_next(tmp);
                p = tmp;
                len++;
            }
            p->set_next(t);
            head = h;
            tail = t;
            tail->set_next(nullptr);
        }

        void shorten(usize count) {
            Node* t;
            usize d = len - count;
            if constexpr (is_double_linked_list) {
                if (d < len / 2) {  // 如果 d 小于长度的一半, 那么从尾部开始遍历会更快
                    t = tail->prev;
                    for (usize i = 0; i < d; i++) {
                        t = t->prev;
                    }
                } else {
                    t = head;
                    for (usize i = 0; i < count; i++) {
                        t = t->next;
                    }
                }
            } else {
                t = head;
                for (usize i = 0; i < count; i++) {
                    t = t->next;
                }
            }
            Node* p = t->next;
            t->set_next(tail);

            while (p != tail) {
                Node* tmp = p;
                p = p->next;
                destroy_node(tmp);
            }
            len = count;
        }

        void extend(usize count) {
            usize d = count - len;
            Node* p;
            if constexpr (is_double_linked_list) {
                p = tail->prev;
            } else {
                p = head;
                for (usize i = 0; i < len; i++) {
                    p = p->next;
                }
            }
            for (usize i = 0; i < d; i++) {
                Node* tmp = construct_node();
                p->set_next(tmp);
                tmp->set_next(tail);
                p = tmp;
            }
            len = count;
        }

        void extend(usize count, const T& val) {
            usize d = count - len;
            Node* p;
            if constexpr (is_double_linked_list) {
                p = tail->prev;
            } else {
                p = head;
                for (usize i = 0; i < len; i++) {
                    p = p->next;
                }
            }
            for (usize i = 0; i < d; i++) {
                Node* tmp = construct_node(val);
                p->set_next(tmp);
                tmp->set_next(tail);
                p = tmp;
            }
            len = count;
        }

        Node* real_tail() const {
            Node* p = head;
            while (p->next != tail) {
                p = p->next;
            }
            return p;
        }

        Node* real_tail() const requires is_double_linked_list {
            return tail->prev;
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

    template<typename T,
            concepts::ForwardNode Node,
            mstl::memory::concepts::Allocator A>
    requires (!basic::RefType<T>)
    class ListIntoIter {
        BaseList<T, Node, A> list;

    public:
        using Item = T;

        explicit ListIntoIter(BaseList<T, Node, A> &&list) : list(std::forward<BaseList<T, Node, A>>(list)) {}

        Option<Item> next() {
            if (!list.empty()) {
                auto res = Option<T>::some(std::move(list.front().unwrap_uncheck()));
                list.pop_front();
                return res;
            } else {
                return Option<T>::none();
            }
        }

        Option<Item> prev()
        requires concepts::Node<Node> {
            if (!list.empty()) {
                auto res = Option<T>::some(std::move(list.back().unwrap_uncheck()));
                list.pop_back();
                return res;
            } else {
                return Option<T>::none();
            }
        }
    };

    static_assert(mstl::iter::Iterator<ListIntoIter<int, ForwardListNode<int>, memory::allocator::Allocator>>);
    static_assert(mstl::iter::DoubleEndedIterator<ListIntoIter<int, ListNode<int>, memory::allocator::Allocator>>);
}

template<typename T>
struct std::iterator_traits<mstl::collection::ListIterSTL<T, mstl::collection::ForwardListNode<T>>>
{
    typedef forward_iterator_tag       iterator_category;
    typedef T                          value_type;
    typedef mstl::usize                difference_type;
    typedef T*                         pointer;
    typedef T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::ListIterSTL<const T, mstl::collection::ForwardListNode<T>>>
{
    typedef forward_iterator_tag             iterator_category;
    typedef T                                value_type;
    typedef mstl::usize                      difference_type;
    typedef const T*                         pointer;
    typedef const T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::ListIterSTL<T, mstl::collection::ListNode<T>>>
{
    typedef bidirectional_iterator_tag iterator_category;
    typedef T                          value_type;
    typedef mstl::usize                difference_type;
    typedef T*                         pointer;
    typedef T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::ListIterSTL<const T, mstl::collection::ListNode<T>>>
{
    typedef bidirectional_iterator_tag  iterator_category;
    typedef T                           value_type;
    typedef mstl::usize                 difference_type;
    typedef const T*                    pointer;
    typedef const T&                    reference;
};

#endif //MODERN_STL_LINKED_LIST_H
