//
// Created by Beniamino Vagnarelli on 31/03/25.
//

#pragma once

#include <iostream>
#include <chrono>
#include <vector>
#include <immintrin.h>
#include <algorithm>
#include <bitscan/bitscan.h>
#include <graph/graph.h>
#include "custom_bitset.h"

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


void popcount_benchmark() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::vector<uint64_t> numbers(500000000);
    std::ranges::generate(numbers, randll);


    std::cout << "PopCount:" << std::endl;
    uint64_t a = 0;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += std::popcount(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "std::popcount = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += __popcount(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "__popcount = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (auto number : numbers) {
        a += __builtin_popcountll(number);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "__builtin_popcountll = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

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
    custom_bitset s_bb_empty(len);
    custom_bitset s_bb_random(len);
    custom_bitset s_bb_complete(len);

    for (uint64_t i = 0; i < len; i++) {
        if (rand() % 2) {
            bb_random.set_bit(i);
            c_bb_random.set_bit(i);
            s_bb_random.set_bit(i);
        }
        bb_complete.set_bit(i);
        c_bb_complete.set_bit(i);
        s_bb_complete.set_bit(i);
    }
    bb_empty.init_scan(bbo::NON_DESTRUCTIVE);
    bb_random.init_scan(bbo::NON_DESTRUCTIVE);
    bb_complete.init_scan(bbo::NON_DESTRUCTIVE);

    begin = std::chrono::steady_clock::now();
    while(bb_empty.next_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (BitCursor cursor = s_bb_empty.first_bit();
         cursor.getPos() != s_bb_empty.size();
         cursor = s_bb_empty.next_bit(cursor)) {
        ++acc;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_random.next_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (BitCursor cursor = s_bb_random.first_bit();
         cursor.getPos() != s_bb_random.size();
         cursor = s_bb_random.next_bit(cursor)) {
        ++acc;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_complete.next_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan complete = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (BitCursor cursor = s_bb_complete.first_bit();
         cursor.getPos() != s_bb_complete.size();
         cursor = s_bb_complete.next_bit(cursor)) {
        ++acc;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset complete = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    std::cout << "Total bits scanned: " << acc << std::endl;
}

void bit_scan_forward_destructive_benchmark() {
    std::cout << "Benchmark bit scan forward destructive" << std::endl;
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
    custom_bitset s_bb_empty(len);
    custom_bitset s_bb_random(len);
    custom_bitset s_bb_complete(len);

    for (uint64_t i = 0; i < len; i++) {
        if (rand() % 2) {
            bb_random.set_bit(i);
            c_bb_random.set_bit(i);
            s_bb_random.set_bit(i);
        }
        bb_complete.set_bit(i);
        c_bb_complete.set_bit(i);
        s_bb_complete.set_bit(i);
    }
    bb_empty.init_scan(bbo::DESTRUCTIVE);
    bb_random.init_scan(bbo::DESTRUCTIVE);
    bb_complete.init_scan(bbo::DESTRUCTIVE);

    begin = std::chrono::steady_clock::now();
    while(bb_empty.next_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (BitCursor cursor = s_bb_empty.first_bit_destructive();
         cursor.getPos() != s_bb_empty.size();
         cursor = s_bb_empty.next_bit_destructive(cursor)) {
        ++acc;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_random.next_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (BitCursor cursor = s_bb_random.first_bit_destructive();
         cursor.getPos() != s_bb_random.size();
         cursor = s_bb_random.next_bit_destructive(cursor)) {
        ++acc;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_complete.next_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan complete = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (BitCursor cursor = s_bb_complete.first_bit_destructive();
         cursor.getPos() != s_bb_complete.size();
         cursor = s_bb_complete.next_bit_destructive(cursor)) {
        ++acc;
    }
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
    custom_bitset s_bb_empty(len);
    custom_bitset s_bb_random(len);
    custom_bitset s_bb_complete(len);

    for (uint64_t i = 0; i < len; i++) {
        if (rand() % 2) {
            bb_random.set_bit(i);
            c_bb_random.set_bit(i);
            s_bb_random.set_bit(i);
        }
        bb_complete.set_bit(i);
        c_bb_complete.set_bit(i);
        s_bb_complete.set_bit(i);
    }
    bb_empty.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
    bb_random.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
    bb_complete.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);

    begin = std::chrono::steady_clock::now();
    while(bb_empty.prev_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (BitCursor cursor = s_bb_empty.last_bit();
         cursor.getPos() != s_bb_empty.size();
         cursor = s_bb_empty.prev_bit(cursor)) {
        ++acc;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset empty = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_random.prev_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (BitCursor cursor = s_bb_random.last_bit();
         cursor.getPos() != s_bb_random.size();
         cursor = s_bb_random.prev_bit(cursor)) {
        ++acc;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset random = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    while(bb_complete.prev_bit() != BBObject::noBit) { ++acc; }
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan complete = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (BitCursor cursor = s_bb_complete.last_bit();
         cursor.getPos() != s_bb_complete.size();
         cursor = s_bb_complete.prev_bit(cursor)) {
        ++acc;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset complete = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    std::cout << "Total bits scanned: " << acc << std::endl;
}


void bitwise_and_benchmark() {
    std::cout << "Benchmark bitwise and" << std::endl;
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

void subtraction_benchmark() {
    std::cout << "Benchmark subtraction" << std::endl;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    uint64_t len = 500000000;

    bitarray bb1(len);
    bitarray bb2(len);
    custom_bitset c_bb1(len);
    custom_bitset c_bb2(len);
    for (uint64_t i = 0; i < len; i++) {
        if (rand() % 2) {
            bb1.set_bit(i);
            c_bb1.set_bit(i);
        }
    }
    for (uint64_t i = 0; i < len; i++) {
        if (rand() % 2) {
            bb2.set_bit(i);
            c_bb2.set_bit(i);
        }
    }
    // std::cout << bbi;
    /*
    begin = std::chrono::steady_clock::now();
    bb1 &= ~bb2;
    end = std::chrono::steady_clock::now();
    std::cout << "bit scan = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    */

    begin = std::chrono::steady_clock::now();
    custom_bitset c_bb3 = c_bb1 - c_bb2;
    end = std::chrono::steady_clock::now();
    std::cout << "custom bitset = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
}

/*void test_custom_bitset() {
    auto bb = custom_bitset(128);
    bb.set_bit(0);
    bb.set_bit(5);
    bb.set_bit(10);
    bb.set_bit(20);
    bb.set_bit(50);
    bb.set_bit(64);
    bb.set_bit(63);
    bb.set_bit(80);
    bb.set_bit(127);
    std::cout << bb << std::endl;
    std::cout << (~bb) << std::endl;

    auto bb2 = custom_bitset(128);
    bb2.set_bit(0);
    bb2.set_bit(10);
    bb2.set_bit(50);
    bb2.set_bit(63);
    bb2.set_bit(127);
    std::cout << bb - bb2 << std::endl;

    uint64_t bit = bb.first_bit();
    do {
        std::cout << bit << " ";
    } while((bit = bb.next_bit()) != bb.size());
    std::cout << std::endl;

    bit = bb.last_bit();
    do {
        std::cout << bit << " ";
    } while((bit = bb.prev_bit()) != bb.size());
    std::cout << std::endl;

    auto bb3 = custom_bitset(bb);
    auto bb4 = bb & bb2;
}*/