//
// Created by benia on 01/04/25.
//

#ifndef BITSET_H
#define BITSET_H
#include <cstdint>
#include <vector>
#include <iostream>
#include <bitset>

// I don't know why, but using uint32_t instead of uint64_t improves performance dramatically
// Maybe it's system dependant
typedef uint32_t WORD;
#define SIZEOF_WORD (sizeof(WORD)*8)

class custom_bitset {
    const uint64_t _size;
    std::vector<WORD> bits;
    std::vector<WORD>::iterator last_block = bits.begin();
    // uint64_t faster than other types
    uint64_t last_bit = 0;

    static uint8_t bit_scan_forward(const uint64_t x) {
        // faster but not always available(?)
        // probably the if is pointless
        #if WORD == uint32_t
        return __builtin_ctz(x);
        #elif WORD == uint64_t
        return __builtin_ctzll(x);
        #endif
        //return std::countr_zero(x);
    }
    bool bit_scan_forward2(const uint64_t x) {
        // faster but not always available(?)
        if (!x) return false;
        #if WORD == uint32_t
        last_bit = __builtin_ctz(x);
        #elif WORD == uint64_t
        last_bit = __builtin_ctzll(x);
        #endif
        return true;
        //return std::countr_zero(x);
    }
    static uint8_t bit_scan_reverse(const uint64_t x) { return (SIZEOF_WORD-1) - std::countl_zero(x); }

public:
    explicit custom_bitset(const uint64_t size) : _size(size), bits(((size-1)/SIZEOF_WORD) + 1) {}

    custom_bitset& operator&=(const custom_bitset& other) {
        if (bits.size() != other.bits.size()) return *this;

        for (auto i = 0; i < bits.size(); ++i) {
            bits[i] &= other.bits[i];
        }

        return *this;
    }

    void setBit(const uint64_t pos) {
        const uint64_t block = pos/SIZEOF_WORD;
        const uint64_t rel_pos = pos%SIZEOF_WORD;
        bits[block] |= (1 << rel_pos);
    }

    void unsetBit(const uint64_t pos) {
        const uint64_t block = pos/SIZEOF_WORD;
        const uint64_t rel_pos = pos%SIZEOF_WORD;
        bits[block] |= (0 << rel_pos); //TODO wrong
    }

    void print() const {
        for (auto i = 0; i < bits.size(); i++) {
            if (i == bits.size()-1)
                std::cout << std::bitset<SIZEOF_WORD>(bits[i]).to_string().substr(0, _size%SIZEOF_WORD);
            else
                std::cout << std::bitset<SIZEOF_WORD>(bits[i]).to_string();
        }
        std::cout << std::endl;
    }

    /*
    uint64_t next_bit() {
        // shift by 64 doesn't work!! undefined behaviour
        // ~1ULL is all 1's except for the lowest one, aka already shifted by one
        // the resulting shift is all 1's shifted by last_bit+1
        const uint64_t masked_number = *last_block & (~1ULL << last_bit);
        if (masked_number != 0) {
            last_bit = bit_scan_forward(masked_number);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), last_block)*64 + last_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }

        for (++last_block; last_block != bits.end(); ++last_block) {
            if (*last_block != 0) {
                last_bit = bit_scan_forward(*last_block);
                return std::distance(bits.begin(), last_block)*64 + last_bit;
            }
        }

        last_block = bits.begin();
        last_bit = 0;

        // probably the latter is better, but in a loop it doesn't make a difference
        return _size;
        //return UINT64_MAX;
    }
    */

    uint64_t next_bit() {
        // shift by 64 doesn't work!! undefined behaviour
        // ~1ULL is all 1's except for the lowest one, aka already shifted by one
        // the resulting shift is all 1's shifted by last_bit+1
        #if WORD == uint32_t
        WORD masked_number = *last_block & (~1U << last_bit);
        #elif WORD == uint64_t
        WORD masked_number = *last_block & (~1ULL << last_bit);
        #endif

        do {
            if (masked_number != 0) {
                last_bit = bit_scan_forward(masked_number);
                // returns the index of the current block + the current bit
                return std::distance(bits.begin(), last_block)*SIZEOF_WORD + last_bit;
                // it's equivalent but not any faster
                //return std::distance(bits.begin(), last_block) << 6 | last_bit;
            }
            masked_number = *(++last_block);
        } while (last_block != bits.end());

        last_block = bits.begin();
        last_bit = 0;

        // probably the latter is better, but in a loop it doesn't make a difference
        return _size;
        //return UINT64_MAX;
    }

    uint64_t next_bit2() {
        // shift by 64 doesn't work!! undefined behaviour
        // ~1ULL is all 1's except for the lowest one, aka already shifted by one
        // the resulting shift is all 1's shifted by last_bit+1
        #if WORD == uint32_t
        WORD masked_number = *last_block & (~1U << last_bit);
        #elif WORD == uint64_t
        WORD masked_number = *last_block & (~1ULL << last_bit);
        #endif

        do {
            if (bit_scan_forward2(masked_number)) {
                return std::distance(bits.begin(), last_block)*SIZEOF_WORD + last_bit;
            }
            masked_number = *(++last_block);
        } while (last_block != bits.end());

        last_block = bits.begin();
        last_bit = 0;

        // probably the latter is better, but in a loop it doesn't make a difference
        return _size;
        //return UINT64_MAX;
    }


    [[nodiscard]] uint64_t size() const { return _size; }
};

#endif //BITSET_H


//
// Created by benia on 01/04/25.
//

#ifndef BITSET_H
#define BITSET_H
#include <cstdint>
#include <vector>
#include <iostream>
#include <bitset>

class custom_bitset {
    const uint64_t _size;
    std::vector<uint32_t> bits;
    std::vector<uint32_t>::iterator last_block = bits.begin();
    // uint64_t faster than other types
    uint64_t last_bit = 0;

    static uint8_t bit_scan_forward(const uint64_t x) {
        // faster but not always available(?)
        return __builtin_ctzll(x);
        //return std::countr_zero(x);
    }
    bool bit_scan_forward2(const uint64_t x) {
        // faster but not always available(?)
        if (!x) return false;
        last_bit = __builtin_ctzll(x);
        return true;
        //return std::countr_zero(x);
    }
    static uint8_t bit_scan_reverse(const uint64_t x) { return 63 - std::countl_zero(x); }

public:
    explicit custom_bitset(const uint64_t size) : _size(size), bits(((size-1)/64) + 1) {}

    void setBit(const uint64_t pos) {
        const uint64_t block = pos/32;
        const uint64_t rel_pos = pos%64;
        bits[block] |= (1 << rel_pos);
    }

    void unsetBit(const uint64_t pos) {
        const uint64_t block = pos/64;
        const uint64_t rel_pos = pos%64;
        bits[block] |= (0 << rel_pos); //TODO wrong
    }

    void print() const {
        for (auto i = 0; i < bits.size(); i++) {
            if (i == bits.size()-1)
                std::cout << std::bitset<64>(bits[i]).to_string().substr(0, _size%64);
            else
                std::cout << std::bitset<64>(bits[i]).to_string();
        }
        std::cout << std::endl;
    }

    /*
    uint64_t next_bit() {
        // shift by 64 doesn't work!! undefined behaviour
        // ~1ULL is all 1's except for the lowest one, aka already shifted by one
        // the resulting shift is all 1's shifted by last_bit+1
        const uint64_t masked_number = *last_block & (~1ULL << last_bit);
        if (masked_number != 0) {
            last_bit = bit_scan_forward(masked_number);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), last_block)*64 + last_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }

        for (++last_block; last_block != bits.end(); ++last_block) {
            if (*last_block != 0) {
                last_bit = bit_scan_forward(*last_block);
                return std::distance(bits.begin(), last_block)*64 + last_bit;
            }
        }

        last_block = bits.begin();
        last_bit = 0;

        // probably the latter is better, but in a loop it doesn't make a difference
        return _size;
        //return UINT64_MAX;
    }
    */

    uint64_t next_bit() {
        // shift by 64 doesn't work!! undefined behaviour
        // ~1ULL is all 1's except for the lowest one, aka already shifted by one
        // the resulting shift is all 1's shifted by last_bit+1
        uint64_t masked_number = *last_block & (~1ULL << last_bit);

        do {
            if (masked_number != 0) {
                last_bit = bit_scan_forward(masked_number);
                // returns the index of the current block + the current bit
                return std::distance(bits.begin(), last_block)*64 + last_bit;
                // it's equivalent but not any faster
                //return std::distance(bits.begin(), last_block) << 6 | last_bit;
            }
            masked_number = *(++last_block);
        } while (last_block != bits.end());

        last_block = bits.begin();
        last_bit = 0;

        // probably the latter is better, but in a loop it doesn't make a difference
        return _size;
        //return UINT64_MAX;
    }

    uint64_t next_bit2() {
        // shift by 64 doesn't work!! undefined behaviour
        // ~1ULL is all 1's except for the lowest one, aka already shifted by one
        // the resulting shift is all 1's shifted by last_bit+1
        uint64_t masked_number = *last_block & (~1ULL << last_bit);

        do {
            if (bit_scan_forward2(masked_number)) {
                return std::distance(bits.begin(), last_block)*64 + last_bit;
            }
            masked_number = *(++last_block);
        } while (last_block != bits.end());

        last_block = bits.begin();
        last_bit = 0;

        // probably the latter is better, but in a loop it doesn't make a difference
        return _size;
        //return UINT64_MAX;
    }


    [[nodiscard]] uint64_t size() const { return _size; }
};

#endif //BITSET_H