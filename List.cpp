#pragma once
#include <cassert>
#include <execution>
#include <cstddef>
#include <iterator>

template <typename Type>
class SingleLinkedList {
    struct Node {
        Node() = default;
        Node(const Type& val, Node* next)
            : value(val)
            , next_node(next) {
        }

        Type value;
        Node* next_node = nullptr;
    };

    template <typename ValueType>
    class BasicIterator {
        friend class SingleLinkedList;

        explicit BasicIterator(Node* node) : node_(node)
        {
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Type;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        BasicIterator() = default;

        BasicIterator(const BasicIterator<Type>& other) noexcept : node_(other.node_)
        {
        }

        BasicIterator& operator=(const BasicIterator& rhs) = default;

        [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
            return !(*this == rhs);
        }

        BasicIterator& operator++() noexcept {
            assert(node_ != nullptr);
            node_ = node_->next_node;
            return *this;
        }

        BasicIterator operator++(int) noexcept {
            auto old_value(*this);
            ++(*this);
            return old_value;
        }

        [[nodiscard]] reference operator*() const noexcept {
            assert(node_ != nullptr);
            return node_->value;
        }

        [[nodiscard]] pointer operator->() const noexcept {
            assert(node_ != nullptr);
            return &node_->value;
        }
    private:
        Node* node_ = nullptr;
    };

public:

    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using Iterator = BasicIterator<Type>;
    using ConstIterator = BasicIterator<const Type>;

    [[nodiscard]] Iterator begin() noexcept {
        return Iterator{ head_.next_node };
    }

    [[nodiscard]] Iterator end() noexcept {
        Node* end_node = nullptr;
        return Iterator{ end_node };
    }

    [[nodiscard]] ConstIterator begin() const noexcept {
        return cbegin();
    }

    [[nodiscard]] ConstIterator end() const noexcept {
        return cend();
    }

    [[nodiscard]] ConstIterator cbegin() const noexcept {
        return ConstIterator{ head_.next_node };
    }

    [[nodiscard]] ConstIterator cend() const noexcept {
        Node* end_node = nullptr;
        return ConstIterator{ end_node };
    }

    [[nodiscard]] Iterator before_begin() noexcept {
        return Iterator{ &head_ };
    }

    [[nodiscard]] ConstIterator before_begin() const noexcept {
        return cbefore_begin();
    }

    [[nodiscard]] ConstIterator cbefore_begin() const noexcept {
        return ConstIterator{ const_cast<Node*>(&head_) };
    }

    SingleLinkedList(std::initializer_list<Type> values) {
        Assign(values.begin(), values.end());  // Может бросить исключение
    }

    SingleLinkedList(const SingleLinkedList& other) {
        assert(size_ == 0 && head_.next_node == nullptr);
        SingleLinkedList other_copy;
        Node* other_ptr = other.head_.next_node;
        Node* othercopy_head_ptr = &other_copy.head_;
        while (other_ptr) {
            othercopy_head_ptr->next_node = new Node(other_ptr->value, nullptr);
            othercopy_head_ptr = othercopy_head_ptr->next_node;
            other_ptr = other_ptr->next_node;
        }
        other_copy.size_ = other.size_;
        swap(other_copy);

        other_ptr = nullptr;
        delete other_ptr;
        othercopy_head_ptr = nullptr;
        delete othercopy_head_ptr;
    }

    SingleLinkedList() : size_(0) {}

    ~SingleLinkedList() {
        Clear();
    }

    SingleLinkedList& operator=(const SingleLinkedList& rhs) {
        if (this != &rhs) {
            auto rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }

    void swap(SingleLinkedList& other) noexcept {
        Node* other_head_ptr = other.head_.next_node;
        size_t other_size = other.size_;

        other.head_.next_node = head_.next_node;
        other.size_ = size_;

        this->head_.next_node = other_head_ptr;
        this->size_ = other_size;

        other_head_ptr = nullptr;
        delete other_head_ptr;
    }

    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }

    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    void PushFront(const Type& value) {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }

    void PopFront() noexcept {
        if (!IsEmpty()) {
            Node* tmp = head_.next_node;
            head_.next_node = head_.next_node->next_node;

            delete tmp;
            --size_;
        }
    }

    Iterator InsertAfter(ConstIterator pos, const Type& value) {
        Node* temp = new Node(value, pos.node_->next_node);
        pos.node_->next_node = temp;
        ++size_;

        temp = nullptr;
        delete temp;

        return Iterator{ pos.node_->next_node };
    }

    Iterator EraseAfter(ConstIterator pos) noexcept {
        if (pos.node_->next_node) {
            Node* temp = pos.node_->next_node;
            pos.node_->next_node = pos.node_->next_node->next_node;
            delete temp;
            --size_;
        }
        return Iterator{ pos.node_->next_node };
    }

    void Clear() noexcept {
        while (head_.next_node) {
            Node* temp = head_.next_node;
            head_.next_node = head_.next_node->next_node;
            delete temp;
            temp = nullptr;
        }
        size_ = 0;
    }

private:

    template <typename InputIterator>
    void Assign(InputIterator from, InputIterator to) {
        // Создаём временный список, в который будем добавлять элементы из диапазона [from, to)
        // Если в процессе добавления будет выброшено исключение,
        // его деструктор подчистит всю память
        SingleLinkedList<Type> tmp;

        // Элементы будут записываться начиная с указателя на первый узел
        Node** node_ptr = &tmp.head_.next_node;
        while (from != to) {
            // Ожидается, что текущий указатель - нулевой
            assert(*node_ptr == nullptr);

            // Создаём новый узел и записываем его адрес в указатель текущего узла
            *node_ptr = new Node(*from, nullptr);
            ++tmp.size_;

            // Теперь node_ptr хранит адрес указателя на следующий узел
            node_ptr = &((*node_ptr)->next_node);

            // Переходим к следующему элементу диапазона
            ++from;
        }

        // Теперь, когда tmp содержит копию элементов диапазона [from, to),
        // можно совершить обмен данными текущего объекта и tmp
        swap(tmp);
        // Теперь текущий список содержит копию элементов диапазона [from, to),
        // а tmp - прежнее значение текущего списка
    }

    Node head_;
    size_t size_ = 0;
};

template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }

    return std::equal(std::execution::par, lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::lexicographical_compare(std::execution::par, lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs < rhs);
}

