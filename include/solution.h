//
// Created by benia on 27/11/2025.
//

#pragma once
#include "threadsafe_vector.h"

template <typename T>
class solution : public threadsafe_vector<T> {
public:
    solution() = default;

    explicit solution(const size_t size): threadsafe_vector<T>(size) {};

    explicit solution(const solution& other) {
        std::lock_guard<std::mutex> lk(other.mut);
        this->data_vector = other.data_vector;
    }

    explicit solution(const std::vector<T>& other) {
        this->data_vector = other;
    }

    solution& operator=(const solution& other) {
        if (this == &other) return *this;

        // Lock both mutexes without deadlock
        std::scoped_lock lk(this->mut, other.mut);
        this->data_vector = other.data_vector;
        return *this;
    }

    solution& operator=(const std::vector<T>& other) {
        std::lock_guard<std::mutex> lk(this->mut);
        this->data_vector = other;
        return *this;
    }

    bool update_solution(const fixed_vector<int>& K, size_t bi) {
        std::lock_guard<std::mutex> lk(this->mut);
        if (K.size()+1 <= this->data_vector.size()) return false;

        this->data_vector = K;
        this->data_vector.push_back(bi);
        this->data_cond.notify_one();
        return true;
    }
};
