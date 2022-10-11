//
// Created by Shiroan on 2022/9/22.
//

#ifndef MODERN_STL_LINKED_LIST_H
#define MODERN_STL_LINKED_LIST_H

#include <concepts>
#include <mstl/global.h>
#include <mstl/utility/tuple.h>
#include <mstl/option/option.h>
#include <mstl/basic_concepts.h>
#include <mstl/iter/iterator.h>
#include <mstl/memory/allocators/allocator_concept.h>
#include <mstl/ops/cmp.h>
#include <mstl/ops/callable.h>

namespace mstl::collection {
    namespace concepts {
        /**
         * @brief 单向链表节点.
         * # typename 标注要求
         * - Item: 节点中所储存的元素
         *      ```cpp
         *      class FooForwordNode {
         *      public:
         *          using Item = Foo;
         *          ...
         *      };
         *      ```
         * # 成员变量要求
         * - next
         *      - 类型要求
         *
         *          指向节点本身的指针.
         *
         *      - 功能描述
         *
         *          指向下一个节点.
         *
         * - data
         *      - 类型要求
         *
         *          指向Item的指针.
         *
         *      - 功能描述
         *
         *          指向节点本身所存的数据.
         *
         * # 成员函数要求
         * - set_next(node)
         *      - 参数要求
         *          - node
         *
         *              指向节点的指针.
         *
         *      - 功能描述
         *
         *          设置当前节点的下一个节点. 若节点为双向节点, 则当下一个节点不为nullptr时, 设置其上一个节点为当前节点,
         */
        template <typename T>
        concept ForwardNode = requires (T node, T* node_ptr){
            typename T::Item;
            { node.next }  -> std::convertible_to<T*>;
            { *node.data } -> std::convertible_to<typename T::Item&>;
            node.set_next(node_ptr);
        };

        /**
         * 双向迭代器
         * # 前提
         * 满足 \link mstl::collection::concepts::ForwardNode ForwardNode \endlink
         * # 成员变量要求
         * - prev
         *      - 类型要求
         *
         *          指向节点的指针
         *
         *      - 功能描述
         *
         *          指向当前节点的上一个节点.
         */
        template <typename T>
        concept Node = ForwardNode<T> && requires (T node) {
            { node.prev } -> std::convertible_to<T*>;
        };
    }

    /**
     * @brief 链表的MSTL风格迭代器.
     * @tparam T 迭代类型
     * @tparam Node 节点类型
     */
    template<typename T,
            concepts::ForwardNode Node>
    requires (!basic::RefType<T>)
    class ListIter{
        Node* start;        // must be init to head->next
        Node* end;          // must be init to tail

    public:
        using Item = T&;

        constexpr ListIter(Node *start, Node *anEnd) : start(start), end(anEnd) {}

        constexpr ListIter(const ListIter&) = default;

        constexpr Option<Item> next() {
            if (start != end) {
                auto res = Option<T&>::some(*start->data);
                start  = start->next;
                return res;
            } else {
                return Option<T&>::none();
            }
        }

        constexpr Option<Item> prev()
        requires concepts::Node<Node> {
            if (end != start) {
                end = end->prev;
                return Option<T&>::some(*end->data);
            } else {
                return Option<T&>::none();
            }
        }
    };

    /**
     * @brief 链表的STL风格迭代器
     *
     * ## 约束
     * 若Node不满足`Node`, 则Reversed不能为true.
     *
     * @tparam T 迭代的元素类型
     * @tparam Node 节点类型
     * @tparam Reversed 标记迭代器是否为逆序
     */
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

        constexpr ListIterSTL(Node *cur) : cur(cur) {}
        constexpr ListIterSTL(const ListIterSTL& r): cur(r.cur) {}

        constexpr ListIterSTL& operator=(const ListIterSTL& r) {
            if (&r == this) {
                return *this;
            }
            cur = r.cur;
            return *this;
        }

        constexpr operator ListIterSTL<const T, Node, Reversed> () {
            return ListIterSTL<const T, Node, Reversed>{cur};
        }

        constexpr T& operator*() {
            return *cur->data;
        }

        constexpr const T& operator*() const {
            return *cur->data;
        }

        constexpr ListIterSTL& operator++() {
            cur = cur->next;
            return *this;
        }

        constexpr ListIterSTL operator++(int) {
            auto tmp = *this;
            cur = cur->next;
            return tmp;
        }

        constexpr ListIterSTL& operator++() requires Reversed {
            return operator--();
        }

        constexpr ListIterSTL operator++(int) requires Reversed {
            return operator--(0);
        }

        constexpr ListIterSTL& operator--()
        requires concepts::Node<Node> && Reversed {
            return operator++();
        }

        constexpr ListIterSTL operator--(int)
        requires concepts::Node<Node> && Reversed {
            return operator++(0);
        }

        constexpr ListIterSTL& operator--()
        requires concepts::Node<Node> {
            cur = cur->prev;
            return *this;
        }

        constexpr ListIterSTL operator--(int)
        requires concepts::Node<Node> {
            auto tmp = *this;
            cur = cur->prev;
            return tmp;
        }

        constexpr bool operator==(const ListIterSTL &rhs) const {
            return cur == rhs.cur;
        }
    };

    namespace _private {
        template <concepts::ForwardNode Node, typename P>
        requires ops::Predicate<P, typename Node::Item, typename Node::Item>
        constexpr utility::Pair<Node*, Node*> merge(Node* a, Node* b, P predicate);

        template <concepts::ForwardNode Node, typename P>
        requires ops::Predicate<P, typename Node::Item, typename Node::Item>
        constexpr utility::Pair<Node*, Node*> mergeSort(Node* head, P predicate) {
            if (head == nullptr || head->next == nullptr) {
                return utility::make_pair(head, (Node*)nullptr);
            }

            auto is_end = [](Node* n) -> bool {                      // 检测n是否为实尾节点
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
        constexpr utility::Pair<Node*, Node*> merge(Node* a, Node* b, P predicate) {
            if (a == nullptr || a->data == nullptr)
                return utility::make_pair(b, (Node*)nullptr);
            if (b == nullptr || b->data == nullptr)
                return utility::make_pair(a, (Node*)nullptr);

            auto is_end = [](Node* n) -> bool {          // 检测n是否为虚尾节点(含空节点)
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

        /**
         * @brief 单链表节点.
         * @tparam T 节点储存的元素类型.
         */
        template<typename T>
        requires (!basic::RefType<T>)
        struct ForwardListNode {
            using Item = T;

            ForwardListNode* next;
            T* data;

            /// 在节点处构造一个元素. 若节点的data为nullptr, 且当前为DEBUG模式, 则引发panic, 否则, 行为未定义.
            template<typename ...Args>
            constexpr void construct(Args&& ...args) {
                MSTL_DEBUG_ASSERT(data != nullptr, "Trying to construct an object at nullptr.");
                std::construct_at(data, std::forward<Args>(args)...);
            }

            /// 在节点处销毁一个元素. 若节点的data为nullptr, 且当前为DEBUG模式, 则引发panic, 否则, 行为未定义.
            constexpr void destroy() {
                MSTL_DEBUG_ASSERT(data != nullptr, "Trying to destroy an object at nullptr.");
                std::destroy_at(data);
            }

            constexpr void set_next(ForwardListNode* n) {
                next = n;
            }
        };

        /**
         * @brief 双链表的节点
         * @tparam T 节点储存的元素的类型
         */
        template<typename T>
        requires (!basic::RefType<T>)
        struct ListNode {
            using Item = T;

            ListNode* prev;
            ListNode* next;
            T* data;

            /// 在节点处构造一个元素. 若节点的data为nullptr, 且当前为DEBUG模式, 则引发panic, 否则, 行为未定义.
            template<typename ...Args>
            constexpr void construct(Args&& ...args) {
                MSTL_DEBUG_ASSERT(data != nullptr, "Trying to construct an object at nullptr.");
                std::construct_at(data, std::forward<Args>(args)...);
            }

            /// 在节点处销毁一个元素. 若节点的data为nullptr, 且当前为DEBUG模式, 则引发panic, 否则, 行为未定义.
            constexpr void destroy() {
                if (data != nullptr)
                    std::destroy_at(data);
            }

            // set the next node to n, and set the prev node of n to this, if possible
            constexpr void set_next(ListNode* n) {
                next = n;
                if (n != nullptr) {
                    n->prev = this;
                }
            }
        };

        static_assert(concepts::ForwardNode<ForwardListNode<int>>);
        static_assert(!concepts::Node<ForwardListNode<int>>);
        static_assert(concepts::Node<ListNode<int>>);

        static_assert(mstl::iter::Iterator<ListIter<int, ForwardListNode<int>>>);
        static_assert(mstl::iter::DoubleEndedIterator<ListIter<int, ListNode<int>>>);
    }

    template<typename T,
            concepts::ForwardNode Node,
            mstl::memory::concepts::Allocator A>
    requires (!basic::RefType<T>)
    class ListIntoIter;

    /**
     * @brief 非连续的可扩容容器类型(链表).
     *
     *  类似于`std::list`和`std::forward_list`, 取决于节点的类型.
     *
     * ## Example
     * @code
     *      List<int> ls = {1, 2, 3};  // List<int> is aka BaseList<int, Node<int>>
     *      assert(ls.front_unchecked() == 1);
     *      ls.push_back(4);
     *      assert(ls.back_unchecked() == 4);
     * @endcode
     *
     * @tparam T 储存的元素类型
     * @tparam Node 节点类型
     * @tparam A 分配器类型
     */
    template<typename T,
            concepts::ForwardNode Node,
            mstl::memory::concepts::Allocator A=mstl::memory::allocator::Allocator>
    requires (!basic::RefType<T>)
    class BaseList {
        static constexpr bool is_double_linked_list = concepts::Node<Node>;

        Node* head = nullptr;  // head node is always an invalid node
        Node* tail = nullptr;  // tail node is always an invalid node
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

        constexpr BaseList() noexcept requires std::default_initializable<A>
        : alloc{} {
            init_empty_list();
        }

        constexpr explicit BaseList(const A& a) noexcept requires basic::CopyAble<A>
        : alloc(a) {
            init_empty_list();
        }

        constexpr BaseList(usize count, const T& value, const A& allocator={})
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

        constexpr explicit BaseList(usize count, const A& allocator={})
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
        constexpr BaseList(InputIt first, InputIt last, const A& allocator=A{})
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

        constexpr BaseList(BaseList&& list) noexcept
                : head(list.head), tail(list.tail), len(list.len), alloc(std::move(list.alloc)) {
            list.head = nullptr;
            list.tail = nullptr;
            list.len = 0;
        }

        constexpr BaseList(BaseList&& list, const A& allocator) noexcept {
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

        constexpr BaseList(const BaseList& o)
        requires basic::CopyAble<T>: alloc(o.alloc) {
            copy_impl(o);
        }

        constexpr BaseList(const BaseList& o, const A& allocator)
        requires basic::CopyAble<T>: alloc(allocator) {
            copy_impl(o);
        }

        constexpr BaseList(std::initializer_list<T> init, const A& allocator=A{}): BaseList(init.begin(), init.end(), allocator) {}

        constexpr ~BaseList() {
            if (head == nullptr) {  // head should not be nullptr
                return;             // unless the list has been moved
            } else {
                clear();              // clear all elements int the list
                destroy_node(head);  // and finally release the virtual head node
                destroy_node(tail);  // and the virtual tail node.
            }
        }

        constexpr BaseList& operator=(const BaseList& rhs)
        requires basic::CopyAble<T> {
            if (&rhs == this) {
                return *this;
            }
            this->~BaseList();
            alloc = rhs.alloc;
            copy_impl(rhs);
            return *this;
        }

        constexpr BaseList& operator=(BaseList&& rhs) noexcept {
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

        constexpr BaseList& operator=(std::initializer_list<T> list) {
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

        constexpr A get_allocator() const noexcept {
            return alloc;
        }

        /// 判断该链表是否已被移动或销毁.
        /// 当该方法返回true, 则调用该方法的行为在事实上也是非法的.
        constexpr bool moved() const noexcept {
            return head == nullptr;
        }

    public:  // visitor
        /**
         * @brief 安全地返回第一个元素的引用.
         * @return 若链表不为空, 则返回第一个元素的引用的Option; 否则, 返回None.
         */
        constexpr Option<T&> front() noexcept {
            if (empty()) {
                return Option<T&>::none();
            } else {
                T* data = head->next->data;
                return Option<T&>::some(*data);
            }
        }

        constexpr Option<const T&> front() const noexcept {
            if (empty()) {
                return Option<const T&>::none();
            } else {
                const T* data = head->next->data;
                return Option<const T&>::some(*data);
            }
        }

        /**
         * @brief 返回第一个元素的引用.
         * @return 若链表不为空, 则返回第一个元素的引用; 否则, 行为未定义.
         */
        constexpr T& front_unchecked() noexcept {
            T* data = head->next->data;
            return *data;
        }

        constexpr const T& front_unchecked() const noexcept {
            T* data = head->next->data;
            return *data;
        }

        /**
         * @brief 安全地返回最后一个元素的引用.
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @return 若链表不为空, 则返回最后一个元素的引用的Option; 否则, 返回None.
         */
        constexpr Option<T&> back() noexcept requires is_double_linked_list {
            if (empty()) {
                return Option<T&>::none();
            } else {
                T* data = tail->prev->data;
                return Option<T&>::some(*data);
            }
        }

        constexpr Option<const T&> back() const noexcept requires is_double_linked_list {
            if (empty()) {
                return Option<const T&>::none();
            } else {
                const T* data = tail->prev->data;
                return Option<const T&>::some(*data);
            }
        }

        /**
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @brief 返回最后一个元素的引用.
         * @return 若链表不为空, 则返回最后一个元素的引用; 否则, 行为未定义.
         */
        constexpr T& back_unchecked() noexcept requires is_double_linked_list {
            T* data = tail->prev->data;
            return *data;
        }

        constexpr const T& front_unchecked() const noexcept requires is_double_linked_list {
            T* data = tail->prev->data;
            return *data;
        }

    public:  // iter
        /**
         * @brief 把链表转换为迭代器.
         *
         * 该迭代器将接管链表中的所有元素的所有权.
         *
         * @attention 调用该函数后, 链表将被消耗(或视为已被移动).
         * @return 链表转换而来的迭代器.
         */
        constexpr IntoIter into_iter() {
            return IntoIter{std::move(*this)};
        }

        /**
         * @brief 从一个MSTL风格的迭代器构建一个链表.
         * 它一般由collect()函数调用.
         * @param iter MSTL风格的迭代器.
         * @attention 由于链表不能储存引用, 因此, 迭代左值引用的迭代器将生成其所迭代的元素的副本, 而迭代右值引用的迭代器将使得其迭代元素被移入新链表.
         * @return 新构造的链表.
         */
        template<iter::Iterator Iter>
        constexpr static decltype(auto) from_iter(Iter iter) {
            BaseList res;
            Node* p = res.head;
            auto val = iter.next();
            while (val.is_some()) {
                Node* tmp = res.template construct_node(val.unwrap_unchecked());
                p->set_next(tmp);
                p = tmp;
                val = iter.next();
                res.len++;
            }
            p->set_next(res.tail);
            return res;
        }

        /**
         * @brief 获取链表的迭代器.
         *
         * 该迭代器将引用该链表.
         *
         * @return 链表的迭代器.
         */
        constexpr Iter iter() noexcept {
            return {head->next, tail};
        }

        constexpr iterator before_begin() noexcept {
            return ListIterSTL<T, Node>(head);
        }

        constexpr const_iterator before_begin() const noexcept {
            return cbefore_begin();
        }

        constexpr const_iterator cbefore_begin() const noexcept {
            return ListIterSTL<const T, Node>(head);
        }

        constexpr iterator begin() noexcept{
            return ListIterSTL<T, Node>(head->next);
        }

        constexpr const_iterator begin() const noexcept {
            return cbegin();
        }

        constexpr const_iterator cbegin() const noexcept{
            return ListIterSTL<const T, Node>(head->next);
        }

        constexpr iterator end() noexcept{
            return ListIterSTL<T, Node>(tail);
        }

        constexpr const_iterator end() const noexcept{
            return cend();
        }

        constexpr const_iterator cend() const noexcept{
            return ListIterSTL<const T, Node>(tail);
        }

        constexpr decltype(auto) rbegin() noexcept
        requires is_double_linked_list {
            return ListIterSTL<T, Node, true>(tail->prev);
        }

        constexpr decltype(auto) rbegin() const noexcept
        requires is_double_linked_list {
            return crbegin();
        }

        constexpr decltype(auto) crbegin() const noexcept
        requires is_double_linked_list {
            return ListIterSTL<const T, Node, true>(tail->prev);
        }

        constexpr decltype(auto) rend() noexcept
        requires is_double_linked_list {
            return ListIterSTL<T, Node, true>(head);
        }

        constexpr decltype(auto) rend() const noexcept
        requires is_double_linked_list {
            return crend();
        }

        constexpr decltype(auto) crend() const noexcept
        requires is_double_linked_list {
            return ListIterSTL<const T, Node, true>(head);
        }

    public:  // capacity
        /**
         * @brief 检查当前链表是否储存有元素.
         * @return 若链表为空, 则返回true; 否则, 返回false.
         */
        constexpr inline bool empty() const noexcept {
            return len == 0;
        }

        /**
         * @brief 检查当前链表储存元素的数量.
         * @return 返回当前链表储存元素的数量.
         */
        constexpr inline usize size() const noexcept {
            return len;
        }

    public:  // editors
        /**
         * @brief 清空链表.
         *
         * 销毁所有元素, 并解分配对应节点的空间.
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      ls.clear();
         *      assert(ls.empty());
         * @endcode
         */
        constexpr void clear() noexcept {
            Node* cur = head->next;
            while (cur != tail) {
                Node* tmp = cur;
                cur = cur->next;

                destroy_node(tmp);
            }
            len = 0;
            head->set_next(tail);
        }

        /**
         * @brief 在pos指向的位置后插入一个元素.
         *
         * 元素以复制的方式插入.
         *
         * @param pos 指向插入元素的位置
         * @param val 要插入的值
         * @return 指向新插入的元素的迭代器
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      int t = 0;
         *      ls.insert_after(ls.before_begin(), t);
         *      assert(ls.front_unchecked() == 0);
         * @endcode
         */
        constexpr iterator insert_after(const_iterator pos, const T& val)
        requires basic::CopyAble<T> {
            return emplace_after(pos, val);
        }

        /**
         * @brief 在pos指向的位置后插入一个元素.
         *
         * 元素以移动的方式插入.
         *
         * @param pos 指向插入元素的位置
         * @param val 要插入的值
         * @return 指向新插入的元素的迭代器
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      ls.insert_after(ls.before_begin(), 0);
         *      assert(ls.front_unchecked() == 0);
         * @endcode
         */
        constexpr iterator insert_after(const_iterator pos, T&& val)
        {
            return emplace_after(pos, std::forward<T>(val));
        }

        /**
         * @brief 在pos指向的位置后插入count个元素.
         *
         * 元素以复制的方式插入.
         *
         * @param pos 指向插入元素的位置
         * @param count 要插入的元素的个数.
         * @param val 要插入的值
         * @return 指向第一个插入的元素的迭代器
         */
        constexpr iterator insert_after(const_iterator pos, usize count, const T& val)
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

        /**
         * @brief 在pos指向的位置后插入范围内的元素.
         *
         * 插入[first, last)范围内的元素.
         *
         * @param pos 指向插入元素的位置
         * @param first 指向范围的的第一个元素的迭代器.
         * @param last 指向范围的最后一个元素(不含)的迭代器.
         * @return 指向第一个插入的元素的迭代器
         */
        template<iter::LegacyInputIterator InputIt>
        constexpr iterator insert_after(const_iterator pos, InputIt first, InputIt last) {
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

        constexpr iterator insert_after(const_iterator pos, std::initializer_list<T> ilist) {
            return insert_after(pos, ilist.begin(), ilist.end());
        }

        /**
         * @brief 在pos指向的位置后构造一个元素.
         *
         * @param pos 指向插入元素的位置
         * @return 指向新构造的元素的迭代器
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      ls.emplace_after(ls.before_begin(), 0);
         *      assert(ls.front_unchecked() == 0);
         * @endcode
         */
        template<typename ...Args>
        constexpr iterator emplace_after(const_iterator pos, Args&& ...args) {
            MSTL_DEBUG_ASSERT(pos != end(), "Trying to emplace element after the tail.");
            Node* tmp = construct_node(std::forward<Args>(args)...);
            tmp->set_next(pos.cur->next);
            pos.cur->set_next(tmp);
            len++;
            return ListIterSTL<T, Node>(tmp);
        }

        /**
         * @brief 删除pos所指向的元素的写下一个元素
         * @param pos 指向欲删除的元素
         * @return 被删除的元素的下一个元素. 若不存在这样的元素, 则返回end().
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      ls.erase_after(ls.before_begin());
         *      assert(ls.front_unchecked() == 2);
         * @endcode
         */
        constexpr iterator erase_after(const_iterator pos) {
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

        /**
         * @brief 删除(first, last)范围内的元素.
         * @param first 指向欲删除的第一个元素(不含)
         * @param last 指向欲删除的最后一个元素(不含)
         * @return 被删除的元素的下一个元素. 若不存在这样的元素, 则返回end().
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      ls.erase_after(ls.before_begin(), ls.end());
         *      assert(ls.empty());
         * @endcode
         */
        constexpr iterator erase_after(const_iterator first, const_iterator last) {
            while (true) {
                if (first == last || first.cur->next == last.cur)
                    break;
                erase_after(first);
            }
            return {last.cur};
        }

        /**
         * @brief 在pos指向的位置构造一个元素.
         *
         * @param pos 指向插入元素的位置
         * @return 指向新构造的元素的迭代器
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      ls.emplace(ls.begin(), 0);
         *      assert(ls.front_unchecked() == 0);
         * @endcode
         */
        template<class ...Args>
        constexpr iterator emplace(const_iterator pos, Args&& ...args)
        requires is_double_linked_list {
            MSTL_DEBUG_ASSERT(pos != before_begin(), "Trying to emplace element before the head.");
            Node* tmp = construct_node(std::forward<Args>(args)...);
            Node* prev = pos.cur->prev;

            prev->set_next(tmp);
            tmp->set_next(pos.cur);
            len++;
            return ListIterSTL<T, Node>(tmp);
        }

        /**
         * @brief 在pos指向的位置插入一个元素.
         *
         * 以复制的方式插入元素
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @param pos 指向插入元素的位置
         * @return 指向新构造的元素的迭代器
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      int t = 0;
         *      ls.insert(ls.begin(), t);
         *      assert(ls.front_unchecked() == 0);
         * @endcode
         */
        constexpr iterator insert(const_iterator pos, const T& val)
        requires basic::CopyAble<T> && is_double_linked_list {
            return emplace(pos, val);
        }

        /**
         * @brief 在pos指向的位置插入一个元素.
         *
         * 以移动的方式插入元素
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @param pos 指向插入元素的位置
         * @return 指向新构造的元素的迭代器
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      ls.insert(ls.begin(), 0);
         *      assert(ls.front_unchecked() == 0);
         * @endcode
         */
        constexpr iterator insert(const_iterator pos, T&& val)
            requires is_double_linked_list {
            return emplace(pos, std::forward<T>(val));
        }

        /**
         * @brief 在pos指向的位置插入count个元素.
         *
         * 元素以复制的方式插入.
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @param pos 指向插入元素的位置
         * @param count 要插入的元素的个数.
         * @param val 要插入的值
         * @return 指向第一个插入的元素的迭代器
         */
        constexpr iterator insert(const_iterator pos, usize count, const T& val)
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

        /**
         * @brief 在pos指向的位置插入范围内的元素.
         *
         * 插入[first, last)范围内的元素.
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @param pos 指向插入元素的位置
         * @param first 指向范围的的第一个元素的迭代器.
         * @param last 指向范围的最后一个元素(不含)的迭代器.
         * @return 指向第一个插入的元素的迭代器
         */
        template<iter::LegacyInputIterator InputIt>
        constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
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

        constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
            return insert(pos, ilist.begin(), ilist.end());
        }

        /**
         * @brief 删除pos所指向的元素
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @param pos 指向欲删除的元素
         * @return 被删除的元素的下一个元素. 若不存在这样的元素, 则返回end().
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      ls.erase_after(ls.begin());
         *      assert(ls.front_unchecked() == 2);
         * @endcode
         */
        constexpr iterator erase(const_iterator pos)
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

        /**
         * @brief 删除[first, last)范围内的元素.
         *
         * @note 若`first == before_begin()`, 则行为未定义.
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @return 最后被删除的元素的下一个元素. 若不存在这样的元素, 则返回end().
         *
         * ## Example
         * @code
         *      Listr<int> ls = {1, 2, 3};
         *      ls.erase_after(ls.begin(), ls.end());
         *      assert(ls.empty());
         * @endcode
         */
        constexpr iterator erase(const_iterator first, const_iterator last)
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

        /**
         * @brief 在链表前以移动的方式插入一个元素.
         * @param value 欲插入的元素.
         */
        constexpr void push_front(T&& value) noexcept
        {
            emplace_front(std::forward<T>(value));
        }

        /**
         * @brief 在链表前以复制的方式插入一个元素.
         * @param value 欲插入的元素.
         */
        constexpr void push_front(const T& value) noexcept
        requires basic::CopyAble<T> {
            emplace_front(value);
        }

        /**
         * @brief 在链表前构造一个元素.
         * @return 所插入的元素的引用.
         */
        template<typename ...Args>
        constexpr Reference emplace_front(Args&& ...args) noexcept {
            Node* node = construct_node(std::forward<Args>(args)...);

            node->set_next(head->next);
            head->set_next(node);
            len++;
            return *node->data;
        }

        /**
         * @brief 在链表前删除一个元素.
         */
        constexpr void pop_front() noexcept {
            Node* tmp = head->next;
            MSTL_DEBUG_ASSERT(tmp != nullptr, "Trying to pop front at an empty list.");
            head->set_next(tmp->next);
            destroy_node(tmp);
            len--;
        }

        /**
         * @brief 在链表后以移动的方式插入一个元素.
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @param value 欲插入的元素.
         */
        constexpr void push_back(T&& val) noexcept
        requires is_double_linked_list {
            emplace_back(std::forward<T>(val));
        }

        /**
         * @brief 在链表后以复制的方式插入一个元素.
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @param value 欲插入的元素.
         */
        constexpr void push_back(const T& val) noexcept
        requires basic::CopyAble<T> && is_double_linked_list {
            emplace_back(val);
        }

        /**
         * @brief 在链表后构造一个元素.
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         *
         * @return 新插入的元素的引用.
         */
        template<typename ...Args>
        constexpr Reference emplace_back(Args&&... args) noexcept
        requires is_double_linked_list {
            Node* node = construct_node(std::forward<Args>(args)...);

            tail->prev->set_next(node);
            node->set_next(tail);

            len++;
            return *node->data;
        }

        /**
         * @brief 在链表后删除一个元素.
         *
         * ## 约束
         * 节点必须满足`Node`. 即, 链表必须是双链表.
         */
        constexpr void pop_back() noexcept
        requires is_double_linked_list {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            MSTL_DEBUG_ASSERT(!empty(), "Trying to pop back at an empty list.");
            Node* tmp = tail->prev;
            tmp->prev->set_next(tail);
            len--;
            destroy_node(tmp);
        }

        /**
         * @brief 改变链表的大小.
         *
         * 若count < `size()`, 则缩小链表到count, 并销毁多余的元素; 否则, 扩大链表到count, 并在尾部填充默认构造的元素.
         */
        constexpr void resize(usize count) noexcept {
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

        /**
         * @brief 改变链表的大小.
         *
         * 若count < `size()`, 则缩小链表到count, 并销毁多余的元素; 否则, 扩大链表到count, 并在尾部以复制的方法填充r.
         */
        constexpr void resize(usize count, const T& val) noexcept {
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

        /**
         * @brief 交换两个链表中储存的元素, 同时交换两者的Allocator.
         * ## Example
         * @code
         *      List<int> a = {1, 2, 3}, b = {4, 5, 6};
         *      a.swap(b);
         *      assert(to_string(a) == "List [4, 5, 6]");
         *      assert(to_string(b) == "List [1, 2, 3]");
         * @endcode
         */
        constexpr void swap(BaseList& list) noexcept {
            std::swap(alloc, list.alloc);
            std::swap(head, list.head);
            std::swap(tail, list.tail);
            std::swap(len, list.len);
        }

    public:  // Operations
        /**
         * @brief 合并两个有序链表.
         *
         * 被合并的链表将被清空.
         *
         * ## Example
         * @code
         *      List<int> a = {1, 3, 5}, b = {2, 4, 6};
         *      a.merge(b);
         *      assert(to_string(a) == "List [1, 2, 3, 4, 5, 6]");
         *      assert(b.empty());
         * @endcode
         *
         * @param other 欲合并的链表
         */
        constexpr void merge(BaseList& other) {
            merge(other, std::less<T>{});
        }

        /**
         * @brief 合并两个有序链表.
         *
         * ## Example
         * @code
         *      List<int> a = {5, 3, 1}, b = {6, 4, 2};
         *      a.merge(b, [](const auto& a, const auto& b){ return a > b; });
         *      assert(to_string(a) == "List [6, 5, 4, 3, 2, 1]");
         *      assert(b.empty());
         * @endcode
         *
         * @tparam Compare 谓词, 判断元素之间的全序关系.
         * @param other 欲合并的链表
         * @param p 谓词. 比较两个元素, 若元素a应在元素b前, 则返回true; 否则返回false.
         */
        template<class Compare>
        constexpr void merge(BaseList& other, Compare p)
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

        /**
         * @brief 合并两个有序链表.
         *
         * 与`merge(BaseList& other)`类似, 但合并后, other视为已移动.
         *
         * @param other
         */
        constexpr void merge(BaseList&& other) {
            merge(std::forward<BaseList>(other), std::less<T>{});
        }

        /**
         * @brief 合并两个有序链表.
         *
         * 与`merge(BaseList& other)`类似, 但合并后, other视为已移动.
         *
         * @tparam Compare 谓词, 判断元素之间的全序关系.
         * @param other 欲合并的链表
         * @param p 谓词. 比较两个元素, 若元素a应在元素b前, 则返回true; 否则返回false.
         */
        template<class Compare>
        constexpr void merge(BaseList&& other, Compare p)
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

        constexpr void splice_after(const_iterator pos, BaseList& other) noexcept {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = other.real_tail();

            t->set_next(pos.cur->next);
            pos.cur->set_next(h);

            other.head->set_next(other.tail);
            len += other.len;
            other.len = 0;
        }

        constexpr void splice(const_iterator pos, BaseList& other) noexcept
        requires is_double_linked_list {
            Node* h = other.head->next;  // the start of the inserting nodes
            Node* t = other.real_tail();

            pos.cur->prev->set_next(h);
            t->set_next(pos.cur);

            other.head->set_next(other.tail);
            len += other.len;
            other.len = 0;
        }

        constexpr void splice_after(const_iterator pos, BaseList&& other) noexcept {
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

        constexpr void splice(const_iterator pos, BaseList&& other) noexcept
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

        /**
         * @brief 删除链表中等于val的元素.
         *
         * ## 约束
         * T必须实现相等运算符.
         *
         * @param val 欲删除的元素.
         * @return 删除的元素的数量.
         */
        constexpr usize remove(const T& val) noexcept requires ops::Eq<T, T> {
            return remove_if([&](const T& val1) {
                return val == val1;
            });
        }

        /**
         *  @brief 删除链表中满足谓词`predicate`的元素.
         * @tparam P 谓词
         * @param predicate 谓词, 若一个元素需要被删除, 则返回true; 否则, 返回false.
         * @return 删除的元素的数量.
         */
        template<typename P>
        constexpr usize remove_if(P predicate) noexcept
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

        /**
         * @brief 逆转一个链表.
         */
        constexpr void reverse() requires (!is_double_linked_list) {
            MSTL_DEBUG_ASSERT(head != nullptr, "The list has been moved.");
            if (len < 2) {
                return;
            }

            Node* p = head->next;
            Node* pr = tail;
            while (p != tail) {
                Node* tmp = p->next;
                p->set_next(pr);
                pr = p;
                p = tmp;
            }
            head->set_next(pr);
        }

        /**
         * @brief 逆转一个链表.
         */
        constexpr void reverse() requires is_double_linked_list {
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

        /**
         * @brief 删除相邻的相同元素, 仅保留一个.
         * @return 删除的元素的数量
         */
        constexpr usize unique() requires ops::Eq<T, T> {
            return unique(std::equal_to<T>());
        }

        /**
         * @brief 删除相邻的等价元素, 仅保留一个.
         *
         * 等价元素由谓词`p`定义.
         *
         * @tparam BinaryPredicate 谓词
         * @param p 谓词, 判断两个元素是否等价
         * @return
         */
        template<class BinaryPredicate>
        constexpr usize unique(BinaryPredicate p) requires ops::Predicate<BinaryPredicate, const T&, const T&> {
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

        /**
         * @brief 对链表进行排序.
         */
        constexpr void sort() {
            sort(std::less<T>());
        }

        /**
         * @brief 对链表进行排序.
         * @tparam P 谓词
         * @param predicate 谓词
         */
        template<typename P>
        constexpr void sort(P predicate)
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
        constexpr Node* alloc_node() {
            Node* node = alloc.template allocate<Node>(1);
            return node;
        }

        // allocate node and data within a continious space
        constexpr Node* alloc_node_with_data() {
            Node* node;
            T* data;
            if (std::is_constant_evaluated()) {
                node = alloc.template allocate<Node>(1);
                data = alloc.template allocate<T>(1);
            } else {
                node = (Node*)(alloc.allocate(get_data_node_layout(), 1));
                data = (T*)((usize)node + get_data_offset());
            }
            node->data = data;
            return node;
        }

        constexpr Node* build_virtual_node() {
            Node* r = alloc_node();
            r->next = nullptr;
            r->data = nullptr;
            if constexpr (is_double_linked_list) {
                r->prev = nullptr;
            }
            return r;
        }

        // deallocate the node without destroy it.
        constexpr void dealloc_node(Node* node) {
            alloc.template deallocate(node, 1);
        }

        constexpr void dealloc_node_with_data(Node* node) {
            if (std::is_constant_evaluated()) {
                alloc.template deallocate(node->data, 1);
                dealloc_node(node);
            } else {
                alloc.deallocate(node, get_data_node_layout(), 1);
            }
        }

        template<typename ...Args>
        constexpr Node* construct_node(Args&& ...args) {
            Node* n = alloc_node_with_data();
            n->construct(std::forward<Args>(args)...);
            return n;
        }

        // Destroy data and the node
        constexpr void destroy_node(Node* node) {
            if (node->data != nullptr) {
                node->destroy();  // destroy data
                dealloc_node_with_data(node);
            } else {
                dealloc_node(node);
            }

        }

        // DO NOT INVOKE when the list is not empty
        constexpr void init_empty_list() {
            Node* h = build_virtual_node();
            head = h;
            Node* t = build_virtual_node();
            tail = t;
            head->set_next(tail);
        }

        constexpr void copy_impl(const BaseList& other)
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

        constexpr void shorten(usize count) {
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

        constexpr void extend(usize count) {
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

        constexpr void extend(usize count, const T& val) {
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

        constexpr Node* real_tail() const {
            Node* p = head;
            while (p->next != tail) {
                p = p->next;
            }
            return p;
        }

        constexpr Node* real_tail() const requires is_double_linked_list {
            return tail->prev;
        }
    };

    template <typename T, memory::concepts::Allocator A=memory::allocator::Allocator>
    using List = BaseList<T, _private::ListNode<T>, A>;

    template <typename T, memory::concepts::Allocator A=memory::allocator::Allocator>
    using ForwardList = BaseList<T, _private::ForwardListNode<T>, A>;

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

        constexpr explicit ListIntoIter(BaseList<T, Node, A> &&list) : list(std::forward<BaseList<T, Node, A>>(list)) {}

        constexpr Option<Item> next() {
            if (!list.empty()) {
                auto res = Option<T>::some(std::move(list.front().unwrap_unchecked()));
                list.pop_front();
                return res;
            } else {
                return Option<T>::none();
            }
        }

        constexpr Option<Item> prev()
        requires concepts::Node<Node> {
            if (!list.empty()) {
                auto res = Option<T>::some(std::move(list.back().unwrap_unchecked()));
                list.pop_back();
                return res;
            } else {
                return Option<T>::none();
            }
        }
    };

    static_assert(mstl::iter::Iterator<ListIntoIter<int, _private::ForwardListNode<int>, memory::allocator::Allocator>>);
    static_assert(mstl::iter::DoubleEndedIterator<ListIntoIter<int, _private::ListNode<int>, memory::allocator::Allocator>>);
}

template<typename T>
struct std::iterator_traits<mstl::collection::ListIterSTL<T, mstl::collection::_private::ForwardListNode<T>>>
{
    typedef forward_iterator_tag       iterator_category;
    typedef T                          value_type;
    typedef mstl::usize                difference_type;
    typedef T*                         pointer;
    typedef T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::ListIterSTL<const T, mstl::collection::_private::ForwardListNode<T>>>
{
    typedef forward_iterator_tag             iterator_category;
    typedef T                                value_type;
    typedef mstl::usize                      difference_type;
    typedef const T*                         pointer;
    typedef const T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::ListIterSTL<T, mstl::collection::_private::ListNode<T>>>
{
    typedef bidirectional_iterator_tag iterator_category;
    typedef T                          value_type;
    typedef mstl::usize                difference_type;
    typedef T*                         pointer;
    typedef T&                         reference;
};

template<typename T>
struct std::iterator_traits<mstl::collection::ListIterSTL<const T, mstl::collection::_private::ListNode<T>>>
{
    typedef bidirectional_iterator_tag  iterator_category;
    typedef T                           value_type;
    typedef mstl::usize                 difference_type;
    typedef const T*                    pointer;
    typedef const T&                    reference;
};

#endif //MODERN_STL_LINKED_LIST_H
