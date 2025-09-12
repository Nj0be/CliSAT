//
// Created by benia on 02/09/2025.
//

#pragma once

// TODO: support MSVC. Use intrinsics instead of inline asm for MSVC

namespace instructions {
    struct features {
        bool popcnt = false;
        bool sse2 = false;
        bool sse4_1 = false;
        bool avx = false;
        bool avx2 = false;
        bool avx512f = false;
    };

    inline void cpuid(std::uint32_t basic_leaf, std::uint32_t extended_leaf,
                      std::uint32_t& eax, std::uint32_t& ebx, std::uint32_t& ecx, std::uint32_t& edx) noexcept {
        asm volatile("cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(basic_leaf), "c"(extended_leaf));
    }

    inline features cpu_supports_impl() {
        features cpu_supports;
        unsigned eax=0, ebx=0, ecx=0, edx=0;

        cpuid(0, 0, eax, ebx, ecx, edx);
        const std::uint32_t max_basic_leaf = eax;

        cpuid(0x80000000, 0, eax, ebx, ecx, edx);
        const std::uint32_t max_extended_leaf = eax;

        if (max_basic_leaf == 0) return cpu_supports;
        cpuid(1, 0, eax, ebx, ecx, edx);
        cpu_supports.popcnt = (ecx >> 23) & 1;
        cpu_supports.sse2 = (ecx >> 19) & 1;
        cpu_supports.sse4_1 = (ecx >> 20) & 1;
        cpu_supports.avx = (ecx >> 28) & 1;

        if (max_basic_leaf < 7) return cpu_supports;

        cpuid(7, 0, eax, ebx, ecx, edx);
        cpu_supports.avx2 = (ebx >> 5) & 1;
        cpu_supports.avx512f = (ebx >> 16) & 1;

        return cpu_supports;
    }

    features cpu_supports = cpu_supports_impl();

    unsigned constexpr popcnt32_sw(const std::uint32_t x) noexcept { return std::popcount(x); }
    unsigned constexpr popcnt64_sw(const std::uint64_t x) noexcept { return std::popcount(x); }
    unsigned constexpr popcnt128_sw(const __uint128_t x) noexcept { return std::popcount(x); }

    unsigned constexpr popcnt32_hw(const std::uint32_t x) noexcept {
        std::uint32_t result;
        asm ("popcnt %1, %0" : "=r"(result) : "r"(x));
        return result;
    }
    unsigned constexpr popcnt64_hw(const std::uint64_t x) noexcept {
        std::uint64_t result;
        asm ("popcnt %1, %0" : "=r"(result) : "r"(x));
        return result;
    }
    unsigned constexpr popcnt128_hw(const __uint128_t x) noexcept {
        auto hi = static_cast<std::uint64_t>(x >> 64);
        auto lo = static_cast<std::uint64_t>(x);
        return popcnt64_hw(lo) + popcnt64_hw(hi);
    }

    inline std::uint32_t bsf32(const std::uint32_t x) {
        std::uint32_t result;
        asm ("bsf %1, %0"
            : "=r"(result)
            : "r"(x)
            : "cc"
        );
        return result;
    }

    inline std::uint64_t bsf64(const std::uint64_t x) {
        std::size_t result;
        asm ("bsfq %1, %0"
            : "=r"(result)
            : "r"(x)
            : "cc"
        );
        return result;
    }

    inline std::uint64_t bsf128(const __uint128_t x) {
        auto hi = static_cast<std::uint64_t>(x >> 64);
        auto lo = static_cast<std::uint64_t>(x);
        if (lo) return bsf64(lo);
        return 64 + bsf64(hi);
    }

    inline std::uint32_t bsr32(const std::uint32_t x) {
        std::uint32_t result;
        asm ("bsr %1, %0"
            : "=r"(result)
            : "r"(x)
            : "cc"
        );
        return result;
    }

    inline std::uint64_t bsr64(const std::uint64_t x) {
        std::size_t result;
        asm ("bsrq %1, %0"
            : "=r"(result)
            : "r"(x)
            : "cc"
        );
        return result;
    }

    inline std::uint64_t bsr128(const __uint128_t x) {
        auto hi = static_cast<std::uint64_t>(x >> 64);
        auto lo = static_cast<std::uint64_t>(x);
        if (hi) return 64 + bsr64(hi);
        return bsr64(lo);
    }

    template <std::unsigned_integral T>
    unsigned popcount(T x) noexcept {
        if (cpu_supports.popcnt) {
            if constexpr (sizeof(T) <= 4) return popcnt32_hw(static_cast<std::uint32_t>(x));
            else if constexpr (sizeof(T) == 8) return popcnt64_hw(static_cast<std::uint64_t>(x));
            else return popcnt128_hw(static_cast<__uint128_t>(x));
        } else {
            if constexpr (sizeof(T) <= 4) return popcnt32_sw(static_cast<std::uint32_t>(x));
            else if constexpr (sizeof(T) == 8) return popcnt64_sw(static_cast<std::uint64_t>(x));
            else return popcnt128_sw(static_cast<__uint128_t>(x));
        }
    }

    template <std::unsigned_integral T>
    std::size_t bit_scan_forward(T x) {
        if constexpr (sizeof(T) <= 4) return bsf32(static_cast<std::uint32_t>(x));
        else if constexpr (sizeof(T) == 8) return bsf64(static_cast<std::uint64_t>(x));
        else if constexpr (sizeof(T) == 16) return bsf128(static_cast<__uint128_t>(x));
    }

    template <std::unsigned_integral T>
    std::size_t bit_scan_reverse(T x) {
        if constexpr (sizeof(T) <= 4) return bsr32(static_cast<std::uint32_t>(x));
        else if constexpr (sizeof(T) == 8) return bsr64(static_cast<std::uint64_t>(x));
        else if constexpr (sizeof(T) == 16) return bsr128(static_cast<__uint128_t>(x));
    }
}