#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <utility>

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

    // Шаблон класса «Базовый Итератор».
    // Определяет поведение итератора на элементы односвязного списка
    // ValueType — совпадает с Type (для Iterator) либо с const Type (для ConstIterator)
    template <typename ValueType>
    class BasicIterator {
        // Класс списка объявляется дружественным, чтобы из методов списка
        // был доступ к приватной области итератора
        friend class SingleLinkedList;

        // Конвертирующий конструктор итератора из указателя на узел списка
        explicit BasicIterator(Node* node)
            : node_(node) {
        }

    public:
        // Объявленные ниже типы сообщают стандартной библиотеке о свойствах этого итератора

        // Категория итератора — forward iterator
        // (итератор, который поддерживает операции инкремента и многократное разыменование)
        using iterator_category = std::forward_iterator_tag;
        // Тип элементов, по которым перемещается итератор
        using value_type = Type;
        // Тип, используемый для хранения смещения между итераторами
        using difference_type = std::ptrdiff_t;
        // Тип указателя на итерируемое значение
        using pointer = ValueType*;
        // Тип ссылки на итерируемое значение
        using reference = ValueType&;

        BasicIterator() = default;

        // Конвертирующий конструктор/конструктор копирования
        // При ValueType, совпадающем с Type, играет роль копирующего конструктора
        // При ValueType, совпадающем с const Type, играет роль конвертирующего конструктора
        BasicIterator(const BasicIterator<Type>& other) noexcept
            : BasicIterator(other.node_) {
        }

        // Чтобы компилятор не выдавал предупреждение об отсутствии оператора = при наличии
        // пользовательского конструктора копирования, явно объявим оператор = и
        // попросим компилятор сгенерировать его за нас
        BasicIterator& operator=(const BasicIterator& rhs) = default;

        // Оператор сравнения итераторов (в роли второго аргумента выступает константный итератор)
        // Два итератора равны, если они ссылаются на один и тот же элемент списка либо на end()
        [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        // Оператор проверки итераторов на неравенство
        // Противоположен !=
        [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
            return !(node_ == rhs.node_);
        }

        // Оператор сравнения итераторов (в роли второго аргумента итератор)
        // Два итератора равны, если они ссылаются на один и тот же элемент списка либо на end()
        [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        // Оператор проверки итераторов на неравенство
        // Противоположен !=
        [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
            return !(node_ == rhs.node_);
        }

        // Оператор прединкремента. После его вызова итератор указывает на следующий элемент списка
        // Возвращает ссылку на самого себя
        // Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
        BasicIterator& operator++() noexcept {
            // Проверяем указывает ли начальный итератор на существующий элемент списка
            assert(node_ != nullptr);
            node_ = node_->next_node;
            return *this;
        }

        // Оператор постинкремента. После его вызова итератор указывает на следующий элемент списка
        // Возвращает прежнее значение итератора
        // Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
        BasicIterator operator++(int) noexcept {
            // Проверяем указывает ли начальный итератор на существующий элемент списка
            assert(node_ != nullptr);

            auto old_value(*this);
            ++(*this);
            return old_value;
        }

        // Операция разыменования. Возвращает ссылку на текущий элемент
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
        [[nodiscard]] reference operator*() const noexcept {
            // Проверяем указывает ли итератор на существующий элемент списка            
            assert(node_ != nullptr);

            return node_->value;
        }

        // Операция доступа к члену класса. Возвращает указатель на текущий элемент списка
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        [[nodiscard]] pointer operator->() const noexcept {
            // Проверяем указывает ли начальный итератор на существующий элемент списка
            assert(node_ != nullptr);

            return  { &(node_->value) };
        }

    private:
        Node* node_ = nullptr;
    };

public:
    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;

    // Итератор, допускающий изменение элементов списка
    using Iterator = BasicIterator<Type>;
    // Константный итератор, предоставляющий доступ для чтения к элементам списка
    using ConstIterator = BasicIterator<const Type>;

    // Конструктор по умолчанию
    SingleLinkedList() = default;

    SingleLinkedList(std::initializer_list<Type> values) {
        FillingListInSeries(values);        
    }

    // Конструктор копирования
    SingleLinkedList(const SingleLinkedList& other) {
        FillingListInSeries(other);
    }

    // Заполняет последовательно односвязный список из заданного контейнера у которого есть
    // begin() и end()
    template <typename Container>
    void FillingListInSeries(const Container& from) {
        // Сначала надо удостовериться, что текущий список пуст
        assert(IsEmpty());

        // Временный список в который будем добавлять элементы из контейнера from
        SingleLinkedList<Type> temp_list;
        // Итератор указывающий на добавленный элемент
        SingleLinkedList<Type>::Iterator temp_it(temp_list.before_begin());

        // Поэлементо добавляем элементы из other во временный список
        for (auto value : from) {
            temp_list.InsertAfter(temp_it, value);
            ++temp_it;
        }

        // После того как элементы скопированы, обмениваем данные текущего списка и tmp
        swap(temp_list);
        // Теперь tmp пуст, а текущий список содержит копию элементов other
    }

    // Деструктор, очищает всю выделеную память
    ~SingleLinkedList() {
        Clear();
    }

    // Вставляет элемент value в начало списка за время O(1)
    void PushFront(const Type& value) {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }

    // Вставляет элемент value после элемента, на который указывает pos
    // Возвращает итератор на вставленный элемент
    // Если при создании элемента будет выброшено исключение, список останется в прежнем состоянии    
    Iterator InsertAfter(ConstIterator pos, const Type& value) {
        assert(pos.node_ != nullptr);
        // Сохраняем адрес ячейки которая будет после добавляемого элемента
        SingleLinkedList<Type>::Node* node_after_added = pos.node_->next_node;
        // Создаём новый элемент, который указывает на следующий за pos элемент
        SingleLinkedList<Type>::Node* new_node = new Node(value, node_after_added);
        // Меняем указатель элемента pos на новый элемент
        pos.node_->next_node = new_node;

        ++size_;
        return Iterator(pos.node_->next_node);
    }

    // Удаляет элемент, следующий за pos.
    // Возвращает итератор на элемент, следующий за удалённым
    Iterator EraseAfter(ConstIterator pos) noexcept {
        // Проверяем что список не пуст
        assert(!IsEmpty());
        // Сохраняем указатель на элемент который следует за удалённым
        SingleLinkedList<Type>::Node* node_after_deleted = pos.node_->next_node->next_node;
        // Удаляем нод следующий за pos
        delete pos.node_->next_node;
        pos.node_->next_node = node_after_deleted;
       
        --size_;
        return Iterator(node_after_deleted);
    }

    // Удаляет первый элемента непустого списка за время O(1)
    void PopFront() noexcept {
        // Проверяем что список не пуст
        assert(!IsEmpty());

        SingleLinkedList<Type>::Node* node_to_del = head_.next_node;
        head_.next_node = head_.next_node->next_node;

        --size_;
        delete node_to_del;
    }

    // Очищает список за время O(N)
    void Clear() noexcept {
        while (head_.next_node != nullptr) {
            Node* node_to_del = head_.next_node;
            head_.next_node = head_.next_node->next_node;
            delete node_to_del;
        }
        size_ = 0;
    }

    // Возвращает количество элементов в списке за время O(1)
    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }

    // Сообщает, пустой ли список за время O(1)
    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Оператор присваивания
    SingleLinkedList& operator=(const SingleLinkedList& rhs) {
        // Проверяем не присваивается ли объект сам себе
        if (this == &rhs) {
            return *this;
        }
        SingleLinkedList tmp(rhs);
        this->swap(tmp);

        return *this;
    }

    // Обменивает содержимое списков за время O(1)
    void swap(SingleLinkedList& other) noexcept {
        // Меняем указатели на первые элементы местами
        std::swap(this->head_.next_node, other.head_.next_node);
        // Меняем размеры местами
        std::swap(this->size_, other.size_);
    }

    // Возвращает итератор, ссылающийся на первый элемент, если список пустой, возвращённый итератор будет равен end()
    [[nodiscard]] Iterator begin() noexcept {
        return Iterator(head_.next_node);
    }

    // Возвращает итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    [[nodiscard]] Iterator end() noexcept {
        return Iterator(nullptr);
    }

    // Возвращает константный итератор, ссылающийся на первый элемент, если список пустой, возвращённый итератор будет равен end()
    [[nodiscard]] ConstIterator begin() const noexcept {
        return ConstIterator(head_.next_node);
    }

    // Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    [[nodiscard]] ConstIterator end() const noexcept {
        return ConstIterator(nullptr);
    }

    // Возвращает константный итератор, ссылающийся на первый элемент, если список пустой, возвращённый итератор будет равен cend()
    [[nodiscard]] ConstIterator cbegin() const noexcept {
        return ConstIterator(head_.next_node);
    }

    // Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    [[nodiscard]] ConstIterator cend() const noexcept {
        return ConstIterator(nullptr);
    }

    // Возвращает итератор, указывающий на позицию перед первым элементом односвязного списка
    [[nodiscard]] Iterator before_begin() noexcept {
        return Iterator(&head_);
    }

    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка
    [[nodiscard]] ConstIterator cbefore_begin() const noexcept {
        return ConstIterator(const_cast<Node*>(&head_));
    }

    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка
    // не очень понятно зачем он тут нужен
    [[nodiscard]] ConstIterator before_begin() const noexcept {
        return ConstIterator(const_cast<Node*>(&head_));
    }

private:
    // Фиктивный узел, используется для вставки "перед первым элементом"
    Node head_ = {};
    size_t size_ = 0;
};

// Меняет местами данные lhs и rhs
template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

// Операторы сравнения
template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(rhs < lhs);
}