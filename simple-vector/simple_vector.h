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

    explicit SimpleVector(size_t size) : items_(size) {
        size_ = size;
        capacity_ = size;
        std::fill(&items_[0], &items_[size_], 0);
    }
    SimpleVector(ReserveProxyObj size) {
        capacity_ = size.Get();
    }

    SimpleVector(size_t size, const Type& value) : items_(size) {
        if (size != 0) {
            capacity_ = size;
            size_ = size;
            std::fill(&items_[0], &items_[size_], value);
        }
    }

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

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index >= size");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index >= size");
        }
        return items_[index];
    }

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
            std::move(&items_[0], &items_[size_], tmp.Get());
            tmp[size_++] = std::move(item);
            items_.swap(tmp);
        }
    }

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

    void PopBack() noexcept {
        if (size_ > 0) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        size_t index = static_cast<size_t>(pos - begin());
        std::move(begin() + index + 1, end(), begin() + index);
        --size_;
        return begin() + index;
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);
    }

    Iterator begin() noexcept {
        return &items_[0];
    }

    Iterator end() noexcept {
        return &items_[size_];
    }

    ConstIterator begin() const noexcept {
        return &items_[0];
    }

    ConstIterator end() const noexcept {
        return &items_[size_];
    }

    ConstIterator cbegin() const noexcept {
        return const_cast<Type*>(& items_[0]);
    }

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