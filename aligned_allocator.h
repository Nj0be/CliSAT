//
// Created by benia on 11/09/2025.
//

#pragma once

#include <bit>
#include <cstddef>
#include <limits>

// C++23 aligned allocator for standard containers.
// Usage:
//   aligned_vector<std::uint64_t, 32> v(1000);
//   auto* p = std::assume_aligned<32>(v.data()); // Safe: allocator guarantees alignment
//
// Guarantees: v.data() is aligned to at least std::max(Align, alignof(T)).

// Primary allocator
template <class T, std::size_t alignment>
// has_single_bit => power of 2
requires (alignment >= alignof(T)) && (std::has_single_bit(alignment))
class aligned_allocator {
public:
    using value_type                             = T;
    using size_type                              = std::size_t;
    using difference_type                        = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal                        = std::true_type; // stateless

    template <class U> struct rebind { using other = aligned_allocator<U, alignment>; };

    constexpr aligned_allocator() noexcept = default;
    template <class U>
    constexpr explicit aligned_allocator(const aligned_allocator<U, alignment>&) noexcept {}

    [[nodiscard]] value_type* allocate(const size_type n) {
        if (n == 0) return nullptr;
        if (n > max_size()) throw std::bad_array_new_length();

        const std::size_t bytes = n * sizeof(T);
        auto* p = static_cast<value_type*>(std::aligned_alloc(alignment, bytes));
        return p;
    }

    // C++23 optional allocate_at_least: returns ptr + possibly larger count.
    // Containers that consult allocator_traits::allocate_at_least can exploit this.
    struct allocation_result {
        value_type* ptr;
        size_type count;
    };

    [[nodiscard]] allocation_result allocate_at_least(const size_type n) {
        // Simple: no over-allocation strategy; always exact.
        return { allocate(n), n };
    }

    static void deallocate(value_type* p, const size_type n) noexcept {
        (void)n; // size not needed for ::operator delete with alignment
        if (!p) return;
        ::operator delete(p, static_cast<std::align_val_t>(alignment));
    }

    // For completeness in generic contexts:
    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    // Standard says stateless allocators compare equal.
    template <class U>
    friend constexpr bool operator==(const aligned_allocator&,
                                     const aligned_allocator<U, alignment>&) noexcept {
        return true;
    }
    template <class U>
    friend constexpr bool operator!=(const aligned_allocator&,
                                     const aligned_allocator<U, alignment>&) noexcept {
        return false;
    }
};