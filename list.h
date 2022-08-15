#include <memory>
#include <type_traits>
#include <iterator>
#include <new>

template<typename T, typename Allocator = std::allocator<T>>
        class List {
private:
    struct BaseNode;
    struct Node;
public:
    template<bool isConst>
    struct common_iterator;

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;

    using AllocTraits = std::allocator_traits<Allocator>;
    using NodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using NodeAllocTraits = std::allocator_traits<NodeAlloc>;
    using TAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
    using TAllocTraits = std::allocator_traits<TAlloc>;
    using BaseNodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<BaseNode>;
    using BaseNodeAllocTraits = std::allocator_traits<BaseNodeAlloc>;

    explicit List(const Allocator& node_alloc = Allocator());
    List(size_t count, const T& value, const Allocator& alloc = Allocator());
    List(size_t count, const Allocator& alloc = Allocator());
    List(const List& list);
    List(List&& list) noexcept;
    ~List();
    List& operator=(const List& list);
    List& operator=(List&& list) noexcept;

    decltype(auto) get_allocator() const;
    size_t size() const;

    template<typename U>
    void push_back(U&& value);

    void push_back();
    void pop_back();

    template<typename U>
    void push_front(U&& value);

    void pop_front();

    template<typename U>
    iterator insert(const_iterator pos, U&& value);

    iterator erase(const_iterator pos);
private:
    Allocator alloc;
    BaseNodeAlloc base_node_alloc;
    NodeAlloc node_alloc;
    TAlloc t_alloc;
    size_t sz;
    BaseNode* basic;
    void createBasic();
    void moveBasic(List&& list);
    void copyList(const List& list);
    void moveList(List& list) noexcept;
    void checkPropagateOnContainerCopyAssignment(const List& list);
    void popAllNodes();
    void pop(BaseNode* deleting_node);

    template<typename U>
    BaseNode* push(BaseNode* pos, U&& value);

    BaseNode* push(BaseNode* pos);

    template<class Key, class Value, class Hash, class Equal, class Alloc>
    friend class UnorderedMap;
};









template<typename T, typename Allocator>
struct List<T, Allocator>::BaseNode {
    BaseNode* prev;
    BaseNode* next;
    BaseNode(BaseNode* prev, BaseNode* next);
    BaseNode() = default;
    virtual ~BaseNode() = default;
};

template<typename T, typename Allocator>
List<T, Allocator>::BaseNode::BaseNode(BaseNode* prev, BaseNode* next) : prev(prev), next(next) {}

template<typename T, typename Allocator>
struct List<T, Allocator>::Node : public List<T, Allocator>::BaseNode {
    T value;
    virtual ~Node() = default;
};









template<typename T, typename Allocator>
template<bool isConst>
struct List<T, Allocator>::common_iterator {
private:
    BaseNode* node;
public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = std::conditional_t<isConst, const T*, T*>;
    using reference = std::conditional_t<isConst, const T&, T&>;
    using iterator_category = std::bidirectional_iterator_tag;
    common_iterator(const BaseNode* node);
    common_iterator(BaseNode* node);
    common_iterator() = default;
    common_iterator(const common_iterator& iter);
    common_iterator& operator=(const common_iterator& common_iter) = default;
    ~common_iterator() = default;
    decltype(auto) operator++();
    decltype(auto) operator++(int);
    decltype(auto) operator--();
    decltype(auto) operator--(int);
    std::conditional_t<isConst, const T&, T&> operator*() const;
    std::conditional_t<isConst, const T*, T*> operator->() const;
    operator const_iterator();
    Node* getNode() const;
    friend bool operator==(const common_iterator& left, const common_iterator& right) {
        return left.node == right.node;
    }
    friend bool operator!=(const common_iterator& left, const common_iterator& right) {
        return left.node != right.node;
    }
    template<typename U>
    friend typename List<T, Allocator>::iterator List<T, Allocator>::insert(const_iterator pos,
                                                                            U&& value);
    friend typename List<T, Allocator>::iterator List<T, Allocator>::erase(const_iterator pos);
};

template<typename T, typename Allocator>
template<bool isConst>
List<T, Allocator>::common_iterator<isConst>::common_iterator(const typename List<T, Allocator>::BaseNode* node) :
node(node) {}

template<typename T, typename Allocator>
template<bool isConst>
List<T, Allocator>::common_iterator<isConst>::common_iterator(typename List<T, Allocator>::BaseNode* node) :
node(node) {}

template<typename T, typename Allocator>
template<bool isConst>
List<T, Allocator>::common_iterator<isConst>::common_iterator(const common_iterator& iter) :
node(iter.node) {}

template<typename T, typename Allocator>
template<bool isConst>
decltype(auto) List<T, Allocator>::common_iterator<isConst>::operator++() {
    node = node->next;
    return (*this);
}

template<typename T, typename Allocator>
template<bool isConst>
decltype(auto) List<T, Allocator>::common_iterator<isConst>::operator++(int) {
    common_iterator<isConst> return_it = *this;
    ++*this;
    return return_it;
}

template<typename T, typename Allocator>
template<bool isConst>
decltype(auto) List<T, Allocator>::common_iterator<isConst>::operator--() {
    node = node->prev;
    return (*this);
}

template<typename T, typename Allocator>
template<bool isConst>
decltype(auto) List<T, Allocator>::common_iterator<isConst>::operator--(int) {
    common_iterator<isConst> return_it = *this;
    --*this;
    return return_it;
}

template<typename T, typename Allocator>
template<bool isConst>
std::conditional_t<isConst, const T&, T&> List<T, Allocator>::common_iterator<isConst>::operator*() const {
    return reinterpret_cast<Node*>(node)->value;
}

template<typename T, typename Allocator>
template<bool isConst>
std::conditional_t<isConst, const T*, T*> List<T, Allocator>::common_iterator<isConst>::operator->() const {
    return &(reinterpret_cast<Node*>(node)->value);
}

template<typename T, typename Allocator>
template<bool isConst>
List<T, Allocator>::common_iterator<isConst>::operator const_iterator() {
    return const_iterator(node);
}

template<typename T, typename Allocator>
template<bool isConst>
typename List<T, Allocator>::Node*
List<T, Allocator>::common_iterator<isConst>::getNode() const {
    return static_cast<Node*>(node);
}










template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::begin() {
    return iterator(basic->next);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::begin() const {
    return cbegin();
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::end() {
    return iterator(basic);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::end() const {
    return cend();
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cbegin() const {
    return const_iterator(basic->next);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cend() const {
    return const_iterator(basic);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rbegin() {
    return reverse_iterator(basic);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rbegin() const {
    return crbegin();
}

template<typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rend() {
    return reverse_iterator(basic);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rend() const {
    return crend();
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crbegin() const {
    return const_reverse_iterator(basic);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crend() const {
    return const_reverse_iterator(basic);
}









template<typename T, typename Allocator>
void List<T, Allocator>::createBasic() {
    basic = BaseNodeAllocTraits::allocate(base_node_alloc, 1);
    BaseNodeAllocTraits::construct(base_node_alloc, basic, basic, basic);
}

template<typename T, typename Allocator>
List<T, Allocator>::List(const Allocator& alloc) : alloc(alloc), base_node_alloc(alloc),
node_alloc(alloc), t_alloc(alloc), sz(0) {
    createBasic();
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc) : alloc(alloc),
base_node_alloc(alloc), node_alloc(alloc), t_alloc(alloc), sz(0) {
    createBasic();
    for (size_t i = 0; i < count; ++i) {
        push_back(value);
    }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const Allocator& alloc) : alloc(alloc),
base_node_alloc(alloc), node_alloc(alloc), t_alloc(alloc), sz(0) {
    createBasic();
    for (size_t i = 0; i < count; ++i) {
        push_back();
    }
}

template<typename T, typename Allocator>
decltype(auto) List<T, Allocator>::get_allocator() const {
    return (alloc); //parentheses are important
}

template<typename T, typename Allocator>
void List<T, Allocator>::copyList(const List& list) {
    Node* list_node = reinterpret_cast<Node*>(list.basic->next);
    for(size_t i = 0; i < list.sz - 1; ++i) {
        push_back(list_node->value);
        list_node = reinterpret_cast<Node*>(list_node->next);
    }
    push_back(list_node->value);
}

template<typename T, typename Allocator>
void List<T, Allocator>::moveList(List& list) noexcept {
    basic->prev = list.basic->prev;
    basic->next = list.basic->next;
    list.basic->prev = list.basic->next = nullptr;
    list.sz = 0;
}

template<typename T, typename Allocator>
List<T, Allocator>::List(const List& list) :
alloc(AllocTraits::select_on_container_copy_construction(list.alloc)),
base_node_alloc(BaseNodeAllocTraits::select_on_container_copy_construction(list.base_node_alloc)),
node_alloc(NodeAllocTraits::select_on_container_copy_construction(list.node_alloc)),
t_alloc(TAllocTraits::select_on_container_copy_construction(list.t_alloc)),
sz(0) {
    createBasic();
    copyList(list);
}

template<typename T, typename Allocator>
List<T, Allocator>::List(List&& list) noexcept :
alloc(AllocTraits::select_on_container_copy_construction(list.alloc)),
base_node_alloc(BaseNodeAllocTraits::select_on_container_copy_construction(list.base_node_alloc)),
node_alloc(NodeAllocTraits::select_on_container_copy_construction(list.node_alloc)),
t_alloc(TAllocTraits::select_on_container_copy_construction(list.t_alloc)),
sz(list.sz) {
    createBasic();
    moveList(list);
}

template<typename T, typename Allocator>
void List<T, Allocator>::popAllNodes() {
    size_t old_size = sz;
    for(size_t i = 0; i < old_size; ++i) {
        pop_back();
    }
}

template<typename T, typename Allocator>
void List<T, Allocator>::checkPropagateOnContainerCopyAssignment(const List& list) {
    if (AllocTraits::propagate_on_container_copy_assignment::value && alloc != list.get_allocator()) {
        alloc = list.get_allocator();
        base_node_alloc = list.base_node_alloc;
        node_alloc = list.node_alloc;
        t_alloc = list.t_alloc;
    }
}

template<typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& list) {
    if(this == &list) return *this;
    popAllNodes();
    checkPropagateOnContainerCopyAssignment(list);
    copyList(list);
    return *this;
}

template<typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(List&& list) noexcept {
    if(this == &list) return *this;
    popAllNodes();
    checkPropagateOnContainerCopyAssignment(list);
    sz = list.sz;
    moveList(list);
    return *this;
}

template<typename T, typename Allocator>
List<T, Allocator>::~List() {
    popAllNodes();
    BaseNodeAllocTraits::destroy(base_node_alloc, basic);
    BaseNodeAllocTraits::deallocate(base_node_alloc, basic, 1);
}

template<typename T, typename Allocator>
size_t List<T, Allocator>::size() const {
    return sz;
}

template<typename T, typename Allocator>
void List<T, Allocator>::pop(BaseNode* deleting_node) {
    if(sz == 1) {
        basic->prev = basic->next = basic;
    } else {
        deleting_node->prev->next = deleting_node->next;
        deleting_node->next->prev = deleting_node->prev;
    }
    TAllocTraits::destroy(t_alloc, &(reinterpret_cast<Node*>(deleting_node)->value));
    BaseNodeAllocTraits::destroy(base_node_alloc, deleting_node);
    NodeAllocTraits::deallocate(node_alloc, reinterpret_cast<Node*>(deleting_node), 1);
    --sz;
}

template<typename T, typename Allocator>
template<typename U>
typename List<T, Allocator>::BaseNode* List<T, Allocator>::push(BaseNode* pos, U&& value) {
    BaseNode* node = NodeAllocTraits::allocate(node_alloc, 1);
    BaseNodeAllocTraits::construct(base_node_alloc, node, pos->prev, pos);
    TAllocTraits::construct(t_alloc, &(reinterpret_cast<Node*>(node)->value),
                            std::forward<U>(value));
    pos->prev = pos->prev->next = node;
    ++sz;
    return node;
}

template<typename T, typename Allocator>
typename List<T, Allocator>::BaseNode* List<T, Allocator>::push(BaseNode* pos) {
    BaseNode* node = NodeAllocTraits::allocate(node_alloc, 1);
    BaseNodeAllocTraits::construct(base_node_alloc, node, pos->prev, pos);
    TAllocTraits::construct(t_alloc, &(reinterpret_cast<Node*>(node)->value));
    pos->prev = pos->prev->next = node;
    ++sz;
    return node;
}

template<typename T, typename Allocator>
template<typename U>
void List<T, Allocator>::push_back(U&& value) {
    push(basic, std::forward<U>(value));
}

template<typename T, typename Allocator>
void List<T, Allocator>::push_back() {
    push(basic);
}

template<typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
    pop(basic->prev);
}

template<typename T, typename Allocator>
template<typename U>
void List<T, Allocator>::push_front(U&& value) {
    push(basic->next, std::forward<U>(value));
}

template<typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
    pop(basic->next);
}

template<typename T, typename Allocator>
template<typename U>
typename List<T, Allocator>::iterator List<T, Allocator>::insert(const_iterator pos, U&& value) {
    Node* node = reinterpret_cast<Node*>(push(pos.node, std::forward<U>(value)));
    return iterator(node);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::erase(const_iterator pos) {
    BaseNode* deleting_node = pos.node;
    ++pos;
    pop(deleting_node);
    return iterator(pos.node);
}