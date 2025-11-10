//
// Created by benia on 10/11/2025.
//

#pragma once

#include <memory>
#include <vector>

template<typename T, typename Alloc = std::allocator<T>>
class fixed_vector {
    std::vector<T, Alloc> data_;
    size_t size_;

public:
    explicit fixed_vector(size_t capacity)
        : data_(capacity), size_(0) {}

    [[nodiscard]] size_t size() const {
        return size_;
    }

    [[nodiscard]] size_t max_size() const {
        return data_.size();
    }

    T& operator[](const size_t index) { return data_[index]; };
    const T& operator[](const size_t index) const { return data_[index]; };

    void push_back(const T& value) {
        assert(size_ < data_.size());
        data_[size_++] = value;
    }

    void pop_back() {
        size_--;
    }

    void clear() {
        size_ = 0;
    }

    void resize(const size_t capacity) {
        assert(capacity <= data_.size());
        size_ = capacity;
    }
};

