//
// Created by benia on 26/11/2025.
//

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <atomic>

template<typename T, typename Container = std::vector<T>, typename Compare = std::less<T>>
class threadsafe_priority_queue {
    mutable std::mutex mut;
    std::priority_queue<T, Container, Compare> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_priority_queue() = default;

    // Copy constructor
    threadsafe_priority_queue(const threadsafe_priority_queue& other) {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;      // copy heap state
    }

    // Copy assignment
    threadsafe_priority_queue& operator=(const threadsafe_priority_queue& other) {
        if (this == &other) return *this;

        std::scoped_lock lock(mut, other.mut);
        data_queue = other.data_queue;
        return *this;
    }

    // Push new item
    void push(T new_value) {
        {
            std::lock_guard<std::mutex> lk(mut);
            data_queue.push(std::move(new_value));
        }
        data_cond.notify_one();
    }

    void wake_all() {
        std::lock_guard<std::mutex> lk(mut);
        data_cond.notify_all();
    }

    // Wait until non-empty and pop
    bool wait_and_pop(T& value, std::atomic_bool& done, const std::function<void()> &before_pop = {}) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [&done, this]{ return !data_queue.empty() || done; });

        if (done) return false;

        if (before_pop) before_pop();
        value = std::move(const_cast<T&>(data_queue.top()));
        data_queue.pop();
        return true;
    }

    // Wait until non-empty and pop (returns shared_ptr)
    std::shared_ptr<T> wait_and_pop(std::atomic_bool& done, const std::function<void()> &before_pop = {}) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [&done, this]{ return !data_queue.empty() || done; });
        
        if (done) return nullptr;

        if (before_pop) before_pop();
        auto res = std::make_shared<T>(std::move(const_cast<T&>(data_queue.top())));
        data_queue.pop();
        return res;
    }

    // Try to pop (returns bool)
    bool try_pop(T& value, const std::function<void()> &before_pop = {}) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;

        if (before_pop) before_pop();
        value = std::move(const_cast<T&>(data_queue.top()));
        data_queue.pop();
        return true;
    }

    // Try to pop (returns shared_ptr)
    std::shared_ptr<T> try_pop(const std::function<void()> &before_pop = {}) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return nullptr;

        if (before_pop) before_pop();
        auto res = std::make_shared<T>(std::move(const_cast<T&>(data_queue.top())));
        data_queue.pop();
        return res;
    }

    bool try_pop_condition(T& value, const std::function<bool(const T&)> &condition, const std::function<void()> &before_pop = {}) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;

        if (!condition(data_queue.top())) return false;
        if (before_pop) before_pop();
        value = std::move(const_cast<T&>(data_queue.top()));
        data_queue.pop();
        return true;
    }

    // Try to pop (returns shared_ptr)
    std::shared_ptr<T> try_pop_condition(const std::function<bool(T)> &condition, const std::function<void()> &before_pop = {}) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return nullptr;

        if (!condition(data_queue.top())) return false;
        if (before_pop) before_pop();
        auto res = std::make_shared<T>(std::move(const_cast<T&>(data_queue.top())));
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
