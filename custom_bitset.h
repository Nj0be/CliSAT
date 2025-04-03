//
// Created by Beniamino Vagnarelli on 01/04/25.
//

#pragma once

#include <cstdint>
#include <vector>
#include <immintrin.h> //__bsrd, __bsrq, etc
#include <tr2/dynamic_bitset>

/*TODO:
 * Destructive scans
 * Refactor first_bit-next_bit and last_bit-prev_bit (counter-intuitive and error-prone)
 */

class custom_bitset {
    const uint64_t _size;
    std::vector<uint64_t> bits;
    std::vector<uint64_t>::iterator forward_block;
    std::vector<uint64_t>::reverse_iterator reverse_block;
    // uint64_t faster than other types
    uint64_t forward_bit;
    uint64_t reverse_bit;

    static uint8_t bit_scan_forward(const uint64_t x) { return __builtin_ctzll(x); }
    static uint8_t bit_scan_reverse(const uint64_t x) { return __bsrq(x); }

    static uint64_t get_block(const uint64_t pos) { return pos/64; };
    static uint64_t get_block_bit(const uint64_t pos) { return pos%64; };

    uint64_t _next_bit(uint64_t pos) const; // used only to print

public:
    explicit custom_bitset(uint64_t size);
    custom_bitset(const custom_bitset& other);

    custom_bitset operator&(const custom_bitset& other);
    custom_bitset operator|(const custom_bitset& other);
    custom_bitset operator~();
    custom_bitset operator&=(const custom_bitset& other);
    custom_bitset operator|=(const custom_bitset& other);
    bool operator[](const uint64_t pos) const { return get_bit(pos); };
    friend std::ostream& operator<<(std::ostream& stream, const custom_bitset& bb);

    // TODO: not SAFE!
    void set_bit(const uint64_t pos) { bits[get_block(pos)] |= (1ULL << get_block_bit(pos)); }
    void unset_bit(const uint64_t pos) { bits[get_block(pos)] &= ~(1ULL << get_block_bit(pos)); }
    bool get_bit(const uint64_t pos) const { return bits[get_block(pos)] >> get_block_bit(pos) & 1; }

    uint64_t first_bit();
    uint64_t next_bit();
    uint64_t last_bit();
    uint64_t prev_bit();

    [[nodiscard]] uint64_t size() const { return _size; }
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
inline custom_bitset::custom_bitset(const uint64_t size): _size((assert(size > 0), size)), bits(((size-1)/64) + 1) {}

inline custom_bitset::custom_bitset(const custom_bitset &other): _size(other._size), bits(other.bits) {}

inline custom_bitset custom_bitset::operator&(const custom_bitset &other) {
    auto bb = custom_bitset(*this);
    bb &= other;

    return bb;
}

inline custom_bitset custom_bitset::operator|(const custom_bitset &other) {
    auto bb = custom_bitset(*this);
    bb |= other;

    return bb;
}

inline custom_bitset custom_bitset::operator~() {
    auto bb = custom_bitset(*this);

    for (uint64_t i = 0; i < bb.bits.size(); ++i) {
        bb.bits[i] = ~bb.bits[i];
    }

    return bb;
}

inline custom_bitset custom_bitset::operator&=(const custom_bitset &other) {
    if (bits.size() != other.bits.size()) return *this;

    for (uint64_t i = 0; i < bits.size(); ++i) {
        bits[i] &= other.bits[i];
    }

    return *this;
}

inline custom_bitset custom_bitset::operator|=(const custom_bitset &other) {
    if (bits.size() != other.bits.size()) return *this;

    for (uint64_t i = 0; i < bits.size(); ++i) {
        bits[i] |= other.bits[i];
    }

    return *this;
}

inline uint64_t custom_bitset::first_bit() {
    forward_block = bits.begin();
    forward_bit = 0;

    do {
        if (*forward_block != 0) {
            forward_bit = bit_scan_forward(*forward_block);
            // returns the index of the current block + the current bit
            return std::distance(bits.begin(), forward_block)*64 + forward_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        ++forward_block;
    } while (forward_block != bits.end());

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

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

inline uint64_t custom_bitset::last_bit() {
    reverse_block = bits.rbegin();
    reverse_bit = (_size-1)%64;

    do {
        if (*reverse_block != 0) {
            reverse_bit = bit_scan_reverse(*reverse_block);
            // returns the index of the current block + the current bit
            return (std::distance(reverse_block, bits.rend())-1)*64 + reverse_bit;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        ++reverse_block;
    } while (reverse_block != bits.rend());

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}

inline uint64_t custom_bitset::prev_bit() {
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

    // probably the latter is better, but in a loop it doesn't make a difference
    return _size;
    //return UINT64_MAX;
}