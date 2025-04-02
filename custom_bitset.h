//
// Created by benia on 01/04/25.
//

#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <bitset>
#include <immintrin.h> //__bsrd, __bsrq, etc

// no pointers, no need to implement copy constructor
class custom_bitset {
    const uint64_t _size;
    std::vector<uint64_t> bits;
    std::vector<uint64_t>::iterator forward_block = bits.begin();
    std::vector<uint64_t>::reverse_iterator reverse_block = bits.rbegin();
    // uint64_t faster than other types
    uint64_t forward_bit = 0;
    uint64_t reverse_bit = 0;

    static uint8_t bit_scan_forward(const uint64_t x) { return __builtin_ctzll(x); }

    static uint8_t bit_scan_reverse(const uint64_t x) { return __bsrq(x); }

public:
    explicit custom_bitset(uint64_t size);

    custom_bitset& operator&=(const custom_bitset& other);

    custom_bitset& operator|=(const custom_bitset& other);

    void setBit(uint64_t pos);

    void unsetBit(uint64_t pos);

    void print() const;

    uint64_t next_bit();

    uint64_t prev_bit();

    void start_bit_scan_forward();

    void start_bit_scan_reverse();

    [[nodiscard]] uint64_t size() const { return _size; }
};


inline custom_bitset::custom_bitset(const uint64_t size): _size(size), bits(((size-1)/64) + 1) {
    start_bit_scan_forward();
    start_bit_scan_reverse();
}

inline custom_bitset & custom_bitset::operator&=(const custom_bitset &other) {
    if (bits.size() != other.bits.size()) return *this;

    for (uint64_t i = 0; i < bits.size(); ++i) {
        bits[i] &= other.bits[i];
    }

    return *this;
}

inline custom_bitset & custom_bitset::operator|=(const custom_bitset &other) {
    if (bits.size() != other.bits.size()) return *this;

    for (uint64_t i = 0; i < bits.size(); ++i) {
        bits[i] |= other.bits[i];
    }

    return *this;
}

inline void custom_bitset::setBit(const uint64_t pos) {
    const uint64_t block = pos/64;
    const uint64_t rel_pos = pos%64;

    bits[block] |= (1ULL << rel_pos);
}

inline void custom_bitset::unsetBit(const uint64_t pos) {
    const uint64_t block = pos/64;
    const uint64_t rel_pos = pos%64;

    bits[block] &= ~(1ULL << rel_pos);
}

inline void custom_bitset::print() const {
    for (uint64_t i = 0; i < bits.size(); i++) {
        if (i == bits.size()-1)
            std::cout << std::bitset<64>(bits[i]).to_string().substr(0, _size%64);
        else
            std::cout << std::bitset<64>(bits[i]).to_string();
    }
    std::cout << std::endl;
}

uint64_t inline custom_bitset::next_bit() {
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by last_bit+1
    uint64_t masked_number = *forward_block & (~1ULL << forward_bit);

    do {
        if (masked_number != 0) {
            forward_bit = bit_scan_forward(masked_number);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), forward_block)*64 + forward_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        masked_number = *(++forward_block);
    } while (forward_block != bits.end());

    start_bit_scan_forward();

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

uint64_t inline custom_bitset::prev_bit() {
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by last_bit+1
    uint64_t masked_number = *reverse_block & ~(~0ULL << reverse_bit);

    //std::cout << masked_number << " " << std::distance(reverse_block, bits.rend()) << " " << reverse_bit << std::endl;
    //std::cout << std::bitset<SIZEOF_WORD>(masked_number).to_string() << " " << std::distance(reverse_block, bits.rend()) << " " << reverse_bit << std::endl;

    do {
        if (masked_number != 0) {
            reverse_bit = bit_scan_reverse(masked_number);
            // returns the index of the current block + the current bit
            return (std::distance(reverse_block, bits.rend())-1)*64 + reverse_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        masked_number = *(++reverse_block);
    } while (reverse_block != bits.rend());

    start_bit_scan_reverse();
    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

inline void custom_bitset::start_bit_scan_forward() {
    forward_block = bits.begin();
    forward_bit = 0;
}

inline void custom_bitset::start_bit_scan_reverse() {
    reverse_block = bits.rbegin();
    reverse_bit = _size%64 - 1;
}
