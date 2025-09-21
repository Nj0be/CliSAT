//
// Created by benia on 02/09/2025.
//

#pragma once

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include <memory>
#include <cassert>
#include <bit>

namespace instructions {
    struct features {
        bool popcnt = false;
        bool sse2 = false;
        bool sse4_1 = false;
        bool avx = false;
        bool avx2 = false;
        bool avx512f = false;
    };

    static inline void cpuid(std::uint32_t basic_leaf, std::uint32_t extended_leaf,
        std::uint32_t& eax, std::uint32_t& ebx, std::uint32_t& ecx, std::uint32_t& edx) noexcept {
#if defined(_MSC_VER)
        int regs[4];
        __cpuidex(regs, static_cast<int>(basic_leaf), static_cast<int>(extended_leaf));
        eax = static_cast<std::uint32_t>(regs[0]);
        ebx = static_cast<std::uint32_t>(regs[1]);
        ecx = static_cast<std::uint32_t>(regs[2]);
        edx = static_cast<std::uint32_t>(regs[3]);
#else
        asm volatile("cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(basic_leaf), "c"(extended_leaf));
#endif
    }

    static inline features cpu_supports_impl() {
        features cpu_supports;
        std::uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

        cpuid(0, 0, eax, ebx, ecx, edx);
        const std::uint32_t max_basic_leaf = eax;

        cpuid(0x80000000, 0, eax, ebx, ecx, edx);
        const std::uint32_t max_extended_leaf = eax;

        if (max_basic_leaf == 0) return cpu_supports;
        cpuid(1, 0, eax, ebx, ecx, edx);
        cpu_supports.popcnt = (ecx >> 23) & 1;
        cpu_supports.sse2 = (edx >> 26) & 1;      // SSE2 is bit 26 of EDX, not ECX
        cpu_supports.sse4_1 = (ecx >> 19) & 1;    // SSE4.1 is bit 19 of ECX
        cpu_supports.avx = (ecx >> 28) & 1;

        if (max_basic_leaf < 7) return cpu_supports;

        cpuid(7, 0, eax, ebx, ecx, edx);
        cpu_supports.avx2 = (ebx >> 5) & 1;
        cpu_supports.avx512f = (ebx >> 16) & 1;

        return cpu_supports;
    }

    inline features cpu_supports = cpu_supports_impl();

    static inline std::size_t bsf32(std::uint32_t x) {
        assert (x != 0);
        [[assume(x != 0)]];

#if _MSC_VER
        unsigned long index;
        _BitScanForward(&index, x);
        x = index;
#else
        asm ("bsf %0, %0" : "=r" (x) : "0" (x));
#endif
        return x;
    }

    static inline std::size_t bsf64(std::uint64_t x) {
        assert (x != 0);
        [[assume(x != 0)]];

#if _MSC_VER
        unsigned long index;
        _BitScanForward64(&index, x);
		x = index;
#else
        asm("bsfq %0, %0" : "=r" (x) : "0" (x));
#endif
        return x;
    }

    /*
    inline std::size_t bsf128(const __uint128_t x) {
        const auto hi = static_cast<std::uint64_t>(x >> 64);
        const auto lo = static_cast<std::uint64_t>(x);
        if (lo) return bsf64(lo);
        return 64 + bsf64(hi);
    }
    */

    static inline std::size_t bsr32(std::uint32_t x) {
        assert (x != 0);
        [[assume(x != 0)]];

#if _MSC_VER
        unsigned long index;
        _BitScanReverse(&index, x);
        x = index;
#else
        asm ("bsr %0, %0" : "=r" (x) : "0" (x));
#endif
        return x;
    }

    static inline std::size_t bsr64(std::uint64_t x) {
        assert (x != 0);
        [[assume(x != 0)]];

#if _MSC_VER
        unsigned long index;
        _BitScanReverse64(&index, x);
        x = index;
#else
        asm("bsrq %0, %0" : "=r" (x) : "0" (x));
#endif
        return x;
    }

    /*
    inline std::size_t bsr128(const __uint128_t x) {
        const auto hi = static_cast<std::uint64_t>(x >> 64);
        const auto lo = static_cast<std::uint64_t>(x);
        if (hi) return 64 + bsr64(hi);
        return bsr64(lo);
    }
    */

    template <std::unsigned_integral T>
    std::size_t bit_scan_forward(T x) {
        if constexpr (sizeof(T) <= 4) return bsf32(static_cast<std::uint32_t>(x));
        else if constexpr (sizeof(T) == 8) return bsf64(static_cast<std::uint64_t>(x));
        //else if constexpr (sizeof(T) == 16) return bsf128(static_cast<__uint128_t>(x));
    }

    template <std::unsigned_integral T>
    std::size_t bit_scan_reverse(T x) {
        if constexpr (sizeof(T) <= 4) return bsr32(static_cast<std::uint32_t>(x));
        else if constexpr (sizeof(T) == 8) return bsr64(static_cast<std::uint64_t>(x));
        //else if constexpr (sizeof(T) == 16) return bsr128(static_cast<__uint128_t>(x));
    }

    static std::size_t constexpr popcnt32_sw(const std::uint32_t x) noexcept { return std::popcount(x); }
    static std::size_t constexpr popcnt64_sw(const std::uint64_t x) noexcept { return std::popcount(x); }
    //std::size_t constexpr popcnt128_sw(const __uint128_t x) noexcept { return std::popcount(x); }

    static std::size_t popcnt32_hw(std::uint32_t x) noexcept {
#if _MSC_VER
		x = __popcnt(x);
#else
        asm ("popcnt %0, %0" : "=r"(x) : "r"(x));
#endif
        return x;
    }

    static std::size_t popcnt64_hw(std::uint64_t x) noexcept {
#if _MSC_VER
        x = __popcnt64(x);
#else
        asm("popcnt %0, %0" : "=r"(x) : "r"(x));
#endif
        return x;
    }

    /*
    std::size_t constexpr popcnt128_hw(const __uint128_t x) noexcept {
        const auto hi = static_cast<std::uint64_t>(x >> 64);
        const auto lo = static_cast<std::uint64_t>(x);
        return popcnt64_hw(lo) + popcnt64_hw(hi);
    }
    */

    template <std::unsigned_integral T>
    std::size_t popcount(T x) noexcept {
        if (cpu_supports.popcnt) {
            if constexpr (sizeof(T) <= 4) return popcnt32_hw(static_cast<std::uint32_t>(x));
            else if constexpr (sizeof(T) == 8) return popcnt64_hw(static_cast<std::uint64_t>(x));
            //else return popcnt128_hw(static_cast<__uint128_t>(x));
        } else {
            if constexpr (sizeof(T) <= 4) return popcnt32_sw(static_cast<std::uint32_t>(x));
            else if constexpr (sizeof(T) == 8) return popcnt64_sw(static_cast<std::uint64_t>(x));
            //else return popcnt128_sw(static_cast<__uint128_t>(x));
        }
    }

    template <std::size_t alignment, std::unsigned_integral T>
    std::size_t popcount(const T* src, const std::size_t n) noexcept {
        src = std::assume_aligned<alignment>(src);

        std::size_t sum = 0;

        /*
        if (cpu_supports.popcnt)
            for (std::size_t i = 0; i < n; ++i)
                sum += popcnt64_hw(src[i]);
        else
            for (std::size_t i = 0; i < n; ++i)
                sum += popcnt64_sw(src[i]);
        */
       
        // faster thanks to branch prediction...
        // no need to inline two different loops
        for (std::size_t i = 0; i < n; ++i) {
            sum += popcount(src[i]);
        }

        return sum;
    }

    template <std::size_t alignment>
    void and_inplace(std::uint64_t* __restrict lhs, const std::uint64_t* __restrict rhs, const std::size_t n) {
        lhs = std::assume_aligned<alignment>(lhs);
        rhs = std::assume_aligned<alignment>(rhs);

        for (std::size_t i = 0; i < n; ++i)
            lhs[i] &= rhs[i];
    }

    template <std::size_t alignment>
    void or_inplace(std::uint64_t* __restrict lhs, const std::uint64_t* __restrict rhs, const std::size_t n) {
        lhs = std::assume_aligned<alignment>(lhs);
        rhs = std::assume_aligned<alignment>(rhs);

        for (std::size_t i = 0; i < n; ++i)
            lhs[i] |= rhs[i];
    }

    template <std::size_t alignment>
    void xor_inplace(std::uint64_t* __restrict lhs, const std::uint64_t* __restrict rhs, const std::size_t n) {
        lhs = std::assume_aligned<alignment>(lhs);
        rhs = std::assume_aligned<alignment>(rhs);

        for (std::size_t i = 0; i < n; ++i)
            lhs[i] ^= rhs[i];
    }

    template <std::size_t alignment>
    void diff_inplace(std::uint64_t* __restrict lhs, const std::uint64_t* __restrict rhs, const std::size_t n) {
        lhs = std::assume_aligned<alignment>(lhs);
        rhs = std::assume_aligned<alignment>(rhs);

        for (std::size_t i = 0; i < n; ++i)
            lhs[i] &= ~rhs[i];
    }

    template <std::size_t alignment>
    void and_store(std::uint64_t* __restrict dest, const std::uint64_t* __restrict src1, const std::uint64_t* __restrict src2, const std::size_t n) {
        dest = std::assume_aligned<alignment>(dest);
        src1 = std::assume_aligned<alignment>(src1);
        src2 = std::assume_aligned<alignment>(src2);

        for (std::size_t i = 0; i < n; ++i)
            dest[i] = src1[i] & src2[i];
    }

    template <std::size_t alignment>
    void and_store(
        std::uint64_t* __restrict dest,
        const std::uint64_t* __restrict src1,
        const std::uint64_t* __restrict src2,
        const std::size_t start,
        const std::size_t end
    ) {
        dest = std::assume_aligned<alignment>(dest);
        src1 = std::assume_aligned<alignment>(src1);
        src2 = std::assume_aligned<alignment>(src2);

        for (std::size_t i = start; i < end; ++i)
            dest[i] = src1[i] & src2[i];
    }

    template <std::size_t alignment>
    void or_store(
        std::uint64_t* __restrict dest, const std::uint64_t* __restrict src1, const std::uint64_t* __restrict src2, const std::size_t n) {
        dest = std::assume_aligned<alignment>(dest);
        src1 = std::assume_aligned<alignment>(src1);
        src2 = std::assume_aligned<alignment>(src2);

        for (std::size_t i = 0; i < n; ++i)
            dest[i] = src1[i] | src2[i];
    }

    template <std::size_t alignment>
    void or_store(
        std::uint64_t* __restrict dest,
        const std::uint64_t* __restrict src1,
        const std::uint64_t* __restrict src2,
        const std::size_t start,
        const std::size_t end
    ) {
        dest = std::assume_aligned<alignment>(dest);
        src1 = std::assume_aligned<alignment>(src1);
        src2 = std::assume_aligned<alignment>(src2);

        for (std::size_t i = start; i < end; ++i)
            dest[i] = src1[i] | src2[i];
    }


    template <std::size_t alignment>
    void xor_store(
        std::uint64_t* __restrict dest, const std::uint64_t* __restrict src1, const std::uint64_t* __restrict src2, const std::size_t n) {
        dest = std::assume_aligned<alignment>(dest);
        src1 = std::assume_aligned<alignment>(src1);
        src2 = std::assume_aligned<alignment>(src2);

        for (std::size_t i = 0; i < n; ++i)
            dest[i] = src1[i] ^ src2[i];
    }

    template <std::size_t alignment>
    void xor_store(
        std::uint64_t* __restrict dest,
        const std::uint64_t* __restrict src1,
        const std::uint64_t* __restrict src2,
        const std::size_t start,
        const std::size_t end
    ) {
        dest = std::assume_aligned<alignment>(dest);
        src1 = std::assume_aligned<alignment>(src1);
        src2 = std::assume_aligned<alignment>(src2);

        for (std::size_t i = start; i < end; ++i)
            dest[i] = src1[i] ^ src2[i];
    }

    template <std::size_t alignment>
    void diff_store(std::uint64_t* __restrict dest, const std::uint64_t* __restrict src1, const std::uint64_t* __restrict src2, const std::size_t n) {
        dest = std::assume_aligned<alignment>(dest);
        src1 = std::assume_aligned<alignment>(src1);
        src2 = std::assume_aligned<alignment>(src2);

        for (std::size_t i = 0; i < n; ++i)
            dest[i] = src1[i] & ~src2[i];
    }

    template <std::size_t alignment>
    void diff_store(
        std::uint64_t* __restrict dest,
        const std::uint64_t* __restrict src1,
        const std::uint64_t* __restrict src2,
        const std::size_t start,
        const std::size_t end
    ) {
        dest = std::assume_aligned<alignment>(dest);
        src1 = std::assume_aligned<alignment>(src1);
        src2 = std::assume_aligned<alignment>(src2);

        for (std::size_t i = start; i < end; ++i)
            dest[i] = src1[i] & ~src2[i];
    }

    template <std::size_t alignment>
    void memset(std::uint64_t* __restrict dest, const std::uint64_t c, const std::size_t n) {
        dest = std::assume_aligned<alignment>(dest);

        for (std::size_t i = 0; i < n; ++i)
            dest[i] = c;
    }

    template <std::size_t alignment>
    void memset(std::uint64_t* __restrict dest, const std::uint64_t c, const std::size_t start, const std::size_t end) {
        dest = std::assume_aligned<alignment>(dest);

        for (std::size_t i = start; i < end; ++i)
            dest[i] = c;
    }

    template <std::size_t alignment>
    void memcpy(std::uint64_t* __restrict dest, const std::uint64_t* __restrict src, const std::size_t n) {
        dest = std::assume_aligned<alignment>(dest);
        src = std::assume_aligned<alignment>(src);

        for (std::size_t i = 0; i < n; ++i)
            dest[i] = src[i];
    }

    template <std::size_t alignment>
    void flip(std::uint64_t* dest, const std::size_t n) {
        dest = std::assume_aligned<alignment>(dest);

        for (std::size_t i = 0; i < n; ++i)
            dest[i] = ~dest[i];
    }
}
