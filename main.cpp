#include <iostream>
#include <vector>
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

    Node* extract(const_iterator pos);
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
    void retieNeighbours(Node* extracting_node);
    List<T, Allocator>::iterator tieNeighboursToNewNode(BaseNode* pos, BaseNode* new_node);
    void destroyNode(Node* node);

    Node* createNullNode();

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
    common_iterator(const common_iterator& iter) = default;
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
    if (list.sz == 0) {
        return;
    }
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
    list.basic->next->prev = list.basic->prev->next = basic;
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
    retieNeighbours(reinterpret_cast<Node*>(deleting_node));
    destroyNode(reinterpret_cast<Node*>(deleting_node));
    --sz;
}

template<typename T, typename Allocator>
void List<T, Allocator>::destroyNode(Node* node) {
    TAllocTraits::destroy(t_alloc, &(node->value));
    BaseNodeAllocTraits::destroy(base_node_alloc, node);
    NodeAllocTraits::deallocate(node_alloc, node, 1);
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
    tieNeighboursToNewNode(pos, node);
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

template<typename T, typename Allocator>
void List<T, Allocator>::retieNeighbours(Node* extracting_node) {
    if(sz == 1) {
        basic->prev = basic->next = basic;
    } else {
        extracting_node->prev->next = extracting_node->next;
        extracting_node->next->prev = extracting_node->prev;
    }
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator
List<T, Allocator>::tieNeighboursToNewNode(BaseNode* pos, BaseNode* new_node) {
    new_node->prev = pos->prev;
    new_node->next = pos;
    pos->prev = pos->prev->next = new_node;
    ++sz;
    return iterator(new_node);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::Node* List<T, Allocator>::extract(const_iterator pos) {
    retieNeighbours(pos.getNode());
    --sz;
    return pos.getNode();
}

template<typename T, typename Allocator>
typename List<T, Allocator>::Node* List<T, Allocator>::createNullNode() {
    Node* node = NodeAllocTraits::allocate(node_alloc, 1);
    BaseNodeAllocTraits::construct(base_node_alloc, reinterpret_cast<BaseNode*>(node), nullptr, nullptr);
    return node;
}















































template<class Key, class Value, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>,
        class Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
private:
    struct Unit;
public:
    using AllocTraits = std::allocator_traits<Alloc>;
    using UnitAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Unit>;
    using UnitAllocTraits = std::allocator_traits<UnitAlloc>;
    using UnitIterAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<
            typename List<Unit, UnitAlloc>::iterator>;
    using UnitIterAllocTraits = std::allocator_traits<UnitIterAlloc>;
private:
    static float _max_load_factor;

    Alloc alloc;
    List<Unit, UnitAlloc> units;
    std::vector<typename List<Unit, UnitAlloc>::iterator, UnitIterAlloc> buckets;
    Hash hasher;
    Equal comparator;

public:
    using NodeType = std::pair<const Key, Value>;
    UnorderedMap();
    UnorderedMap(const UnorderedMap& unordered_map);
    UnorderedMap(UnorderedMap&& unordered_map) noexcept;
    UnorderedMap& operator=(const UnorderedMap& unordered_map);
    UnorderedMap& operator=(UnorderedMap&& unordered_map) noexcept;
    ~UnorderedMap() = default;

    template<bool isConst>
    struct common_iterator;

    using Iterator = common_iterator<false>;
    using ConstIterator = common_iterator<true>;

    Iterator begin();
    ConstIterator begin() const;
    Iterator end();
    ConstIterator end() const;
    ConstIterator cbegin() const;
    ConstIterator cend() const;

    size_t bucket_count() const;
    size_t size() const;
    size_t max_size() const;
    float load_factor() const;
    float max_load_factor() const;
    void max_load_factor(float ml);

    Value& operator[](Key&& key);
    Value& operator[](const Key& key);

    Value& at(const Key& key) const;

    template<typename __NodeType>
    std::enable_if_t<std::is_constructible_v<NodeType, __NodeType&&>, std::pair<Iterator, bool>>
    insert(__NodeType&& key_val);
    std::pair<Iterator, bool> insert(const NodeType& key_val);
    std::pair<Iterator, bool> insert(NodeType&& key_val);

    template<class InputIterator>
    void insert(InputIterator begin, InputIterator end);

    template<class... Args>
    std::pair<Iterator, bool> emplace(Args&&... args);

    ConstIterator find(const Key& key) const;
    Iterator find(const Key& key);

    void erase(Iterator iter);
    void erase(Iterator begin, Iterator end);

    void reserve(size_t new_size);

private:
    void insertNodeInList(typename List<Unit, UnitAlloc>::Node* node, List<Unit, UnitAlloc>& units,
                          std::vector<typename List<Unit, UnitAlloc>::iterator, UnitIterAlloc>& buckets);

    void rehash_if();
    decltype(auto) findValueInBucket(size_t hash, const Key& key) const;

    template<typename __NodeType>
    decltype(auto) insertNewUnitAtBucketBegin(__NodeType&& key_val, size_t hash,
                                              bool insert_to_end);
    template<typename __Key>
    Value& generalOperatorSquareBrackets(__Key&& key);

    bool bucketIsEmpty(
            const std::vector<typename List<Unit, UnitAlloc>::iterator, UnitIterAlloc>& buckets,
            size_t hash) const;

    size_t countHash(const Key& key) const;
};

template<class Key, class Value, class Hash, class Equal, class Alloc>
float UnorderedMap<Key, Value, Hash, Equal, Alloc>::_max_load_factor = 0.95;










template<class Key, class Value, class Hash, class Equal, class Alloc>
struct UnorderedMap<Key, Value, Hash, Equal, Alloc>::Unit {
    NodeType key_val;
    size_t hash;

    template<class __Key, class __Value>
    Unit(__Key&& key, __Value&& value, size_t hash);

    template<class __NodeType>
    Unit(__NodeType&& key_val, size_t hash);

    Unit(const Unit& unit);
    Unit& operator=(const Unit& unit);

    Unit(Unit&& unit) noexcept;
    Unit& operator=(Unit&& unit) noexcept;
};

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<class __Key, class __Value>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::Unit::Unit
        (__Key&& key, __Value&& value, size_t hash) :
        key_val(std::forward<__Key>(key), std::forward<__Value>(value)), hash(hash) {}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<class __NodeType>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::Unit::Unit
        (__NodeType&& key_val, size_t hash) :
        key_val(std::forward<__NodeType>(key_val)), hash(hash) {}

template<class Key, class Value, class Hash, class Equal, class Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::Unit::Unit(const Unit& unit) :
        key_val(unit.key_val), hash(unit.hash) {}

template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Unit&
UnorderedMap<Key, Value, Hash, Equal, Alloc>::Unit::operator=(const Unit& unit) {
    key_val = unit.key_val;
    hash = unit.hash;
    return *this;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::Unit::Unit(Unit&& unit) noexcept :
        key_val(std::move(unit.key_val)), hash(unit.hash) {}

template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Unit&
UnorderedMap<Key, Value, Hash, Equal, Alloc>::Unit::operator=(Unit&& unit) noexcept {
    key_val = std::move(unit.key_val);
    hash = unit.hash;
    return *this;
}










template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
struct UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator {
private:
    std::conditional_t<isConst, typename List<Unit, UnitAlloc>::const_iterator,
            typename List<Unit, UnitAlloc>::iterator> list_iter;
public:
    using difference_type = std::ptrdiff_t;
    using value_type = Value;
    using pointer = std::conditional_t<isConst, const Value*, Value*>;
    using reference = std::conditional_t<isConst, const Value&, Value&>;
    using iterator_category = std::forward_iterator_tag;

    common_iterator() = default;
    common_iterator(const typename List<Unit, UnitAlloc>::template
    common_iterator<isConst>& list_iter);
    common_iterator(const common_iterator& iter) = default;
    decltype(auto) operator=(const common_iterator& iter);

    decltype(auto) operator++();
    decltype(auto) operator++(int);
    std::conditional_t<isConst, const NodeType&, NodeType&> operator*();
    std::conditional_t<isConst, const NodeType*, NodeType*> operator->();
    operator ConstIterator();

    friend bool operator==(const common_iterator& first, const common_iterator& second) {
        return first.list_iter == second.list_iter;
    }
    friend bool operator!=(const common_iterator& first, const common_iterator& second) {
        return !(first == second);
    }
    std::conditional_t<isConst, const size_t&, size_t&> hash() const;
    decltype(auto) listIterator() const;
};

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator<isConst>::
common_iterator(const typename List<Unit, UnitAlloc>::template common_iterator<isConst>&
list_iter) : list_iter(list_iter) {}


template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
decltype(auto) UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator<isConst>::
operator=(const common_iterator<isConst>& iter) {
    list_iter = iter.list_iter;
    return *this;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
decltype(auto) UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator<isConst>::
operator++() {
    ++list_iter;
    return *this;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
decltype(auto) UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator<isConst>::
operator++(int) {
    auto return_it = *this;
    ++*this;
    return return_it;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
std::conditional_t<isConst,
        const typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::NodeType&,
        typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::NodeType&>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator<isConst>::
operator*() {
    return list_iter->key_val;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
std::conditional_t<isConst,
        const typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::NodeType*,
        typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::NodeType*>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator<isConst>::operator->() {
    return &list_iter->key_val;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator<isConst>::
operator ConstIterator() {
    return ConstIterator(list_iter);
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
decltype(auto) UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator<isConst>::
listIterator() const {
    return list_iter;
}


template<class Key, class Value, class Hash, class Equal, class Alloc>
template<bool isConst>
std::conditional_t<isConst, const size_t&, size_t&>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::common_iterator<isConst>::hash() const {
    return list_iter->hash;
}










template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::begin() {
    return Iterator(units.begin());
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::begin() const {
    return cbegin();
}


template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::end() {
    return Iterator(units.end());
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::end() const {
    return cend();
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::cbegin() const {
    return ConstIterator(units.cbegin());
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::cend() const {
    return ConstIterator(units.cend());
}










template<class Key, class Value, class Hash, class Equal, class Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap() : buckets(2) {}

template<class Key, class Value, class Hash, class Equal, class Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap(const UnorderedMap& unordered_map) :
        alloc(AllocTraits::select_on_container_copy_construction(unordered_map.alloc)),
        units(unordered_map.units), buckets(unordered_map.buckets) {}

template<class Key, class Value, class Hash, class Equal, class Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap(UnorderedMap&& unordered_map) noexcept :
        alloc(AllocTraits::select_on_container_copy_construction(unordered_map.alloc)),
        units(std::move(unordered_map.units)), buckets(std::move(unordered_map.buckets)) {}

template<class Key, class Value, class Hash, class Equal, class Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>&
UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(const UnorderedMap& unordered_map) {
    if(this == &unordered_map) return *this;
    if(AllocTraits::propagate_on_container_copy_assignment::value && alloc != unordered_map.alloc) {
        alloc = unordered_map.alloc;
    }
    units = unordered_map.units;
    buckets = unordered_map.buckets;
    return *this;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>&
UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(UnorderedMap&& unordered_map) noexcept {
    if(this == &unordered_map) return *this;
    if(AllocTraits::propagate_on_container_copy_assignment::value && alloc != unordered_map.alloc) {
        alloc = unordered_map.alloc;
    }
    units = std::move(unordered_map.units);
    buckets = std::move(unordered_map.buckets);
    return *this;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<class __Key>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::generalOperatorSquareBrackets(__Key&& key) {
    rehash_if();
    size_t hash = countHash(key);
    if (bucketIsEmpty(buckets, hash)) {
        return insertNewUnitAtBucketBegin(
                NodeType(std::forward<Key>(key), Value()), hash, true).first->second;
    }
    std::pair<Iterator, bool> value_was_found_in_bucket = findValueInBucket(hash, key);
    if (!value_was_found_in_bucket.second) {
        return value_was_found_in_bucket.first->second;
    }
    return insertNewUnitAtBucketBegin(
            NodeType(std::forward<__Key>(key), Value()), hash, false).first->second;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator[](const Key& key) {
    return generalOperatorSquareBrackets(key);
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator[](Key&& key) {
    return generalOperatorSquareBrackets(std::move(key));
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(const Key& key) const {
    size_t hash = countHash(key);
    if (bucketIsEmpty(buckets, hash)) {
        throw std::exception();
    }
    std::pair<Iterator, bool> value_was_found_in_bucket = findValueInBucket(hash, key);
    if (!value_was_found_in_bucket.second) {
        return value_was_found_in_bucket.first->second;
    }
    throw std::exception();
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
decltype(auto)
UnorderedMap<Key, Value, Hash, Equal, Alloc>::findValueInBucket(size_t hash, const Key& key) const {
    auto it = Iterator(buckets[hash]);
    while (it.hash() == hash) {
        if (comparator(it->first, key)) {
            return std::pair<Iterator, bool>{it, false};
        }
        ++it;
    }
    return std::pair<Iterator, bool>{it, true};
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<typename __NodeType>
decltype(auto)
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insertNewUnitAtBucketBegin
        (__NodeType&& key_val, size_t hash, bool insert_to_end) {
    buckets[hash] = units.insert((insert_to_end ? units.end() : buckets[hash]),
                                 Unit(std::forward<__NodeType>(key_val), hash));
    return std::pair<Iterator, bool>(Iterator(buckets[hash]), true);
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<typename __NodeType>
std::enable_if_t<std::is_constructible_v<
        typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::NodeType, __NodeType&&>,
        std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator, bool>>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(__NodeType&& key_val) {
    return emplace(std::forward<__NodeType>(key_val));
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(const NodeType& key_val) {
    return emplace(key_val);
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(NodeType&& key_val) {
    return emplace(std::move(key_val));
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(const Key& key) const {
    size_t hash = countHash(key);
    if (bucketIsEmpty(buckets, hash)) {
        return end();
    }
    std::pair<Iterator, bool> value_was_found_in_bucket = findValueInBucket(hash, key);
    if (!value_was_found_in_bucket.second) {
        return value_was_found_in_bucket.first;
    }
    return end();
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator
UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(const Key& key) {
    size_t hash = countHash(key);
    if (bucketIsEmpty(buckets, hash)) {
        return end();
    }
    std::pair<Iterator, bool> value_was_found_in_bucket = findValueInBucket(hash, key);
    if (!value_was_found_in_bucket.second) {
        return value_was_found_in_bucket.first;
    }
    return end();
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<class InputIterator>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::
insert(InputIterator begin, InputIterator end) {
    for(auto it = begin; it != end; ++it) {
        insert(*it);
    }
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::rehash_if() {
    if(size() + 1 > max_load_factor() * bucket_count()) {
        reserve(2 * bucket_count() * max_load_factor() + 1);
    }
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::reserve(size_t new_size) {
    if(new_size / max_load_factor() + 1 < size()) {
        return;
    }
    std::vector<typename List<Unit, UnitAlloc>::iterator, UnitIterAlloc>
            new_buckets(new_size / max_load_factor() + 1);
    List<Unit, UnitAlloc> new_units;
    typename List<Unit, UnitAlloc>::iterator iter = units.begin();
    while (iter != units.end()) {
        typename List<Unit, UnitAlloc>::iterator copy_iter = iter++;
        typename List<Unit, UnitAlloc>::Node* node = units.extract(copy_iter);
        insertNodeInList(node, new_units, new_buckets);
    }
    buckets = std::move(new_buckets);
    units = std::move(new_units);
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::
insertNodeInList(typename List<Unit, UnitAlloc>::Node* node, List<Unit, UnitAlloc>& units,
                 std::vector<typename List<Unit, UnitAlloc>::iterator, UnitIterAlloc>& buckets) {
    size_t hash = hasher(node->value.key_val.first) % buckets.size();
    node->value.hash = hash;
    if (bucketIsEmpty(buckets, hash)) {
        buckets[hash] = units.tieNeighboursToNewNode(units.end().getNode(), node);
        return;
    }
    buckets[hash] = units.tieNeighboursToNewNode(buckets[hash].getNode(), node);
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
template<class... Args>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator, bool>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::emplace(Args&&... args) {
    rehash_if();
    typename List<Unit, UnitAlloc>::Node* node = units.createNullNode();
    AllocTraits::construct(alloc, &(node->value.key_val), std::forward<Args>(args)...);
    size_t hash = node->value.hash = countHash(node->value.key_val.first);
    if (bucketIsEmpty(buckets, hash)) {
        buckets[hash] = units.tieNeighboursToNewNode(units.end().getNode(), node);
        return std::pair<Iterator, bool>(Iterator(buckets[hash]), true);
    }
    auto value_was_found_in_bucket = findValueInBucket(hash, node->value.key_val.first);
    if (!value_was_found_in_bucket.second) {
        units.destroyNode(node);
        return value_was_found_in_bucket;
    }
    buckets[hash] = units.tieNeighboursToNewNode(buckets[hash].getNode(), node);
    return std::pair<Iterator, bool>(Iterator(buckets[hash]), true);
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(Iterator iter) {
    Iterator next_iter = iter;
    ++next_iter;
    buckets[iter.hash()] = (next_iter.hash() == iter.hash() ? next_iter.listIterator() : Iterator().listIterator());
    units.erase(iter.listIterator());
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(Iterator begin, Iterator end) {
    while (begin != end) {
        Iterator iter = begin;
        ++begin;
        erase(iter);
    }
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::bucket_count() const {
    return buckets.size();
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::size() const {
    return units.size();
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::max_size() const { //18446744073709551615
    return 100000000;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
float UnorderedMap<Key, Value, Hash, Equal, Alloc>::load_factor() const {
    return static_cast<float>(size()) / bucket_count();
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
float UnorderedMap<Key, Value, Hash, Equal, Alloc>::max_load_factor() const {
    return _max_load_factor;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::max_load_factor(float ml) {
    _max_load_factor = ml;
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
bool UnorderedMap<Key, Value, Hash, Equal, Alloc>::bucketIsEmpty(
        const std::vector<typename List<Unit, UnitAlloc>::iterator, UnitIterAlloc>& buckets,
        size_t hash) const {
    return buckets[hash] == typename List<Unit, UnitAlloc>::iterator();
}

template<class Key, class Value, class Hash, class Equal, class Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::countHash(const Key& key) const {
    return hasher(key) % buckets.size();
}