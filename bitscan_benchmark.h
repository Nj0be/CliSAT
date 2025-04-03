//
// Created by Beniamino Vagnarelli on 31/03/25.
//

#pragma once

#include <iostream>
#include <chrono>
#include <vector>
#include <immintrin.h>
#include <algorithm>
#include <bitset>
#include <bitscan/bitscan.h>
#include "custom_bitset.h"

constexpr uint64_t de_bruijn = 0x03f79d71b4cb0a89;

constexpr uint8_t ms1bTable[256] {
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
};

uint8_t bit_scan_forward(const uint64_t number) {
    if (number == 0) return 0;  // or do something else, it returns 0 anyway

    const uint8_t index64[64] = {
        0, 1, 48, 2, 57, 49, 28, 3,
        61, 58, 50, 42, 38, 29, 17, 4,
        62, 55, 59, 36, 53, 51, 43, 22,
        45, 39, 33, 30, 24, 18, 12, 5,
        63, 47, 56, 27, 60, 41, 37, 16,
        54, 35, 52, 21, 44, 32, 23, 11,
        46, 26, 40, 15, 34, 20, 31, 10,
        25, 14, 19, 9, 13, 8, 7, 6
    };
    // The bitwise and between number and -number results in all 0's except for the least significant bit
    // for the properties of two's complement
    // Multiplying the least significant bit with the de_bruijn number B(2,6) results in a sequence of 6 MSB
    // unique for each power of two (number with only one bit set)
    // Then we shift the 6 MSB to 6 LSB, and we convert the unique sequence to the correspondent position
    // of the LS1B (Least Significant 1 Bit)
    return index64[(number & -number) * de_bruijn >> 58];
}

uint8_t bit_scan_reverse(uint64_t number) {
    if (number == 0) return 0;  // or do something else, it returns 0 anyway

    uint8_t result = 0;
    if (number >= 0xFFFFFFFF) {
        number >>= 32;
        result = 32;
    }
    if (number >= 0xFFFF) {
        number >>= 16;
        result += 16;
    }
    if (number >= 0xFF) {
        number >>= 8;
        result += 8;
    }

    return result + ms1bTable[number];
}

uint64_t randll() {
    return (static_cast<uint64_t>(std::rand()) << 32) | std::rand();
}

void bitscan_benchmark1() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::vector<uint64_t> numbers(50000000);
    std::ranges::generate(numbers, randll);


    std::cout << "Bit Scan Forward:" << std::endl;
    uint64_t a = 0;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += bit_scan_forward(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "diy implementation = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += __builtin_ctzll(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "__builtin_ctzll = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += __bsfq(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "__bsfq = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    // up to 32 bits integers
    /*
    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += _bit_scan_forward(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "_bit_scan_forward = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    */

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        unsigned long long  Ret;
        __asm__
        (
            "bsfq %[number], %[Ret]"
            :[Ret] "=r" (Ret)
            :[number] "mr" (number)
        );
        a += Ret;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "asm bsfq = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += std::countr_zero(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "std::countr_zero = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    std::cout << std::endl;


    std::cout << "Bit Scan Reverse" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += bit_scan_reverse(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "diy implementation = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += 63 - __builtin_clzll(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "63 - _builtin_clzll = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += __bsrq(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "__bsrq = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += __builtin_ia32_bsrdi(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "__builtin_ia32_bsrdi = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    // up to 32 bits integers
    /*
    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += _bit_scan_reverse(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "_bit_scan_reverse = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    */

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
         unsigned long long Ret;
        __asm__
        (
            "bsrq %[number], %[Ret]"
            :[Ret] "=r" (Ret)
            :[number] "mr" (number)
        );
        a += Ret;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "asm bsrq = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += std::bit_floor(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "std::bit_floor = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += std::bit_width(number) - 1;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "std::bit_width = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += 63 - std::countl_zero(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "std::countl_zero = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    std::cout << a << std::endl;
}

void bit_scan_forward_benchmark() {
    std::cout << "Benchmark bit scan forward" << std::endl;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;

    uint64_t len = 50000000;
    uint64_t acc = 0;
    uint64_t bit = 0;

    bitarray bb_empty(len);
    bitarray bb_random(len);
    bitarray bb_complete(len);
    custom_bitset c_bb_empty(len);
    custom_bitset c_bb_random(len);
    custom_bitset c_bb_complete(len);
    for (uint64_t i = 0; i < len; i++) {
        if (rand() % 2) {
            bb_random.set_bit(i);
            c_bb_random.set_bit(i);
        }
        bb_complete.set_bit(i);
        c_bb_complete.set_bit(i);
    }
    bb_empty.init_scan(bbo::NON_DESTRUCTIVE);
    bb_random.init_scan(bbo::NON_DESTRUCTIVE);
    bb_complete.init_scan(bbo::NON_DESTRUCTIVE);

    begin = std::chrono::steady_clock::now();
    while(bb_empty.next_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    bit = c_bb_empty.first_bit();
    while(bit != c_bb_empty.size()) { bit = c_bb_empty.next_bit(); ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_random.next_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    bit = c_bb_random.first_bit();
    while(bit != c_bb_random.size()) { bit = c_bb_random.next_bit(); ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_complete.next_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan complete = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    bit = c_bb_complete.first_bit();
    while(bit != c_bb_complete.size()) { bit = c_bb_complete.next_bit(); ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset complete = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    std::cout << "Total bits scanned: " << acc << std::endl;
}

void bit_scan_reverse_benchmark() {
    std::cout << "Benchmark bit scan reverse" << std::endl;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;

    uint64_t len = 50000000;
    uint64_t acc = 0;
    uint64_t bit = 0;

    bitarray bb_empty(len);
    bitarray bb_random(len);
    bitarray bb_complete(len);
    custom_bitset c_bb_empty(len);
    custom_bitset c_bb_random(len);
    custom_bitset c_bb_complete(len);
    for (uint64_t i = 0; i < len; i++) {
        if (rand() % 2) {
            bb_random.set_bit(i);
            c_bb_random.set_bit(i);
        }
        bb_complete.set_bit(i);
        c_bb_complete.set_bit(i);
    }
    bb_empty.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
    bb_random.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
    bb_complete.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
    // std::cout << bbi;

    begin = std::chrono::steady_clock::now();
    while(bb_empty.prev_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    bit = c_bb_empty.last_bit();
    while(bit != c_bb_empty.size()) { bit = c_bb_empty.prev_bit(); ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_random.prev_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    bit = c_bb_random.last_bit();
    while(bit != c_bb_random.size()) { bit = c_bb_random.prev_bit(); ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_complete.prev_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan complete = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    bit = c_bb_complete.last_bit();
    while(bit != c_bb_complete.size()) { bit = c_bb_complete.prev_bit(); ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset complete = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    std::cout << "Total bits scanned: " << acc << std::endl;
}


void bitwise_and_benchmark() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    uint64_t len = 500000000;

    uint64_t acc = 0;

    bitarray bbi1(len);
    bitarray bbi2(len);
    custom_bitset bb31(len);
    custom_bitset bb32(len);
    for (uint64_t i = 0; i < len; i++) {
        if (rand() % 2) {
            bbi1.set_bit(i);
            bb31.set_bit(i);
        }
    }
    for (uint64_t i = 0; i < len; i++) {
        if (rand() % 2) {
            bbi2.set_bit(i);
            bb32.set_bit(i);
        }
    }
    // std::cout << bbi;

    begin = std::chrono::steady_clock::now();
    bbi1 &= bbi2;
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    bb31 &= bb32;
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset next_bit = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    std::cout << acc << std::endl;
}

void test_custom_biset() {
    bitarray bbi(100);
    bbi.set_bit(10);
    bbi.set_bit(20);
    bbi.set_bit(25);
    bbi.set_bit(70);
    bbi.set_bit(85);
    std::cout << bbi;

    bbi.init_scan(bbo::NON_DESTRUCTIVE);
    int nBit = 0;
    while( (nBit = bbi.next_bit()) != BBObject::noBit ) {
        std::cout << nBit << " ";
    }
    std::cout << std::endl;
}