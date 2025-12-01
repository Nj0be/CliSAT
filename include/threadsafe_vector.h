//
// Created by benia on 25/11/2025.
//

#pragma once
#include <condition_variable>
#include <mutex>
#include <vector>

template<typename T>
class threadsafe_vector {
protected:
    mutable std::mutex mut;
    std::vector<T> data_vector;
    std::condition_variable data_cond;

public:
    threadsafe_vector() = default;

    explicit threadsafe_vector(const size_t size): data_vector(size) {};

    threadsafe_vector(const threadsafe_vector& other) {
        std::lock_guard<std::mutex> lk(other.mut);
        data_vector = other.data_vector;
    }

    explicit threadsafe_vector(const std::vector<T>& other) {
        data_vector = other;
    }

    operator std::vector<T>() const {
        std::lock_guard<std::mutex> lk(mut);

        return data_vector;
    }

    threadsafe_vector& operator=(const threadsafe_vector& other) {
        if (this == &other) return *this;

        // Lock both mutexes without deadlock
        std::scoped_lock lk(mut, other.mut);
        data_vector = other.data_vector;
        return *this;
    }

    threadsafe_vector& operator=(const std::vector<T>& other) {
        std::lock_guard<std::mutex> lk(mut);
        data_vector = other;
        return *this;
    }

    T& operator[](const size_t index) {
        assert(index < data_vector.size());

        std::lock_guard<std::mutex> lk(mut);
        return data_vector[index];
    }

    const T& operator[](const size_t index) const {
        assert(index < data_vector.size());

        std::lock_guard<std::mutex> lk(mut);
        return data_vector[index];
    }

    void push_back(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_vector.push_back(std::move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{ return !data_vector.empty(); });
        value = std::move(data_vector.back());
        data_vector.pop_back();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{ return !data_vector.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_vector.back())));
        data_vector.pop_back();
        return res;
    }

    bool try_pop_back(T& value) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_vector.empty())
            return false;
        value = std::move(data_vector.back());
        data_vector.pop_back();
        return true;
    }

    std::shared_ptr<T> try_pop_back() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_vector.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_vector.back())));
        data_vector.pop_back();
        return res;
    }

    void clear() {
        std::lock_guard<std::mutex> lk(mut);
        data_vector.clear();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_vector.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_vector.size();
    }
};
