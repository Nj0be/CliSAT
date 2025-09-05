//
// Created by benia on 02/09/2025.
//

module;

#include<cstdint>
#include<bit>
#include<immintrin.h>

export module instructions;

inline bool has_popcnt() {
    unsigned eax, ebx, ecx, edx;
    eax = 1;
    __asm__ __volatile__("cpuid"
                         : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                         : "a"(eax));
    return (ecx >> 23) & 1; // POPCNT bit
}

unsigned constexpr popcnt_sw(const uint64_t x) noexcept { return std::popcount(x); }
unsigned constexpr popcnt_hw(const uint64_t x) noexcept {
    uint64_t result;
    asm ("popcnt %1, %0" : "=r"(result) : "r"(x));
    return result;
}

inline unsigned (*popcount_func)(uint64_t) = nullptr;

export constexpr unsigned popcount(const uint64_t x) noexcept {
    return popcount_func(x);
}

/*
static size_type bit_scan_forward(const block_type x) { return __builtin_ctzll(x); }
static size_type bit_scan_reverse(const block_type x) { return __bsrq(x); }
*/

export inline std::size_t bit_scan_forward(const uint64_t x) {
    std::size_t result;
    asm ("bsfq %1, %0"
        : "=r"(result)
        : "r"(x)
        : "cc"
    );
    return result;
}

export inline std::size_t bit_scan_reverse(const uint64_t x) {
    std::size_t result;
    asm ("bsrq %1, %0"
        : "=r"(result)
        : "r"(x)
        : "cc"
    );
    return result;
}

export inline uint_fast32_t bit_set(uint_fast32_t x, const std::size_t n) {
    asm( "bts %1,%0": "+rm"(x) : "r"(n));
    return x;
}

export inline uint_fast32_t bit_reset(uint_fast32_t x, const std::size_t n) {
    asm( "btr %1,%0": "+rm"(x) : "r"(n));
    return x;
}

export inline uint_fast32_t bit_complement(uint_fast32_t x, const std::size_t n) {
    asm( "btc %1,%0": "+rm"(x) : "r"(n));
    return x;
}

export inline bool bit_test(const uint_fast32_t x, const std::size_t n) {
    /*
    unsigned char result;

    asm("bt %2, %1\n\t"   // btq = 64-bit version
        "setc %0"          // set result = CF (carry flag)
        : "=r"(result)     // output in a general register
        : "r"(x), "r"(n) // inputs: x, index
        : "cc"             // clobbers condition codes
    );

    return result;
*/
    return (x >> n) & 1;
}

export inline void initialize() {
    popcount_func = has_popcnt() ? popcnt_hw : popcnt_sw;
}

