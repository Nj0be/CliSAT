//
// Created by benia on 24/11/2025.
//

#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class threadsafe_queue {
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue() {}

    threadsafe_queue(const threadsafe_queue& other) {
        std::lock_guard<std::mutex> lk(other.mut); // lock source
        data_queue = other.data_queue;             // copy the queue
        // mut and data_cond are default-constructed
    }

    threadsafe_queue& operator=(const threadsafe_queue& other) {
        if (this == &other) return *this; // self-assignment check

        // Lock both mutexes without deadlock (C++17)
        std::scoped_lock lock(mut, other.mut);

        // Copy the underlying queue
        data_queue = other.data_queue;

        // data_cond is left default-constructed
        return *this;
    }

    void push(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{ return !data_queue.empty(); });
        value = std::move(data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{ return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value, const std::function<void()> &before_pop = {}) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;

        if (before_pop) before_pop();
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop(const std::function<void()> &before_pop = {}) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();

        if (before_pop) before_pop();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }
};
