//
// Created by benia on 24/11/2025.
//

#pragma once
#include <atomic>
#include <functional>
#include <thread>

#include "threadsafe_priority_queue.h"
#include "threadsafe_queue.h"
#include "threadsafe_stack.h"

template<typename T>
class thread_pool {
public:
    struct Task {
        int depth;
        size_t sequence;
        std::function<void(T&, size_t)> func;
    };

private:
    struct TaskCompare {
        bool operator()(const Task& a, const Task& b) const {
            if (a.depth == b.depth)
                return a.sequence > b.sequence;  // FIFO for same priority
            return a.depth < b.depth;   // max depth = highest priority
        }
    };

    std::atomic_bool done = false;
    const size_t G_size;
    threadsafe_priority_queue<Task, std::vector<Task>, TaskCompare> work_queue;
    std::vector<std::jthread> threads;
    std::atomic_uint64_t threads_working = 0;
    std::atomic_uint64_t curr_sequence = 0;
    std::mutex bitset_borrow;
    std::deque<custom_bitset> bitset_pool;
    threadsafe_stack<size_t> bitset_stack;
    std::mutex ISs_borrow;
    std::deque<std::vector<custom_bitset>> ISs_pool;
    threadsafe_stack<size_t> ISs_stack;
    std::mutex color_class_borrow;
    std::deque<std::vector<int>> color_class_pool;
    threadsafe_stack<size_t> color_class_stack;
    std::mutex K_borrow;
    std::deque<fixed_vector<int>> K_pool;
    threadsafe_stack<size_t> K_stack;
    std::mutex u_borrow;
    std::deque<std::vector<int>> u_pool;
    threadsafe_stack<size_t> u_stack;
    std::mutex alpha_borrow;
    std::deque<fixed_vector<int>> alpha_pool;
    threadsafe_stack<size_t> alpha_stack;

    std::condition_variable work_done_cv;
    std::mutex work_done_m;

    void worker_thread() {
        T state = T(G_size);

        Task task;

        // we set thread to work before popping
        // important, we could quit before finishing if we don't set thread_working!
        while (work_queue.wait_and_pop(task, done, [this]() {
            ++threads_working;
        })) {
            task.func(state, task.sequence);
            --threads_working;

            if (!working()) {
                std::lock_guard lg(work_done_m);
                work_done_cv.notify_all();
            }
        }
    }

    bool working() const {
        if (!work_queue.empty() || threads_working) return true;
        return false;
    }

public:
    std::atomic_bool stop_threads = false;
    explicit thread_pool(const size_t G_size, const size_t thread_count) : G_size(G_size) {
        try {
            for (unsigned i = 0; i < thread_count; i++) {
                threads.emplace_back(&thread_pool::worker_thread, this);
            }
        } catch (...) {
            done = true;
            throw;
        }
    }

    ~thread_pool() {
        done = true;
        work_queue.wake_all();
    }

    bool get_task(Task& task) {
        return work_queue.try_pop(task);
    }

    bool get_higher_priority_task(Task& task, const int depth, const size_t sequence) {
        return work_queue.try_pop_condition(task, [depth, sequence](const Task& new_task) {
            if (new_task.depth == depth) return new_task.sequence < sequence;
            return new_task.depth > depth;
        });
    }

    size_t get_new_sequence() {
        return curr_sequence++;
    }

    template<typename FunctionType>
    void submit(const int depth, FunctionType f) {
        work_queue.push(Task(depth, curr_sequence, std::function<void(T&, size_t)>(f)));
        ++curr_sequence;
    }

    void wait_until_idle() {
        std::unique_lock lock(work_done_m);
        work_done_cv.wait(lock, [&]{ return !working(); });
        stop_threads = false;
    }

    bool all_threads_working() const {
        return threads_working == threads.size();
    }

    bool is_queue_full() {
        if (work_queue.size() >= threads.size()) return true;
        return false;
    }

    size_t borrow_bitset() {
        size_t idx;
        if (bitset_stack.try_pop(idx)) return idx;

        std::unique_lock lock(bitset_borrow);
        bitset_pool.emplace_back(G_size);
        return bitset_pool.size() - 1;
    }

    void give_back_bitset(const size_t index) {
        assert(index < bitset_pool.size());
        bitset_stack.push(index);
    }

    custom_bitset& get_bitset(const size_t index) {
        assert(index < bitset_pool.size());
        return bitset_pool[index];
    }

    size_t borrow_ISs() {
        size_t idx;
        if (ISs_stack.try_pop(idx)) return idx;

        std::unique_lock lock(ISs_borrow);
        ISs_pool.emplace_back();
        return ISs_pool.size() - 1;
    }

    void give_back_ISs(const size_t index) {
        assert(index < ISs_pool.size());
        ISs_stack.push(index);
    }

    std::vector<custom_bitset>& get_ISs(const size_t index) {
        assert(index < ISs_pool.size());
        return ISs_pool[index];
    }

    size_t borrow_color_class() {
        size_t idx;
        if (color_class_stack.try_pop(idx)) return idx;

        std::unique_lock lock(color_class_borrow);
        color_class_pool.emplace_back(G_size);
        return color_class_pool.size() - 1;
    }

    void give_back_color_class(const size_t index) {
        assert(index < color_class_pool.size());
        color_class_stack.push(index);
    }

    std::vector<int>& get_color_class(const size_t index) {
        assert(index < color_class_pool.size());
        return color_class_pool[index];
    }

    size_t borrow_K() {
        size_t idx;
        if (K_stack.try_pop(idx)) return idx;

        std::unique_lock lock(K_borrow);
        K_pool.emplace_back(G_size);
        return K_pool.size() - 1;
    }

    void give_back_K(const size_t index) {
        assert(index < K_pool.size());
        K_stack.push(index);
    }

    fixed_vector<int>& get_K(const size_t index) {
        assert(index < K_pool.size());
        return K_pool[index];
    }

    size_t borrow_u() {
        size_t idx;
        if (u_stack.try_pop(idx)) return idx;

        std::unique_lock lock(u_borrow);
        u_pool.emplace_back(G_size);
        return u_pool.size() - 1;
    }

    void give_back_u(const size_t index) {
        assert(index < u_pool.size());
        u_stack.push(index);
    }

    std::vector<int>& get_u(const size_t index) {
        assert(index < u_pool.size());
        return u_pool[index];
    }

    size_t borrow_alpha() {
        size_t idx;
        if (alpha_stack.try_pop(idx)) return idx;

        std::unique_lock lock(alpha_borrow);
        alpha_pool.emplace_back(G_size);
        return alpha_pool.size() - 1;
    }

    void give_back_alpha(const size_t index) {
        assert(index < alpha_pool.size());
        alpha_stack.push(index);
    }

    fixed_vector<int>& get_alpha(const size_t index) {
        assert(index < alpha_pool.size());
        return alpha_pool[index];
    }
};
