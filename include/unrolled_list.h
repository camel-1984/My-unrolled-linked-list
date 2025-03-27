#pragma once

#include <exception>
#include <memory>

template<typename T, size_t NodeMaxSize = 10, typename Allocator = std::allocator<T>>
class unrolled_list {
public:

    class Node {
    public:
        T arr[NodeMaxSize];
        size_t node_size = 0;
        Node* next = nullptr;
        Node* prev = nullptr;
    };

    size_t list_size = 0;
    Node* head = nullptr;
    Node* tail = nullptr;

    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    NodeAllocator node_allocator;
    using ElementAllocator = std::allocator_traits<Allocator>;
    Allocator element_allocator;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using const_pointer = const T*;
    using const_reference = const T&;
    using allocator_type = std::allocator<Node>;

    class Iterator {
    public:
        Node* current_node = nullptr;
        size_t current_index = 0;

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using const_pointer = const T*;
        using const_reference = const T&;

        explicit Iterator(Node* node) : current_node(node) {}

        Iterator(Node* node, size_t index) : current_node(node), current_index(index) {}

        reference operator*() {
            return current_node->arr[current_index];
        }
        pointer operator->() {
            return &(current_node->arr[current_index]);
        }
        const_reference operator*() const {
            return current_node->arr[current_index];
        }
        const_pointer operator->() const {
            return current_node->arr[current_index];
        }
        Iterator& operator++() {
            if (current_index + 1 >= current_node->node_size) {
                current_node = current_node->next;
                current_index = 0;
            } else {
                ++current_index;
            }
            return *this;
        }
        Iterator& operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }
        Iterator& operator--() {
            if (current_index == 0) {
                current_node = current_node->prev;
                current_index = current_node->node_size - 1;
            } else {
                --current_index;
            }
            return *this;
        }
        Iterator& operator--(int) {
            Iterator temp = *this;
            --(*this);
            return temp;
        }
        bool operator==(const Iterator& other) const {
            return current_node == other.current_node && current_index == other.current_index;
        }
        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    };

    using iterator = Iterator;
    using const_iterator = const Iterator;
    using size_type = size_t;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    reference front() {
        return head->arr[0];
    }
    reference back() {
        return tail->arr[tail->node_size - 1];
    }
    const_reference front() const{
        return head->arr[0];
    }
    const_reference back() const {
        return tail->arr[tail->node_size - 1];
    }
    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const {
        return reverse_iterator(end());
    }
    reverse_iterator rend() {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const {
        return reverse_iterator(begin());
    }
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator crend() const {
        return const_reverse_iterator(begin());
    }
    iterator begin() {
        return Iterator(head, 0);
    }
    iterator end() {
        if (tail == nullptr) {
            return Iterator(nullptr);
        }
        return Iterator(tail->next);
    }
    const_iterator begin() const {
        return Iterator(head, 0);
    }
    const_iterator end() const {
        if (tail == nullptr) {
            return Iterator(nullptr);
        }
        return Iterator(tail->next);
    }
    const_iterator cbegin() const {
        return begin();
    }
    const_iterator cend() const {
        return end();
    }
    bool empty() {
        if (head == nullptr) {
            return true;
        }
        return false;
    }
    size_t size() {
        return list_size;
    }
    size_t max_size() {
        return std::numeric_limits<size_t>::max();
    }

    unrolled_list() = default;
    explicit unrolled_list(const allocator_type& alloc) : node_allocator(alloc){}
    explicit unrolled_list(const Allocator& alloc) : node_allocator(alloc), element_allocator(alloc) {}
    unrolled_list(size_t count, const T& value) {
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }
    unrolled_list(std::initializer_list<T> init) {
        for (const auto& item : init) {
            push_back(item);
        }
    }
    unrolled_list(const unrolled_list& other) {
        if (this == &other) {
            return;
        }
        node_allocator = other.node_allocator;
        element_allocator = other.element_allocator;
        list_size = other.list_size;
        node_allocator.allocate(list_size/NodeMaxSize);
        for (size_t i = 0; i < list_size; i++) {
            this->push_back(other[i]);
        }
    }
    template <typename InputIterator>
    unrolled_list(InputIterator begin, InputIterator end, const Allocator& alloc = Allocator())
        : node_allocator(alloc), element_allocator(alloc) {
        Node* current_node = nullptr;
        Node* last_allocated_node = nullptr;
        size_t elements_in_current_node = 0;
        try {
            for (; begin != end; ++begin) {
                if (!current_node || elements_in_current_node == NodeMaxSize) {
                    current_node = allocate_node();
                    elements_in_current_node = 0;
                    if (last_allocated_node) {
                        last_allocated_node->next = current_node;
                    } else {
                        head = current_node;
                    }
                    last_allocated_node = current_node;
                    tail = current_node;
                }
                ElementAllocator::construct(
                    element_allocator,
                    &current_node->arr[elements_in_current_node],
                    *begin
                );
                elements_in_current_node++;
                ++current_node->node_size;
                list_size++;
            }
        } catch (...) {
            rollback_after_exception(head, list_size);
            throw;
        }
    }
    unrolled_list(unrolled_list&& other, const allocator_type& alloc); // можно не делать

    ~unrolled_list() noexcept {
        clear();
    }

    void clear() noexcept {
        Node* current_node = head;
        while(current_node != nullptr) {
            Node* next_node = current_node->next;
            for (size_t i = 0; i < current_node->node_size; i++) {
                ElementAllocator::destroy(element_allocator, &(current_node->arr[i]));
            }
            node_allocator.deallocate(current_node, 1);
            current_node = next_node;
        }
        tail = nullptr;
        head = nullptr;
        list_size = 0;
    }
    allocator_type get_allocator() const noexcept {
        return node_allocator;
    }
    Node* allocate_node() {
        Node* node = std::allocator_traits<NodeAllocator>::allocate(node_allocator, 1);
        node->node_size = 0;
        node->next = nullptr;
        return node;
    }
    void remove_node(Node* node) {
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            head = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            tail = node->prev;
        }
        node_allocator.deallocate(node, 1);
    }
    void rollback_after_exception(Node* first_node, size_t elements_to_destroy) {
        Node* current_node = first_node;
        size_t destroyed_elements = 0;
        while (current_node && destroyed_elements < elements_to_destroy) {
            for (size_t i = 0; i < current_node->node_size; ++i) {
                if (destroyed_elements >= elements_to_destroy) break;
                ElementAllocator::destroy(
                    element_allocator,
                    &current_node->arr[i]
                );
                destroyed_elements++;
            }
            Node* next_node = current_node->next;
            std::allocator_traits<NodeAllocator>::deallocate(node_allocator, current_node, 1);
            current_node = next_node;
        }
        head = nullptr;
        tail = nullptr;
        list_size = 0;
    }

    unrolled_list& operator=(std::initializer_list<T> init) {
        clear();
        for (const auto& item : init) {
            push_back(item);
        }
        return *this;
    }
    unrolled_list& operator=(const unrolled_list& other) {
        if (this == &other) {
            return *this;
        }
        node_allocator = other.node_allocator;
        element_allocator = other.element_allocator;
        list_size = other.list_size;
        node_allocator.allocate(list_size/NodeMaxSize);
        for (size_t i = 0; i < list_size; i++) {
            this->push_back(other[i]);
        }
        return *this;
    }
    bool operator==(const unrolled_list& other) const {
        if (this == &other) {
            return true;
        }
        if (list_size != other.list_size) {
            return false;
        }
        Node* current_one = head;
        Node* current_two = other.head;
        size_t elements_compared = 0;
        while (current_one != nullptr && current_two != nullptr) {
            for (size_t j = 0; j < current_one->node_size; ++j) {
                if (current_one->arr[j] != current_two->arr[j]) {
                    return false;
                }
                ++elements_compared;
                if (elements_compared == list_size) {
                    return true;
                }
            }
            current_one = current_one->next;
            current_two = current_two->next;
        }
        return true;
    }
    bool operator!=(const unrolled_list& other) const {
        return !(this == other);
    }

    void push_back(const T& value) {
        bool new_node_added = false;
        Node* new_node = nullptr;
        if (!head || tail->node_size == NodeMaxSize) {
            new_node = node_allocator.allocate(1);
            try {
                new_node->node_size = 0;
                new_node->prev = tail;
                new_node->next = nullptr;
                if (!head) {
                    head = new_node;
                } else {
                    tail->next = new_node;
                }
                tail = new_node;
                new_node_added = true;
            } catch (...) {
                node_allocator.deallocate(new_node, 1);
                throw;
            }
        }
        try {
            ElementAllocator::construct(element_allocator, &(tail->arr[tail->node_size]), value);
            ++tail->node_size;
            list_size++;
        } catch (...) {
            if (new_node_added) {
                if (tail->prev) {
                    Node* prev_tail = tail->prev;
                    prev_tail->next = nullptr;
                    node_allocator.deallocate(tail, 1);
                    tail = prev_tail;
                } else {
                    node_allocator.deallocate(head, 1);
                    head = tail = nullptr;
                }
            }
            throw;
        }
    }
    void push_front(const T& value) {
    Node* new_node = nullptr;
    bool new_node_added = false;
        if (!head || head->node_size == NodeMaxSize) {
        new_node = node_allocator.allocate(1);
            try {
                new_node->node_size = 0;
                new_node->prev = nullptr;
                new_node->next = head;
                if (head) {
                head->prev = new_node;
                } else {
                tail = new_node;
                }
                head = new_node;
                new_node_added = true;
                } catch (...) {
                node_allocator.deallocate(new_node, 1);
                throw;
                }
        }
        try {
            for (size_t i = head->node_size; i > 0; --i) {
                std::allocator_traits<Allocator>::construct(element_allocator, &head->arr[i],
                    std::move_if_noexcept(head->arr[i - 1]));
                std::allocator_traits<Allocator>::destroy(element_allocator,&head->arr[i - 1]);
            }
            std::allocator_traits<Allocator>::construct(element_allocator, &head->arr[0], value);
            ++head->node_size;
            list_size++;
        } catch (...) {
            if (new_node_added) {
                if (head->next) {
                    head->next->prev = nullptr;
                } else {
                    tail = nullptr;
                }
                Node* old_head = head;
                head = head->next;
                node_allocator.deallocate(old_head, 1);
            } else {
                for (size_t i = 0; i < head->node_size - 1; ++i) {
                    std::allocator_traits<Allocator>::construct(element_allocator, &head->arr[i],
                        std::move_if_noexcept(head->arr[i + 1]));
                    std::allocator_traits<Allocator>::destroy(element_allocator, &head->arr[i + 1]);
                }
                --head->node_size;
            }
            throw;
        }
    }
    void pop_back()  noexcept {
        if (tail->node_size == 0) {
            return;
        }
        ElementAllocator::destroy(element_allocator, &(tail->arr[tail->node_size - 1]));
        --tail->node_size;
        list_size--;
        if (tail->node_size == 0) {
            if (tail->prev) {
                Node* prev_tail = tail->prev;
                prev_tail->next = nullptr;
                node_allocator.deallocate(tail, 1);
                tail = prev_tail;
            } else {
                node_allocator.deallocate(head, 1);
                head = tail = nullptr;
            }
        }
    }
    void pop_front() noexcept {
        if (head->node_size == 0) {
            return;
        }
        ElementAllocator::destroy(element_allocator, &(head->arr[0]));
        for (size_t i = 1; i < head->node_size; ++i) {
            head->arr[i - 1] = std::move(head->arr[i]);
        }
        --head->node_size;
        list_size--;
        if (head->node_size == 0) {
            if (head->next) {
                Node* prev_head = head->next;
                prev_head->prev = nullptr;
                node_allocator.deallocate(head, 1);
                head = prev_head;
            } else {
                node_allocator.deallocate(head, 1);
                head = tail = nullptr;
            }
        }
    }
    iterator erase(iterator pos) noexcept {
            Node* node = pos.current_node;
            size_t idx = pos.current_index;
            ElementAllocator::destroy(element_allocator, &(node->arr[idx]));
            for (size_t i = idx; i < node->node_size - 1; ++i) {
                node->arr[i] = std::move_if_noexcept(node->arr[i+1]);
            }
            --node->node_size;
            list_size--;
            if (node->node_size == 0) {
                if (node->prev) {
                    node->prev->next = node->next;
                } else {
                    head = node->next;
                }
                if (node->next) {
                    node->next->prev = node->prev;
                } else {
                    tail = node->prev;
                }
                node_allocator.deallocate(node, 1);
                if (head) {
                    return Iterator(head,0);
                } else {
                    return end();
                }
            }
            if (idx < node->node_size) {
                return Iterator(node, idx);
            } else if (node->next) {
                return Iterator(node->next, 0);
            } else {
                return end();
            }
    }
    iterator erase(iterator first, iterator last) noexcept {
        if (first == last) {
            return first;
        }
        Node* start_node = first.current_node;
        size_t start_idx = first.current_index;
        Node* end_node = last.current_node;
        size_t end_idx = last.current_index;
        if (start_node == end_node) {
            for (size_t i = start_idx; i < end_idx; ++i) {
                std::destroy_at(&start_node->arr[i]);
            }
            size_t shift_count = start_node->node_size - end_idx;
            for (size_t i = start_idx; i < start_idx + shift_count; ++i) {
                start_node->arr[i] = std::move(start_node->arr[end_idx + i - start_idx]);
            }
            start_node->node_size -= (end_idx - start_idx);
            list_size -= (end_idx - start_idx);
            if (start_node->node_size == 0) {
                remove_node(start_node);
                if (start_node->next) {
                    return Iterator(start_node->next, 0);
                } else {
                    return end();
                }
            }
            return Iterator(start_node, start_idx);
        }
        for (size_t i = start_idx; i < start_node->node_size; ++i) {
            std::destroy_at(&start_node->arr[i]);
            }
        size_t removed_from_start = start_node->node_size - start_idx;
        start_node->node_size = start_idx;
        for (size_t i = 0; i < end_idx; ++i) {
            std::destroy_at(&end_node->arr[i]);
        }
        size_t removed_from_end = end_idx;
        Node* current_node = start_node->next;
        while (current_node != end_node) {
            Node* next_node = current_node->next;
            remove_node(current_node);
            current_node = next_node;
        }
        list_size -= (removed_from_start + removed_from_end);
        if (start_node->node_size == 0) {
            remove_node(start_node);
            if (start_node->next) {
                return Iterator(start_node->next, 0);
            } else {
                return end();
            }
        }
        if (end_node->node_size == 0) {
            remove_node(end_node);
            return Iterator(end_node->next, 0);
        }
        return Iterator(end_node, 0);
    }
    Iterator insert(Iterator pos, const T& value) {
        T temp_value = value;
        size_t original_list_size = list_size;
        Node* original_head = head;
        Node* original_tail = tail;
        try {
            if (pos == end()) {
                push_back(temp_value);
                return Iterator(tail, tail->node_size - 1);
            }
            Node* node = pos.current_node;
            size_t index = pos.current_index;
            if (node->node_size == NodeMaxSize) {
                Node* new_node = node_allocator.allocate(1);
                std::allocator_traits<NodeAllocator>::construct(node_allocator, new_node);
                const size_t split_pos = NodeMaxSize / 2;
                for (size_t i = split_pos; i < NodeMaxSize; ++i) {
                    new_node->arr[i - split_pos] = std::move_if_noexcept(node->arr[i]);
                    node->arr[i].~T();
                }
                new_node->node_size = NodeMaxSize - split_pos;
                node->node_size = split_pos;
                new_node->next = node->next;
                new_node->prev = node;
                if (node->next) node->next->prev = new_node;
                node->next = new_node;
                if (index >= split_pos) {
                node = new_node;
                index -= split_pos;
                }
            }
            for (size_t i = node->node_size; i > index; --i) {
                node->arr[i] = std::move_if_noexcept(node->arr[i - 1]);
            }
            new (&node->arr[index]) T(std::move(temp_value));
            ++node->node_size;
            ++list_size;
            return Iterator(node, index);
        } catch (...) {
            if (list_size != original_list_size) {
                head = original_head;
                tail = original_tail;
                list_size = original_list_size;
            }
            throw;
        }
    }
    Iterator insert(Iterator pos, size_t count, const T& value) {
        if (count == 0) {
            return pos;
        }
        size_t original_list_size = list_size;
        Node* original_head = head;
        Node* original_tail = tail;
        try {
            Node* node = pos.current_node;
            size_t index = pos.current_index;
            if (pos == end()) {
                for (size_t i = 0; i < count; ++i) {
                    push_back(value);
                }
                return Iterator(end());
            }
            if (node->node_size + count <= NodeMaxSize) {
                for (size_t i = node->node_size; i > index; --i) {
                    node->arr[i+count-1] = std::move_if_noexcept(node->arr[i-1]);
                }
                for (size_t i = 0; i < count; ++i) {
                    ElementAllocator::construct(element_allocator, &node->arr[index+i], value);
                }
                node->node_size += count;
                list_size += count;
                return Iterator(node, index);
            }
            size_t need_to_move = node->node_size - index;
            Node* new_node = node_allocator.allocate(1);
            new_node->node_size = need_to_move;
            new_node->prev = node;
            if (node->next) {
                node->next->prev = new_node;
                new_node->next = node->next;
            } else {
                tail = new_node;
                new_node->next = nullptr;
            }
            node->next = new_node;
            for (size_t i = 0; i < need_to_move; i++) {
                new_node->arr[need_to_move-1-i] = std::move_if_noexcept(node->arr[node->node_size - 1 - i]);
                ElementAllocator::destroy(element_allocator,
                &node->arr[node->node_size - 1 - i]);
            }
            for (size_t i = 0; i < count; i++) {
                ElementAllocator::construct(element_allocator, &node->arr[index+i], value);
            }
            node->node_size = NodeMaxSize;
            list_size += count;
            return Iterator(node, index);
        } catch (...) {
            if (list_size != original_list_size) {
                head = original_head;
                tail = original_tail;
                list_size = original_list_size;
            }
            throw;
        }
    }
};
