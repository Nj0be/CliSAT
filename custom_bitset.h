//
// Created by Beniamino Vagnarelli on 01/04/25.
//

#pragma once

#include <cstdint>
#include <vector>
#include <immintrin.h> //__bsrd, __bsrq, etc
#include <cassert>
#include <algorithm>

/*TODO:
 * Destructive scans
 * Refactor first_bit-next_bit and last_bit-prev_bit (counter-intuitive and error-prone)
 */

class custom_bitset {
    const uint64_t _size;
    std::vector<uint64_t> bits;
    std::vector<uint64_t>::iterator current_block = bits.begin();
    // uint64_t faster than other types
    uint64_t current_bit = 0;

    static uint8_t bit_scan_forward(const uint64_t x) { return __builtin_ctzll(x); }
    static uint8_t bit_scan_reverse(const uint64_t x) { return __bsrq(x); }

    static uint64_t get_block(const uint64_t pos) { return pos/64; };
    static uint64_t get_block_bit(const uint64_t pos) { return pos%64; };

    [[nodiscard]] uint64_t _next_bit(uint64_t pos) const; // used only to print

public:
    explicit custom_bitset(uint64_t size);
    explicit custom_bitset(uint64_t size, bool default_value);
    explicit custom_bitset(const std::vector<uint64_t>& v);
    custom_bitset(const custom_bitset& other);

    custom_bitset operator&(const custom_bitset& other) const;
    custom_bitset operator|(const custom_bitset& other) const;
    custom_bitset operator~() const;
    custom_bitset operator-(const custom_bitset& other) const;
    custom_bitset& operator=(const custom_bitset& other);
    custom_bitset& operator&=(const custom_bitset& other);
    custom_bitset& operator|=(const custom_bitset& other);
    custom_bitset& operator-=(const custom_bitset& other);
    bool operator[](const uint64_t pos) const { return get_bit(pos); };
    friend std::ostream& operator<<(std::ostream& stream, const custom_bitset& bb);
    explicit operator bool() const;
    explicit operator std::vector<uint64_t>();

    // TODO: not SAFE!
    void set_bit(const uint64_t pos) { bits[get_block(pos)] |= 1ULL << get_block_bit(pos); }
    static void set_block_bit(const std::vector<uint64_t>::iterator &block, const uint64_t &bit) { *block |= 1ULL << bit; }
    void unset_bit(const uint64_t pos) { bits[get_block(pos)] &= ~(1ULL << get_block_bit(pos)); }
    static void unset_block_bit(const std::vector<uint64_t>::iterator &block, const uint64_t &bit) { *block &= ~(1ULL << bit); }
    // we move bit to first position and we mask everything that it isn't on position 1 to 0
    [[nodiscard]] bool get_bit(const uint64_t pos) const { return bits[get_block(pos)] >> get_block_bit(pos) & 1; }

    uint64_t first_bit();
    uint64_t first_bit_destructive();
    uint64_t next_bit();
    uint64_t next_bit_destructive();
    uint64_t last_bit();
    uint64_t last_bit_destructive();
    uint64_t prev_bit();
    uint64_t prev_bit_destructive();

    uint64_t degree();

    void negate();

    [[nodiscard]] uint64_t size() const { return _size; }
    // TODO: removing caching seems beneficial. Verify that
    [[nodiscard]] uint64_t n_set_bits() const {
        uint64_t tot = 0;
        for (auto word : bits) {
            tot += std::popcount(word);
        }

        return tot;
    }

    void swap(custom_bitset& other) noexcept;
};

inline uint64_t custom_bitset::_next_bit(const uint64_t pos) const {
    auto block = bits.begin() + get_block(pos);
    auto block_bit = get_block_bit(pos);

    if (pos == _size) block = bits.begin();
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by last_bit+1
    uint64_t masked_number = *block & ((pos == _size) ? ~0ULL : (~1ULL << block_bit));

    do {
        if (masked_number != 0) {
            block_bit = bit_scan_forward(masked_number);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), block)*64 + block_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        masked_number = *(++block);
    } while (block != bits.end());

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

inline std::ostream& operator<<(std::ostream &stream, const custom_bitset &bb) {
    uint64_t tot = 0;
    std::string tmp = "[";

    auto bit = bb._next_bit(bb.size());

    while (bit != bb.size()) {
        tmp += std::format("{} ", bit);
        tot++;
        bit = bb._next_bit(bit);
    }
    tmp += std::format("({})]", tot);

    return (stream << tmp);
}

// TODO: is assert good enough?
inline custom_bitset::custom_bitset(const uint64_t size): custom_bitset(size, 0) {}

// we set everything to 1 or to 0
inline custom_bitset::custom_bitset(uint64_t size, const bool default_value): _size((assert(size > 0), size)), bits(((_size-1)/64) + 1, default_value*~0ULL) {
    // unset last part of last block
    bits.back() &= ~(~0ULL << (_size%64));
}

// TODO: change assert
inline custom_bitset::custom_bitset(const std::vector<uint64_t> &v): _size((assert(v.size()), *std::ranges::max_element(v) + 1)), bits((_size-1)/64 + 1) {
    for (const auto pos: v) {
        set_bit(pos);
    }
}

inline custom_bitset::custom_bitset(const custom_bitset &other): _size(other._size), bits(other.bits) {}

inline custom_bitset custom_bitset::operator&(const custom_bitset &other) const {
    auto bb(*this);
    bb &= other;

    return bb;
}

inline custom_bitset custom_bitset::operator|(const custom_bitset &other) const {
    auto bb(*this);
    bb |= other;

    return bb;
}

inline custom_bitset custom_bitset::operator~() const {
    auto bb(*this);
    bb.negate();

    return bb;
}

inline custom_bitset custom_bitset::operator-(const custom_bitset &other) const {
    auto bb(*this);
    bb -= other;

    return bb;
}

inline custom_bitset& custom_bitset::operator=(const custom_bitset &other) {
    bits = other.bits;
    //current_block = other.current_block;
    //current_bit = other.current_bit;

    return *this;
}

inline custom_bitset& custom_bitset::operator&=(const custom_bitset &other) {
    for (uint64_t i = 0; i < bits.size(); ++i) {
        // to avoid out of bound memory access
        if (i >= other.bits.size()) bits[i] = 0;
        else {
            bits[i] &= other.bits[i];
        }
    }

    return *this;
}

// TODO: not safe
inline custom_bitset& custom_bitset::operator|=(const custom_bitset &other) {
    //if (bits.size() != other.bits.size()) return *this;

    for (uint64_t i = 0; i < bits.size(); ++i) {
        bits[i] |= other.bits[i];
    }

    return *this;
}

inline custom_bitset& custom_bitset::operator-=(const custom_bitset &other) {
    const auto min_length = std::min(size(), other.size());
    const auto min_block = (min_length-1)/64 + 1;

    // equivalent to *this &= ~other; but faster
    for (uint64_t i = 0; i < min_block; ++i) {
        bits[i] &= ~other.bits[i];
    }

    return *this;
}

inline custom_bitset::operator bool() const {
    for (const auto word:bits) {
        if (word) return true;
    }

    return false;
}

inline custom_bitset::operator std::vector<unsigned long>() {
    const auto orig_block = current_block;
    const auto orig_bit = current_bit;

    std::vector<uint64_t> list;

    auto bit = first_bit();
    while (bit != size()) {
        list.push_back(bit);
        bit = next_bit();
    }

    current_block = orig_block;
    current_bit = orig_bit;

    return list;
}

inline uint64_t custom_bitset::first_bit() {
    current_block = bits.begin();
    current_bit = 0;

    do {
        if (*current_block != 0) {
            current_bit = bit_scan_forward(*current_block);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), current_block)*64 + current_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        ++current_block;
    } while (current_block != bits.end());

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

inline uint64_t custom_bitset::first_bit_destructive() {
    current_block = bits.begin();
    current_bit = 0;

    do {
        if (*current_block != 0) {
            current_bit = bit_scan_forward(*current_block);
            unset_block_bit(current_block, current_bit);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), current_block)*64 + current_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        ++current_block;
    } while (current_block != bits.end());

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

// bug: not returning the first bit if set ...
// we use first_bit to get the first bit, and we start from there...
inline uint64_t custom_bitset::next_bit() {
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by last_bit+1
    uint64_t masked_number = *current_block & (~1ULL << current_bit);

    do {
        if (masked_number != 0) {
            current_bit = bit_scan_forward(masked_number);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), current_block)*64 + current_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        masked_number = *(++current_block);
    } while (current_block != bits.end());

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

inline uint64_t custom_bitset::next_bit_destructive() {
// shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by last_bit+1
    uint64_t masked_number = *current_block & (~1ULL << current_bit);

    do {
        if (masked_number != 0) {
            current_bit = bit_scan_forward(masked_number);
            unset_block_bit(current_block, current_bit);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), current_block)*64 + current_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        masked_number = *(++current_block);
    } while (current_block != bits.end());

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

inline uint64_t custom_bitset::last_bit() {
    current_block = bits.end()-1;
    current_bit = (_size-1)%64;

    do {
        if (*current_block != 0) {
            current_bit = bit_scan_reverse(*current_block);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), current_block)*64 + current_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        --current_block;
    } while (current_block != bits.begin()-1);

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

inline uint64_t custom_bitset::last_bit_destructive() {
    current_block = bits.end()-1;
    current_bit = (_size-1)%64;

    do {
        if (*current_block != 0) {
            current_bit = bit_scan_reverse(*current_block);
            unset_block_bit(current_block, current_bit);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), current_block)*64 + current_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        --current_block;
    } while (current_block != bits.begin()-1);

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

inline uint64_t custom_bitset::prev_bit() {
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by last_bit+1
    uint64_t masked_number = *current_block & ~(~0ULL << current_bit);

    //std::cout << masked_number << " " << std::distance(current_block, bits.rend()) << " " << current_bit << std::endl;
    //std::cout << std::bitset<SIZEOF_WORD>(masked_number).to_string() << " " << std::distance(current_block, bits.rend()) << " " << current_bit << std::endl;

    do {
        if (masked_number != 0) {
            current_bit = bit_scan_reverse(masked_number);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), current_block)*64 + current_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        masked_number = *(--current_block);
    } while (current_block != bits.begin()-1);

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

// TODO: optimize (use variable and update?)
inline uint64_t custom_bitset::degree() {
    const auto orig_block = current_block;
    const auto orig_bit = current_bit;

    uint64_t degree = 0;

    auto bit = first_bit();
    while (bit != size()) {
        degree++;
        bit = next_bit();
    }

    current_block = orig_block;
    current_bit = orig_bit;

    return degree;
}

inline void custom_bitset::negate() {
    for (unsigned long & bit : bits) {
        bit = ~bit;
    }
}

inline void custom_bitset::swap(custom_bitset &other) noexcept {
    const auto tmp = *this;
    *this = other;
    other = tmp;
}
