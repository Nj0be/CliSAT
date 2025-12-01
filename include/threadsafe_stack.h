//
// Created by benia on 24/11/2025.
//

#pragma once

#pragma once
#include <condition_variable>
#include <mutex>
#include <stack>

template<typename T>
class threadsafe_stack {
    mutable std::mutex mut;
    std::stack<T> data_stack;
    std::condition_variable data_cond;

public:
    threadsafe_stack() {}

    void push(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_stack.push(std::move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{ return !data_stack.empty(); });
        value = std::move(data_stack.top());
        data_stack.pop();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{ return !data_stack.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_stack.top())));
        data_stack.pop();
        return res;
    }

    bool try_pop(T& value, const std::function<void()> &before_pop = {}) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_stack.empty())
            return false;

        if (before_pop) before_pop();
        value = std::move(data_stack.top());
        data_stack.pop();
        return true;
    }

    std::shared_ptr<T> try_pop(const std::function<void()> &before_pop = {}) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_stack.empty())
            return std::shared_ptr<T>();

        if (before_pop) before_pop();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_stack.top())));
        data_stack.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_stack.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_stack.size();
    }
};
