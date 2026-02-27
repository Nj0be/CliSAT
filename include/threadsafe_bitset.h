//
// Created by benia on 26/02/2026.
//

#pragma once
#include <condition_variable>
#include <mutex>

#include "custom_bitset.h"

class threadsafe_bitset {
protected:
    mutable std::mutex mut;
    custom_bitset bitset;
    std::condition_variable data_cond;

public:
    threadsafe_bitset() = default;

    explicit threadsafe_bitset(const size_t size): bitset(size) {};

    threadsafe_bitset(const threadsafe_bitset& other) {
        std::lock_guard lk(other.mut);
        bitset = other.bitset;
    }

    explicit threadsafe_bitset(const custom_bitset& other) {
        bitset = other;
    }

    operator custom_bitset() const {
        std::lock_guard lk(mut);

        return bitset;
    }

    threadsafe_bitset& operator=(const threadsafe_bitset& other) {
        if (this == &other) return *this;

        // Lock both mutexes without deadlock
        std::scoped_lock lk(mut, other.mut);
        bitset = other.bitset;
        return *this;
    }

    threadsafe_bitset& operator=(const custom_bitset& other) {
        std::lock_guard lk(mut);
        bitset = other;
        return *this;
    }

    bool operator[](const custom_bitset::reference& ref) const {
        std::lock_guard lk(mut);
        return bitset.test(ref);
    };
    bool operator[](const custom_bitset::size_type pos) const {
        std::lock_guard lk(mut);
        return bitset.test(pos);
    };
    custom_bitset::iterator operator[](const custom_bitset::reference& ref) {
        std::lock_guard lk(mut);
        return bitset[ref];
    };
    custom_bitset::iterator operator[](const custom_bitset::size_type pos) {
        std::lock_guard lk(mut);
        return bitset[pos];
    };

    void set(const custom_bitset::reference& ref) {
        std::lock_guard lk(mut);
        bitset.set(ref);
    }

    void set(const custom_bitset::size_type pos) {
        std::lock_guard lk(mut);
        bitset.set(pos);
    }

    void reset() {
        std::lock_guard lk(mut);
        bitset.reset();
    }

    bool none() const {
        std::lock_guard lk(mut);
        return bitset.none();
    }

    bool any() const {
        std::lock_guard lk(mut);
        return bitset.any();
    }

    size_t size() const {
        std::lock_guard lk(mut);
        return bitset.size();
    }
};
