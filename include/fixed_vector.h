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

    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;  // or also value_type*
        using reference         = T&;  // or also value_type&

        iterator(pointer ptr) : m_ptr(ptr) {}

        reference operator*() { return *m_ptr; }
        pointer operator->() { return m_ptr; }

        // Prefix increment
        iterator& operator++() { ++m_ptr; return *this; }

        // Postfix increment
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

        // Prefix decrement
        iterator& operator--() { --m_ptr; return *this; }

        // Postfix decrement
        iterator operator--(int) { iterator tmp = *this; --(*this); return tmp; }

        friend bool operator== (const iterator& a, const iterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const iterator& a, const iterator& b) { return a.m_ptr != b.m_ptr; };
    private:
        pointer m_ptr;
    };

    class const_iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = const T*;  // or also value_type*
        using reference         = const T&;  // or also value_type&

        const_iterator(const pointer ptr) : m_ptr(ptr) {}

        reference operator*() const { return *m_ptr; }
        pointer operator->() const { return m_ptr; }

        // Prefix increment
        const_iterator& operator++() { ++m_ptr; return *this; }

        // Postfix increment
        const_iterator operator++(int) { const_iterator tmp = *this; ++(*this); return tmp; }

        // Prefix decrement
        const_iterator& operator--() { --m_ptr; return *this; }

        // Postfix decrement
        const_iterator operator--(int) { const_iterator tmp = *this; --(*this); return tmp; }

        friend bool operator== (const const_iterator& a, const const_iterator& b) { return a.m_ptr == b.m_ptr; };
    private:
        pointer m_ptr;
    };

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

    operator std::vector<T>() const {
        std::vector<T> data_copy(data_.begin(), data_.begin() + size_);
        return data_copy;
    }

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

    iterator begin() { return iterator(&data_[0]); }
    const_iterator begin() const { return const_iterator(&data_[0]); }
    iterator end() { return iterator(&data_[size_]); } // size_ is out of bounds
    const_iterator end() const { return const_iterator(&data_[size_]); } // size_ is out of bounds
};