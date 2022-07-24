#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <utility>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t cap) : capacity_(cap) {}
    size_t Get() {
        return capacity_;
    }
private:
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : items_(size) {
        size_ = size;
        capacity_ = size;
        std::fill(&items_[0], &items_[size_], 0);
    }
    SimpleVector(ReserveProxyObj size) {
        capacity_ = size.Get();
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : items_(size) {
        if (size != 0) {
            capacity_ = size;
            size_ = size;
            std::fill(&items_[0], &items_[size_], value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : items_(init.size()) {
        capacity_ = init.size();
        size_ = init.size();
        std::copy(init.begin(), init.end(), &items_[0]);
    }

    SimpleVector(const SimpleVector& other) {
        if (this != &other) {
            ArrayPtr<Type> tmp(other.GetCapacity());
            std::copy(other.begin(), other.end(), tmp.Get());
            items_.swap(tmp);
            size_ = other.GetSize();
            capacity_ = other.GetCapacity();
        }
    }

    SimpleVector(SimpleVector&& other) noexcept {
        if (this != &other) {
            capacity_ = std::exchange(other.capacity_,0);
            items_ = std::move(other.items_);
            size_ = std::exchange(other.size_,0);
        }
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index >= size");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index >= size");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(&items_[0], &items_[size_], &tmp[0]);
            items_.swap(tmp);
            capacity_ = new_capacity;
        }
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        }
        else if(new_size < capacity_) {
            std::fill(&items_[size_], &items_[new_size], 0);
            size_ = new_size;
        }
        else if (new_size > capacity_) {
            capacity_ = new_size * 2;
            ArrayPtr<Type> tmp(capacity_);
            std::move(&items_[0], &items_[size_], tmp.Get());
            items_.swap(tmp);
            size_ = new_size;
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (capacity_ == 0) {
            ArrayPtr<Type> tmp(1);
            items_.swap(tmp);
            capacity_ = 1;
        }
        if (size_ < capacity_) {
            items_[size_++] = item;
        }
        else {
            capacity_ *= 2;
            ArrayPtr<Type> tmp(capacity_);
            std::copy(&items_[0], &items_[size_], tmp.Get());
            tmp[size_++] = item;
            items_.swap(tmp);
        }
    }

    void PushBack(Type&& item) {
        if (capacity_ == 0) {
            ArrayPtr<Type> tmp(1);
            items_.swap(tmp);
            capacity_ = 1;
        }
        if (size_ < capacity_) {
            items_[size_++] = std::move(item);
        }
        else {
            capacity_ *= 2;
            ArrayPtr<Type> tmp(capacity_);
            tmp =  std::move(items_);
            tmp[size_++] = std::move(item);
            items_.swap(tmp);
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = static_cast<size_t>(pos - begin());
        if (pos == end()) {
            PushBack(value);
            return &items_[index];
        }
        if (size_ < capacity_) {
            ArrayPtr<Type> tmp(capacity_);
            std::copy(&items_[0], &items_[index], tmp.Get());
            tmp[index] = std::move(value);
            std::copy_backward(&items_[index], &items_[size_], &tmp[size_+1]);
            items_.swap(tmp);
            ++size_;
            return &items_[index];
        }
        else {
            capacity_ *= 2;
            ArrayPtr<Type> tmp(capacity_);
            std::copy(&items_[0], &items_[index], tmp.Get());
            tmp[index] = std::move(value);
            std::copy_backward(&items_[index], &items_[size_], &tmp[size_ + 1]);
            items_.swap(tmp);
            ++size_;
            return &items_[index];
        }
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = static_cast<size_t>(pos - begin());
        if (pos == end()) {
            PushBack(std::move(value));
            return &items_[index];
        }
        if (size_ < capacity_) {
            ArrayPtr<Type> tmp(capacity_);
            std::move(&items_[0], &items_[index], tmp.Get());
            tmp[index] = std::move(value);
            std::move_backward(&items_[index], &items_[size_], &tmp[size_ + 1]);
            items_.swap(tmp);
            ++size_;
            return &items_[index];
        }
        else {
            capacity_ *= 2;
            ArrayPtr<Type> tmp(capacity_);
            std::move(&items_[0], &items_[index], tmp.Get());
            tmp[index] = std::move(value);
            std::move_backward(&items_[index], &items_[size_], &tmp[size_ + 1]);
            items_.swap(tmp);
            ++size_;
            return &items_[index];
        }
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ > 0) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        size_t index = static_cast<size_t>(pos - begin());
        std::move(begin() + index + 1, end(), begin() + index);
        --size_;
        return begin() + index;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return &items_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return &items_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return &items_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return &items_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return const_cast<Type*>(& items_[0]);
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return const_cast<Type*>(&items_[size_]);
    }
private:
    size_t size_{};
    size_t capacity_{};
    ArrayPtr<Type> items_;
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.begin(),lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),rhs.begin(),rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs >= lhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs|| rhs == lhs;
}