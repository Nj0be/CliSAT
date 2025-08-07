//
// Created by Beniamino Vagnarelli on 01/04/25.
//

#pragma once

#include <cstdint>
#include <vector>
#include <immintrin.h> //__bsrd, __bsrq, etc
#include <cassert>
#include <algorithm>
#include <iostream>

/*TODO:
 * Destructive scans
 * Refactor first_bit-next_bit and last_bit-prev_bit (counter-intuitive and error-prone)
 */

struct BitCursor {
private:
    uint64_t block_index;     // current block index
    uint64_t bit;       // bit position inside block

public:
    BitCursor(const uint64_t block_index, const uint64_t bit): block_index(block_index), bit(bit) {}
    // Absolute position of bit
    [[nodiscard]] uint64_t get_pos() const {
        return block_index * 64 + bit;
    }

    friend class custom_bitset;
};

class custom_bitset {
    uint64_t _size;
    std::vector<uint64_t> bits;

    static uint8_t bit_scan_forward(const uint64_t x) { return __builtin_ctzll(x); }
    static uint8_t bit_scan_reverse(const uint64_t x) { return __bsrq(x); }

    static uint64_t get_block(const uint64_t pos) { return pos/64; };
    static uint64_t get_block_bit(const uint64_t pos) { return pos%64; };

    // Used to allocate at least one block regardless of size
    static uint64_t blocks_needed(const uint64_t size) {
        return (size + 63) / 64 | (size == 0);
    }

    // it's necessary because even if size it's 0, we still have a block inside bits vector
    static uint64_t get_last_block(const uint64_t size) {
        return ((size + 63) / 64) - (size != 0);
    }

public:
    explicit custom_bitset(uint64_t size);
    custom_bitset(uint64_t size, bool default_value);
    custom_bitset(uint64_t size, uint64_t set_first_n_bits);
    custom_bitset(const custom_bitset& other, uint64_t size);
    explicit custom_bitset(const std::vector<uint64_t>& v);
    custom_bitset(const std::vector<uint64_t> &v, uint64_t size);
    custom_bitset(const custom_bitset& other) = default;

    custom_bitset operator&(const custom_bitset& other) const;
    custom_bitset operator|(const custom_bitset& other) const;
    custom_bitset operator~() const;
    custom_bitset operator-(const custom_bitset& other) const;
    custom_bitset& operator=(const custom_bitset& other);
    bool operator==(const custom_bitset& other) const;
    custom_bitset& operator&=(const custom_bitset& other);
    custom_bitset& operator|=(const custom_bitset& other);
    custom_bitset& operator-=(const custom_bitset& other);
    bool operator[](const uint64_t pos) const { return get_bit(pos); };
    friend std::ostream& operator<<(std::ostream& stream, const custom_bitset& bb);
    explicit operator bool() const;
    explicit operator std::vector<uint64_t>() const;

    // TODO: not SAFE!
    void set_bit(const uint64_t pos) { bits[get_block(pos)] |= 1ULL << get_block_bit(pos); }
    static void set_block_bit(const std::vector<uint64_t>::iterator &block, const uint64_t &bit) { *block |= 1ULL << bit; }
    void unset_bit(const uint64_t pos) { bits[get_block(pos)] &= ~(1ULL << get_block_bit(pos)); }
    static void unset_block_bit(const std::vector<uint64_t>::iterator &block, const uint64_t &bit) { *block &= ~(1ULL << bit); }
    void unset_block_bit(const BitCursor& cursor) { bits[cursor.block_index] &= ~(1ULL << cursor.bit); }
    // we move bit to first position and we mask everything that it isn't on position 1 to 0
    [[nodiscard]] bool get_bit(const uint64_t pos) const { return bits[get_block(pos)] >> get_block_bit(pos) & 1; }

    void unset_all();

    [[nodiscard]] BitCursor first_bit() const;
    BitCursor first_bit_destructive();
    [[nodiscard]] BitCursor next_bit(const BitCursor& current_cursor) const;
    BitCursor next_bit_destructive(const BitCursor& current_cursor);
    [[nodiscard]] BitCursor last_bit() const;
    BitCursor last_bit_destructive();
    [[nodiscard]] BitCursor prev_bit(const BitCursor& current_cursor) const;
    BitCursor prev_bit_destructive(const BitCursor& current_cursor);

    [[nodiscard]] uint64_t degree() const;
    [[nodiscard]] uint64_t neighbors_degree() const;

    void negate();

    [[nodiscard]] uint64_t size() const { return _size; }
    // TODO: removing caching seems beneficial. Verify that
    [[nodiscard]] uint64_t n_set_bits() const {
        uint64_t tot = 0;
        for (const auto word : bits) {
            tot += std::popcount(word);
        }

        return tot;
    }

    void swap(custom_bitset& other) noexcept;

    custom_bitset& resize(const uint64_t new_size) {
        if (_size == new_size) return *this;
        _size = new_size;
        bits.resize(blocks_needed(_size));
        bits[get_last_block(_size)] &= ~(~0ULL << (_size%64));
        return *this;
    }
};

inline std::ostream& operator<<(std::ostream &stream, const custom_bitset &bb) {
    uint64_t tot = 0;
    std::string tmp = "[";

    auto cursor = bb.first_bit();

    while (cursor.get_pos() != bb.size()) {
        tmp += std::format("{} ", cursor.get_pos());
        tot++;
        cursor = bb.next_bit(cursor);
    }
    tmp += std::format("({})]", tot);

    return (stream << tmp);
}

inline custom_bitset::custom_bitset(const uint64_t size): custom_bitset(size, false) {}

// we set everything to 1 or to 0
inline custom_bitset::custom_bitset(const uint64_t size, const bool default_value): _size(size), bits(blocks_needed(size), default_value*~0ULL) {
    // unset last part of last block (if there is one)
    if (_size%64) bits.back() &= ~(~0ULL << (_size%64));
}

inline custom_bitset::custom_bitset(const uint64_t size, const uint64_t set_first_n_bits): _size(size), bits(blocks_needed(size)) {
    const auto n_bits = std::min(set_first_n_bits, size);

    const auto n_block = (n_bits-1)/64;
    const auto n_bit = n_bits%64;

    for (uint64_t i = 0; i <= n_block; i++) {
        bits[i] = UINT64_MAX;
    }

    bits[n_block] &= ~(~0ULL << n_bit);
}

inline custom_bitset::custom_bitset(const custom_bitset& other, const uint64_t size): custom_bitset(other) {
    this->resize(size);
}

inline custom_bitset::custom_bitset(const std::vector<uint64_t> &v): _size(*std::ranges::max_element(v) + 1), bits(blocks_needed(_size)) {
    for (const auto pos: v) {
        set_bit(pos);
    }
}

inline custom_bitset::custom_bitset(const std::vector<uint64_t> &v, const uint64_t size): _size(size), bits(blocks_needed(size)) {
    for (const auto pos: v) {
        set_bit(pos);
    }
}

inline custom_bitset custom_bitset::operator&(const custom_bitset& other) const {
    /*
    custom_bitset bb(std::max(size(), other.size()));
    for (uint64_t i = 0; i < bits.size(); ++i) {
        if (i < other.bits.size()) bb.bits[i] = bits[i] & other.bits[i];
    }
    return bb;
    */

    custom_bitset bb(std::max(size(), other.size()));
    bb.bits = bits;
    bb &= other;
    return bb;

    /*
    if (size() >= other.size()) {
        custom_bitset bb(*this);
        bb &= other;

        return bb;
    } else {
        custom_bitset bb(other);
        bb &= *this;

        return bb;
    }
*/
}

inline custom_bitset custom_bitset::operator|(const custom_bitset& other) const {
    custom_bitset bb(std::max(size(), other.size()));
    for (uint64_t i = 0; i < bits.size(); ++i) {
        if (i < other.bits.size()) bb.bits[i] = bits[i] | other.bits[i];
    }
    return bb;

    /*
    custom_bitset bb(std::max(size(), other.size()));
    bb.bits = bits;
    bb |= other;
    return bb;
    */

    /*
    if (size() >= other.size()) {
        custom_bitset bb(*this);
        bb |= other;

        return bb;
    } else {
        custom_bitset bb(other);
        bb |= *this;

        return bb;
    }
*/
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
    _size = other.size();
    bits = other.bits;
    bits.resize(blocks_needed(_size));
    bits[get_last_block(_size)] &= ~(~0ULL << (_size%64));

    return *this;
}

inline bool custom_bitset::operator==(const custom_bitset &other) const {
    if (this == &other) return true;
    if (size() != other.size()) return false;

    for (uint64_t i = 0; i < bits.size(); i++) {
        if (bits[i] != other.bits[i]) return false;
    }

    return true;
}

inline custom_bitset& custom_bitset::operator&=(const custom_bitset &other) {
    for (uint64_t i = 0; i < bits.size(); ++i) {
        if (i < other.bits.size()) bits[i] &= other.bits[i];
        else bits[i] = 0;
        // equivalent to:
        //bits[i] &= ((i < other.bits.size())*~0ULL & other.bits[i]);
        //bits[i] &= (i < other.bits.size() ? other.bits[i] : 0);
    }

    return *this;
}

inline custom_bitset& custom_bitset::operator|=(const custom_bitset &other) {
    for (uint64_t i = 0; i < bits.size(); ++i) {
        if (i < other.bits.size()) bits[i] |= other.bits[i];
        //equivalent to:
        //bits[i] |= ((i < other.bits.size())*other.bits[i]);
        //bits[i] |= (i < other.bits.size() ? other.bits[i] : 0);
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
    //std::ranges::any_of(bits, [&](auto const& word){ return word; });
    return std::ranges::any_of(bits, [](const uint64_t word) { return word != 0; });
}

inline custom_bitset::operator std::vector<unsigned long>() const {
    auto block = bits.begin();
    auto block_bit = 0;

    std::vector<uint64_t> list;

    do {
        auto masked_number = *block;
        while (masked_number != 0) {
            block_bit = bit_scan_forward(masked_number);
            list.push_back(std::distance(bits.begin(), block)*64 + block_bit);
            // always shift by one even if we find 0
            masked_number = *block & (~1ULL << block_bit);
        }
        ++block;
    } while (block != bits.end());

    return list;
}

inline void custom_bitset::unset_all() {
    for (auto &word : bits) {
        word = 0;
    }
}

inline BitCursor custom_bitset::first_bit() const {
    BitCursor cursor(0, 0);

    while (cursor.block_index < bits.size()) {
        if (bits[cursor.block_index] != 0) {
            cursor.bit = bit_scan_forward(bits[cursor.block_index]);
            // returns the index of the current block + the current bit
            return cursor;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
        ++cursor.block_index;
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return {_size/64, _size%64};
    //return UINT64_MAX;
}

inline BitCursor custom_bitset::first_bit_destructive() {
    const BitCursor cursor = first_bit();
    if (cursor.get_pos() != _size) unset_block_bit(cursor);
    return cursor;
}

// bug: not returning the first bit if set ...
// we use first_bit to get the first bit, and we start from there...
inline BitCursor custom_bitset::next_bit(const BitCursor& current_cursor) const {
    BitCursor cursor = current_cursor;

    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by last_bit+1
    const uint64_t masked_number = bits[cursor.block_index] & (~1ULL << cursor.bit);
    if (masked_number != 0) {
        cursor.bit = bit_scan_forward(masked_number);
        return cursor;
    }

    while (++cursor.block_index < bits.size()) {
        if (bits[cursor.block_index] != 0) {
            cursor.bit = bit_scan_forward(bits[cursor.block_index]);
            // returns the index of the current block + the current bit
            return cursor;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return {_size/64, _size%64};
    //return UINT64_MAX;
}

inline BitCursor custom_bitset::next_bit_destructive(const BitCursor& current_cursor) {
    const BitCursor cursor = next_bit(current_cursor);
    if (cursor.get_pos() != _size) unset_block_bit(cursor);
    return cursor;
}

inline BitCursor custom_bitset::last_bit() const {
    BitCursor cursor(bits.size() - 1, (_size-1)%64);

    do {
        if (bits[cursor.block_index] != 0) {
            cursor.bit = bit_scan_reverse(bits[cursor.block_index]);
            // returns the index of the current block + the current bit
            return cursor;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
    } while (cursor.block_index-- > 0);

    // probably the latter is better, but in a loop it doesn't make a difference
    return {_size/64, _size%64};
    //return UINT64_MAX;
}

inline BitCursor custom_bitset::last_bit_destructive() {
    const BitCursor cursor = last_bit();
    if (cursor.get_pos() != _size) unset_block_bit(cursor);
    return cursor;
}

inline BitCursor custom_bitset::prev_bit(const BitCursor& current_cursor) const {
    BitCursor cursor = current_cursor;

    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by last_bit+1
    const uint64_t masked_number = bits[cursor.block_index] & ~(~0ULL << cursor.bit);
    if (masked_number != 0) {
        cursor.bit = bit_scan_reverse(masked_number);
        return cursor;
    }

    while (cursor.block_index-- > 0) {
        if (bits[cursor.block_index] != 0) {
            cursor.bit = bit_scan_reverse(bits[cursor.block_index]);
            // returns the index of the current block + the current bit
            return cursor;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_bit;
        }
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return {_size/64, _size%64};
    //return UINT64_MAX;
}

inline BitCursor custom_bitset::prev_bit_destructive(const BitCursor& current_cursor) {
    const BitCursor cursor = prev_bit(current_cursor);
    if (cursor.get_pos() != _size) unset_block_bit(cursor);
    return cursor;
}

// TODO: optimize (use variable and update?)
inline uint64_t custom_bitset::degree() const {
    return n_set_bits();
}

inline void custom_bitset::negate() {
    for (unsigned long & bit : bits) {
        bit = ~bit;
    }
    bits.back() &= ~(~0ULL << (_size%64));
}

inline void custom_bitset::swap(custom_bitset &other) noexcept {
    const auto tmp = *this;
    *this = other;
    other = tmp;
}
