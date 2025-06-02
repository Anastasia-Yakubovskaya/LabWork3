#ifndef SKIP_LIST_H
#define SKIP_LIST_H

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace stl {

/**
 * @brief STL-compatible skip list container
 * @tparam T Type of elements stored in the container
 * @tparam Compare Comparison functor (defaults to std::less<T>)
 */
template <typename T, typename Compare = std::less<T>>
class skip_list {
private:
    /** @brief Skip list node structure */
    struct Node {
        T value;                        /**< Node value */
        std::vector<Node*> forward;      /**< Array of pointers to next nodes */

        /**
         * @brief Node constructor
         * @param val Value to store in node
         * @param level Node level (height)
         */
        Node(const T& val, size_t level) : value(val), forward(level + 1, nullptr) {}
        
        /**
         * @brief Node move constructor
         * @param val Value to move into node
         * @param level Node level (height)
         */
        Node(T&& val, size_t level) : value(std::move(val)), forward(level + 1, nullptr) {}
    };

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_compare = Compare;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    /**
     * @brief Skip list iterator class
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename skip_list::value_type;
        using difference_type = typename skip_list::difference_type;
        using pointer = typename skip_list::pointer;
        using reference = typename skip_list::reference;

        iterator() noexcept : current(nullptr) {}
        explicit iterator(Node* node) noexcept : current(node) {}

        reference operator*() const { return current->value; }
        pointer operator->() const { return &current->value; }

        iterator& operator++() {
            current = current->forward[0];
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const noexcept { return current == other.current; }
        bool operator!=(const iterator& other) const noexcept { return !(*this == other); }

    private:
        Node* current;
        friend class skip_list;
    };

    /**
     * @brief Skip list const iterator class
     */
    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename skip_list::value_type;
        using difference_type = typename skip_list::difference_type;
        using pointer = typename skip_list::const_pointer;
        using reference = typename skip_list::const_reference;

        const_iterator() noexcept : current(nullptr) {}
        explicit const_iterator(Node* node) noexcept : current(node) {}
        const_iterator(const iterator& it) noexcept : current(it.current) {}

        reference operator*() const { return current->value; }
        pointer operator->() const { return &current->value; }

        const_iterator& operator++() {
            current = current->forward[0];
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const noexcept { return current == other.current; }
        bool operator!=(const const_iterator& other) const noexcept { return !(*this == other); }

    private:
        const Node* current;
        friend class skip_list;
    };

    /**
     * @brief Default constructor
     */
    skip_list() : current_level(0), element_count(0), 
                 head(new Node(T(), MAX_LEVEL)), tail(nullptr),
                 comp(), gen(std::random_device{}()), dist(0.0f, 1.0f) {
        std::fill(head->forward.begin(), head->forward.end(), nullptr);
    }

    /**
     * @brief Constructor with custom comparator
     * @param comp Comparison functor
     */
    explicit skip_list(const Compare& comp) : current_level(0), element_count(0),
                                            head(new Node(T(), MAX_LEVEL)), tail(nullptr),
                                            comp(comp), gen(std::random_device{}()), dist(0.0f, 1.0f) {
        std::fill(head->forward.begin(), head->forward.end(), nullptr);
    }

    /**
     * @brief Copy constructor
     * @param other Skip list to copy from
     */
    skip_list(const skip_list& other) : current_level(0), element_count(0),
                                      head(new Node(T(), MAX_LEVEL)), tail(nullptr),
                                      comp(other.comp), gen(std::random_device{}()), dist(0.0f, 1.0f) {
        std::fill(head->forward.begin(), head->forward.end(), nullptr);
        for (const auto& val : other) {
            insert(val);
        }
    }

    /**
     * @brief Move constructor
     * @param other Skip list to move from
     */
    skip_list(skip_list&& other) noexcept 
        : current_level(other.current_level), element_count(other.element_count),
          head(other.head), tail(other.tail), comp(std::move(other.comp)),
          gen(std::move(other.gen)), dist(std::move(other.dist)) {
        other.head = new Node(T(), MAX_LEVEL);
        std::fill(other.head->forward.begin(), other.head->forward.end(), nullptr);
        other.current_level = 0;
        other.element_count = 0;
        other.tail = nullptr;
    }

    /**
     * @brief Initializer list constructor
     * @param init Initializer list with elements
     * @param comp Comparison functor
     */
    skip_list(std::initializer_list<T> init, const Compare& comp = Compare()) 
        : skip_list(comp) {
        insert(init.begin(), init.end());
    }

    /**
     * @brief Range constructor
     * @tparam InputIt Input iterator type
     * @param first Range begin iterator
     * @param last Range end iterator
     * @param comp Comparison functor
     */
    template <typename InputIt>
    skip_list(InputIt first, InputIt last, const Compare& comp = Compare()) 
        : skip_list(comp) {
        insert(first, last);
    }

    /**
     * @brief Destructor
     */
    ~skip_list() {
        clear();
        delete head;
    }

    /**
     * @brief Copy assignment operator
     * @param other Skip list to copy from
     * @return Reference to this skip list
     */
    skip_list& operator=(const skip_list& other) {
        if (this != &other) {
            clear();
            comp = other.comp;
            for (const auto& val : other) {
                insert(val);
            }
        }
        return *this;
    }

    /**
     * @brief Move assignment operator
     * @param other Skip list to move from
     * @return Reference to this skip list
     */
    skip_list& operator=(skip_list&& other) noexcept {
        if (this != &other) {
            clear();
            delete head;
            
            head = other.head;
            tail = other.tail;
            current_level = other.current_level;
            element_count = other.element_count;
            comp = std::move(other.comp);
            gen = std::move(other.gen);
            dist = std::move(other.dist);
            
            other.head = new Node(T(), MAX_LEVEL);
            std::fill(other.head->forward.begin(), other.head->forward.end(), nullptr);
            other.current_level = 0;
            other.element_count = 0;
            other.tail = nullptr;
        }
        return *this;
    }

    /**
     * @brief Initializer list assignment
     * @param init Initializer list with elements
     * @return Reference to this skip list
     */
    skip_list& operator=(std::initializer_list<T> init) {
        clear();
        insert(init.begin(), init.end());
        return *this;
    }

    /**
     * @brief Returns iterator to beginning
     * @return Iterator to first element
     */
    iterator begin() noexcept { return iterator(head->forward[0]); }

    /**
     * @brief Returns const iterator to beginning
     * @return Const iterator to first element
     */
    const_iterator begin() const noexcept { return const_iterator(head->forward[0]); }

    /**
     * @brief Returns const iterator to beginning
     * @return Const iterator to first element
     */
    const_iterator cbegin() const noexcept { return const_iterator(head->forward[0]); }

    /**
     * @brief Returns iterator to end
     * @return Iterator to element after last
     */
    iterator end() noexcept { return iterator(nullptr); }

    /**
     * @brief Returns const iterator to end
     * @return Const iterator to element after last
     */
    const_iterator end() const noexcept { return const_iterator(nullptr); }

    /**
     * @brief Returns const iterator to end
     * @return Const iterator to element after last
     */
    const_iterator cend() const noexcept { return const_iterator(nullptr); }

    /**
     * @brief Checks if container is empty
     * @return true if empty, false otherwise
     */
    bool empty() const noexcept { return element_count == 0; }

    /**
     * @brief Returns number of elements
     * @return Number of elements in container
     */
    size_type size() const noexcept { return element_count; }

    /**
     * @brief Clears the container
     */
    void clear() noexcept {
        Node* current = head->forward[0];
        while (current != nullptr) {
            Node* next = current->forward[0];
            delete current;
            current = next;
        }
        std::fill(head->forward.begin(), head->forward.end(), nullptr);
        current_level = 0;
        element_count = 0;
        tail = nullptr;
    }

    /**
     * @brief Inserts element
     * @param value Value to insert
     * @return Pair of iterator and bool indicating insertion success
     */
    std::pair<iterator, bool> insert(const T& value) {
        std::vector<Node*> update(MAX_LEVEL + 1, nullptr);
        Node* current = head;

        for (int i = current_level; i >= 0; --i) {
            while (current->forward[i] != nullptr && comp(current->forward[i]->value, value)) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0];

        if (current != nullptr && !comp(value, current->value) && !comp(current->value, value)) {
            return std::make_pair(iterator(current), false);
        }

        size_type new_level = random_level();
        if (new_level > current_level) {
            for (size_type i = current_level + 1; i <= new_level; ++i) {
                update[i] = head;
            }
            current_level = new_level;
        }

        Node* new_node = new Node(value, new_level);
        for (size_type i = 0; i <= new_level; ++i) {
            new_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = new_node;
        }

        if (new_node->forward[0] == nullptr) {
            tail = new_node;
        }

        ++element_count;
        return std::make_pair(iterator(new_node), true);
    }

    /**
     * @brief Inserts element with hint
     * @param hint Iterator hint for insertion position
     * @param value Value to insert
     * @return Iterator to inserted element
     */
    iterator insert(iterator hint, const T& value) {
        (void)hint;
        return insert(value).first;
    }

    /**
     * @brief Inserts elements from range
     * @tparam InputIt Input iterator type
     * @param first Range begin iterator
     * @param last Range end iterator
     */
    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    /**
     * @brief Inserts elements from initializer list
     * @param ilist Initializer list with elements
     */
    void insert(std::initializer_list<T> ilist) {
        insert(ilist.begin(), ilist.end());
    }

    /**
     * @brief Erases element at position
     * @param pos Iterator to element to erase
     * @return Iterator to next element
     */
    iterator erase(iterator pos) {
        if (pos == end()) {
            return end();
        }

        Node* node_to_delete = pos.current;
        iterator next(node_to_delete->forward[0]);

        std::vector<Node*> update(MAX_LEVEL + 1, nullptr);
        Node* current = head;

        for (int i = current_level; i >= 0; --i) {
            while (current->forward[i] != nullptr && comp(current->forward[i]->value, node_to_delete->value)) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0];

        if (current != node_to_delete) {
            return end();
        }

        for (size_type i = 0; i <= current_level; ++i) {
            if (update[i]->forward[i] != node_to_delete) {
                break;
            }
            update[i]->forward[i] = node_to_delete->forward[i];
        }

        if (node_to_delete == tail) {
            tail = (update[0] == head) ? nullptr : update[0];
        }

        delete node_to_delete;

        while (current_level > 0 && head->forward[current_level] == nullptr) {
            --current_level;
        }

        --element_count;
        return next;
    }

    /**
     * @brief Erases elements with given value
     * @param value Value to erase
     * @return Number of erased elements (0 or 1)
     */
    size_type erase(const T& value) {
        iterator it = find(value);
        if (it != end()) {
            erase(it);
            return 1;
        }
        return 0;
    }

    /**
     * @brief Erases elements in range
     * @param first Range begin iterator
     * @param last Range end iterator
     * @return Iterator to element after last erased
     */
    iterator erase(iterator first, iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return first;
    }

    /**
     * @brief Swaps contents with another skip list
     * @param other Skip list to swap with
     */
    void swap(skip_list& other) noexcept {
        using std::swap;
        swap(head, other.head);
        swap(tail, other.tail);
        swap(current_level, other.current_level);
        swap(element_count, other.element_count);
        swap(comp, other.comp);
        swap(gen, other.gen);
        swap(dist, other.dist);
    }

    /**
     * @brief Counts elements with given value
     * @param value Value to count
     * @return Number of elements (0 or 1 since duplicates not allowed)
     */
    size_type count(const T& value) const {
        return find(value) != end() ? 1 : 0;
    }

    /**
     * @brief Finds element with given value
     * @param value Value to find
     * @return Iterator to found element or end()
     */
    iterator find(const T& value) {
        Node* current = head;
        for (int i = current_level; i >= 0; --i) {
            while (current->forward[i] != nullptr && comp(current->forward[i]->value, value)) {
                current = current->forward[i];
            }
        }

        current = current->forward[0];
        if (current != nullptr && !comp(value, current->value) && !comp(current->value, value)) {
            return iterator(current);
        }
        return end();
    }

    /**
     * @brief Finds element with given value
     * @param value Value to find
     * @return Const iterator to found element or end()
     */
    const_iterator find(const T& value) const {
        Node* current = head;
        for (int i = current_level; i >= 0; --i) {
            while (current->forward[i] != nullptr && comp(current->forward[i]->value, value)) {
                current = current->forward[i];
            }
        }

        current = current->forward[0];
        if (current != nullptr && !comp(value, current->value) && !comp(current->value, value)) {
            return const_iterator(current);
        }
        return end();
    }

    /**
     * @brief Returns range of elements matching given value
     * @param value Value to find
     * @return Pair of iterators defining the range
     */
    std::pair<iterator, iterator> equal_range(const T& value) {
        iterator it = lower_bound(value);
        if (it != end() && !comp(value, *it)) {
            iterator next = it;
            ++next;
            return std::make_pair(it, next);
        }
        return std::make_pair(it, it);
    }

    /**
     * @brief Returns range of elements matching given value
     * @param value Value to find
     * @return Pair of const iterators defining the range
     */
    std::pair<const_iterator, const_iterator> equal_range(const T& value) const {
        const_iterator it = lower_bound(value);
        if (it != end() && !comp(value, *it)) {
            const_iterator next = it;
            ++next;
            return std::make_pair(it, next);
        }
        return std::make_pair(it, it);
    }

    /**
     * @brief Finds first element not less than value
     * @param value Value to compare
     * @return Iterator to found element or end()
     */
    iterator lower_bound(const T& value) {
        Node* current = head;
        for (int i = current_level; i >= 0; --i) {
            while (current->forward[i] != nullptr && comp(current->forward[i]->value, value)) {
                current = current->forward[i];
            }
        }
        current = current->forward[0];
        return iterator(current);
    }

    /**
     * @brief Finds first element not less than value
     * @param value Value to compare
     * @return Const iterator to found element or end()
     */
    const_iterator lower_bound(const T& value) const {
        Node* current = head;
        for (int i = current_level; i >= 0; --i) {
            while (current->forward[i] != nullptr && comp(current->forward[i]->value, value)) {
                current = current->forward[i];
            }
        }
        current = current->forward[0];
        return const_iterator(current);
    }

    /**
     * @brief Finds first element greater than value
     * @param value Value to compare
     * @return Iterator to found element or end()
     */
    iterator upper_bound(const T& value) {
        Node* current = head;
        for (int i = current_level; i >= 0; --i) {
            while (current->forward[i] != nullptr && !comp(value, current->forward[i]->value)) {
                current = current->forward[i];
            }
        }
        current = current->forward[0];
        return iterator(current);
    }

    /**
     * @brief Finds first element greater than value
     * @param value Value to compare
     * @return Const iterator to found element or end()
     */
    const_iterator upper_bound(const T& value) const {
        Node* current = head;
        for (int i = current_level; i >= 0; --i) {
            while (current->forward[i] != nullptr && !comp(value, current->forward[i]->value)) {
                current = current->forward[i];
            }
        }
        current = current->forward[0];
        return const_iterator(current);
    }

    /**
     * @brief Returns comparison functor
     * @return Value comparison functor
     */
    value_compare value_comp() const {
        return comp;
    }

    /**
     * @brief Equality comparison operator
     * @param other Skip list to compare with
     * @return true if equal, false otherwise
     */
    bool operator==(const skip_list& other) const {
        if (size() != other.size()) {
            return false;
        }
        return std::equal(begin(), end(), other.begin());
    }

    /**
     * @brief Inequality comparison operator
     * @param other Skip list to compare with
     * @return true if not equal, false otherwise
     */
    bool operator!=(const skip_list& other) const {
        return !(*this == other);
    }

private:
    static constexpr size_type MAX_LEVEL = 16;
    static constexpr float PROBABILITY = 0.5f;
    size_type current_level;
    size_type element_count;
    Node* head;
    Node* tail;
    Compare comp;
    mutable std::mt19937 gen;
    mutable std::uniform_real_distribution<float> dist;

    size_type random_level() const {
        size_type level = 0;
        while (dist(gen) < PROBABILITY && level < MAX_LEVEL) {
            ++level;
        }
        return level;
    }
};

} // namespace stl

#endif // SKIP_LIST_H
