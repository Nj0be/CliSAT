//
// Created by Beniamino Vagnarelli on 01/04/25.
//

#pragma once

#include <immintrin.h> //__bsrd, __bsrq, etc
#include <algorithm>

class custom_bitset {
public:
    class iterator {
        custom_bitset* bs;
        std::vector<uint64_t>::iterator block;     // current block index
        uint8_t bit;       // bit position inside block

    public:
        using value_type = uint64_t;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;

        explicit iterator() : bs(nullptr), block(nullptr), bit(0) {}
        iterator(custom_bitset* bitset, const uint64_t pos) : bs(bitset), block(bs->bits.begin() + pos/64), bit(pos%64) {}
        // iterator(custom_bitset* bitset, const uint64_t block_index, const uint8_t bit) : bs(bitset), block(bs->bits.begin() + block_index), bit(bit) {}
        // iterator(custom_bitset* bitset, const std::vector<uint64_t>::iterator block, const uint8_t bit) : bs(bitset), block(block), bit(bit) {}

        value_type operator*() const { return (block - bs->bits.begin())*64 + bit; }
        //iterator operator-(const uint64_t x) const { return block_index * 64 + bit; }

        // prefix increment
        iterator& operator++() { *this = bs->next(*this); return *this; }
        // postfix increment
        iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        // prefix decrement
        iterator& operator--() { *this = bs->prev(*this); return *this; }
        // postfix decrement
        iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        bool operator==(const iterator& other) const { return bs == other.bs && block == other.block && bit == other.bit; }
        //bool operator!=(const iterator& other) const { return !(*this == other); }

        friend class custom_bitset;
    };

    class const_iterator {
        const custom_bitset* bs;
        std::vector<uint64_t>::const_iterator block;     // current block index
        uint8_t bit;       // bit position inside block

    public:
        using value_type = uint64_t;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;

        explicit const_iterator() : bs(nullptr), block(nullptr), bit(0) {}
        const_iterator(const custom_bitset* bitset, const uint64_t pos) : bs(bitset), block(bs->bits.begin() + pos/64), bit(pos%64) {}
        // const_iterator(const custom_bitset* bitset, const uint64_t block_index, const uint8_t bit) : bs(bitset), block(bs->bits.begin() + block_index), bit(bit) {}
        // const_iterator(const custom_bitset* bitset, const std::vector<uint64_t>::iterator block, const uint8_t bit) : bs(bitset), block(block), bit(bit) {}
        explicit const_iterator(const iterator& it) : bs(it.bs), block(it.block), bit(it.bit) {}

        value_type operator*() const { return (block - bs->bits.begin())*64 + bit; }
        //iterator operator-(const uint64_t x) const { return block_index * 64 + bit; }

        // prefix increment
        const_iterator& operator++() { *this = bs->next(*this); return *this; }
        // postfix increment
        const_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        // prefix decrement
        const_iterator& operator--() { *this = bs->prev(*this); return *this; }
        // postfix decrement
        const_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        bool operator==(const const_iterator& other) const { return bs == other.bs && block == other.block && bit == other.bit; }
        //bool operator!=(const const_iterator& other) const { return !(*this == other); }

        friend class custom_bitset;
    };

    [[nodiscard]] iterator begin() { return front(); }
    [[nodiscard]] const_iterator begin() const { return front(); }
    [[nodiscard]] iterator end() { return {this, _size}; }
    [[nodiscard]] const_iterator end() const { return {this, _size}; }

private:
    uint64_t _size;
    std::vector<uint64_t> bits;

    static uint8_t bit_scan_forward(const uint64_t x) { return __builtin_ctzll(x); }
    static uint8_t bit_scan_reverse(const uint64_t x) { return __bsrq(x); }

    static uint64_t get_block(const uint64_t pos) { return pos/64; };
    static uint64_t get_block_bit(const uint64_t pos) { return pos%64; };

    // Used to allocate at least one block regardless of size
    static uint64_t blocks_needed(const uint64_t size) { return get_block(size + 63) | (size == 0); }

    // it's necessary because even if size it's 0, we still have a block inside bits vector
    static uint64_t get_last_block(const uint64_t size) { return get_block(size + 63) - (size != 0); }

    // static void set_block_bit(const std::vector<uint64_t>::iterator &block, const uint64_t &bit) { *block |= 1ULL << bit; }
    static void set_block_bit(const iterator& it) { *it.block |= 1ULL << it.bit; }
    // static void unset_block_bit(const std::vector<uint64_t>::iterator &block, const uint8_t &bit) { *block &= ~(1ULL << bit); }
    static void unset_block_bit(const iterator& it) { *it.block &= ~(1ULL << it.bit); }

    template <typename Iter>
    Iter front_impl(const Iter& begin, const Iter& end) const;

    template <typename Iter>
    Iter next_impl(Iter it, const Iter& end) const;

    template <typename Iter>
    Iter back_impl(const Iter& begin, const Iter& end) const;

    template <typename Iter>
    Iter prev_impl(Iter it, const Iter& end) const;

    [[nodiscard]] iterator first() { return {this, 0}; }
    [[nodiscard]] const_iterator first() const { return {this, 0}; }
    [[nodiscard]] iterator last() { return {this, _size-1}; }
    [[nodiscard]] const_iterator last() const { return {this, _size-1}; }

public:
    explicit custom_bitset(uint64_t size);
    custom_bitset(uint64_t size, bool default_value);
    custom_bitset(uint64_t size, uint64_t set_first_n_bits);
    custom_bitset(const custom_bitset& other, uint64_t size);
    explicit custom_bitset(const std::vector<uint64_t>& v);
    custom_bitset(const std::vector<uint64_t> &v, uint64_t size);

    custom_bitset operator&(const custom_bitset& other) const;
    custom_bitset operator|(const custom_bitset& other) const;
    custom_bitset operator~() const;
    custom_bitset operator-(const custom_bitset& other) const;
    bool operator==(const custom_bitset& other) const;
    custom_bitset& operator&=(const custom_bitset& other);
    custom_bitset& operator|=(const custom_bitset& other);
    custom_bitset& operator-=(const custom_bitset& other);
    bool operator[](const const_iterator& it) const { return test(it); };
    bool operator[](const uint64_t pos) const { return test(pos); };
    friend std::ostream& operator<<(std::ostream& stream, const custom_bitset& bb);
    explicit operator std::vector<uint64_t>() const;

    // TODO: not SAFE!
    static void set_bit(const iterator& it) { *it.block |= 1ULL << it.bit; }
    void set_bit(const uint64_t pos) { set_bit({this, pos}); }
    static void unset_bit(const iterator& it) { *it.block &= ~(1ULL << it.bit); }
    void unset_bit(const uint64_t pos) { unset_bit({this, pos}); }
    // we move bit to first position and we mask everything that it isn't on position 1 to 0
    [[nodiscard]] static bool test(const const_iterator& it) { return *it.block >> get_block_bit(it.bit) & 1; }
    [[nodiscard]] bool test(const uint64_t pos) const { return test({this, pos}); }

    [[nodiscard]] iterator front() { return front_impl(first(), end()); };
    [[nodiscard]] const_iterator front() const { return front_impl(first(), end()); }
    iterator pop_front();

    [[nodiscard]] iterator next(const iterator& it) { return next_impl(it, end()); }
    [[nodiscard]] const_iterator next(const const_iterator& it) const { return next_impl(it, end()); }
    iterator pop_next(iterator it);

    [[nodiscard]] iterator back() { return back_impl(last(), end()); }
    [[nodiscard]] const_iterator back() const { return back_impl(last(), end()); }
    iterator pop_back();

    [[nodiscard]] iterator prev(const iterator& it) { return prev_impl(it, end()); }
    [[nodiscard]] const_iterator prev(const const_iterator& it) const { return prev_impl(it, end()); }
    iterator pop_prev(iterator it);

    [[nodiscard]] uint64_t degree() const { return count(); };
    [[nodiscard]] uint64_t neighbors_degree() const;

    void negate();
    void reset();

    [[nodiscard]] uint64_t size() const { return _size; }
    [[nodiscard]] uint64_t count() const;

    void swap(custom_bitset& other) noexcept;

    [[nodiscard]] bool all() const;
    [[nodiscard]] bool any() const;
    [[nodiscard]] bool none() const;

    custom_bitset& resize(uint64_t new_size);
};

inline std::ostream& operator<<(std::ostream &stream, const custom_bitset &bb) {
    std::string out = "[";
    auto tot = 0;

    for (const auto v : bb) {
        out += std::format("{} ", v);
        tot++;
    }
    out += std::format("({})]", tot);

    return (stream << out);
}

inline custom_bitset::custom_bitset(const uint64_t size): custom_bitset(size, false) {}

// we set everything to 1 or to 0
inline custom_bitset::custom_bitset(const uint64_t size, const bool default_value): _size(size), bits(blocks_needed(size), default_value*~0ULL) {
    // unset last part of last block (if there is one)
    if (get_block_bit(_size)) bits.back() &= ~(~0ULL << get_block_bit(_size));
}

inline custom_bitset::custom_bitset(const uint64_t size, const uint64_t set_first_n_bits): _size(size), bits(blocks_needed(size)) {
    const auto n_bits = std::min(set_first_n_bits, size);

    const auto n_block = get_block(n_bits-1);

    for (uint64_t i = 0; i <= n_block; i++) {
        bits[i] = UINT64_MAX;
    }

    bits[n_block] &= ~(~0ULL << get_block_bit(n_bits));
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
    custom_bitset bb(std::max(size(), other.size()));
    for (uint64_t i = 0; i < bits.size(); ++i) {
        if (i < other.bits.size()) bb.bits[i] = bits[i] & other.bits[i];
    }
    return bb;

    /*
    custom_bitset bb(std::max(size(), other.size()));
    bb.bits = bits;
    bb &= other;
    return bb;
    */

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

// TODO: broken with size 0
inline custom_bitset& custom_bitset::operator-=(const custom_bitset &other) {
    const auto min_length = std::min(size(), other.size());
    const auto min_block = get_block(min_length-1)+ 1;

    // equivalent to *this &= ~other; but faster
    for (uint64_t i = 0; i < min_block; ++i) {
        bits[i] &= ~other.bits[i];
    }

    return *this;
}

inline custom_bitset::operator std::vector<unsigned long>() const {
    std::vector<uint64_t> list;

    for (const auto v : *this) {
        list.push_back(v);
    }

    return list;
}

inline void custom_bitset::reset() {
    for (auto &word : bits) {
        word = 0;
    }
}

template<typename Iter>
Iter custom_bitset::front_impl(const Iter& begin, const Iter& end) const {
    auto it = begin;

    while (it.block != bits.end()) {
        if (*it.block != 0) {
            it.bit = bit_scan_forward(*it.block);
            // returns the index of the current block + the current bit
            return it;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | back;
        }
        ++it.block;
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return end;
    //return UINT64_MAX;
}

inline custom_bitset::iterator custom_bitset::pop_front() {
    const auto it = front();
    if (it != end()) unset_block_bit(it);
    return it;
}

template<typename Iter>
Iter custom_bitset::next_impl(Iter it, const Iter& end) const {
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by back+1
    const uint64_t masked_number = *it.block & (~1ULL << it.bit);
    if (masked_number != 0) {
        it.bit = bit_scan_forward(masked_number);
        return it;
    }

    while (++it.block != bits.end()) {
        if (*it.block != 0) {
            it.bit = bit_scan_forward(*it.block);
            // returns the index of the current block + the current bit
            return it;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | back;
        }
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return end;
    //return UINT64_MAX;
}

inline custom_bitset::iterator custom_bitset::pop_next(iterator it) {
    it = next(it);
    if (it != end()) unset_block_bit(it);
    return it;
}

template<typename Iter>
Iter custom_bitset::back_impl(const Iter& begin, const Iter& end) const {
    auto it = begin;

    do {
        if (*it.block != 0) {
            it.bit= bit_scan_reverse(*it.block);
            // returns the index of the current block + the current bit
            return it;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_back;
        }
    } while (it.block-- != bits.begin());

    // probably the latter is better, but in a loop it doesn't make a difference
    return end;
    //return UINT64_MAX;
}

inline custom_bitset::iterator custom_bitset::pop_back() {
    const auto it = back();
    if (it != end()) unset_block_bit(it);
    return it;
}

template<typename Iter>
Iter custom_bitset::prev_impl(Iter it, const Iter& end) const {
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by back+1
    const uint64_t masked_number = *it.block & ~(~0ULL << it.bit);
    if (masked_number != 0) {
        it.bit = bit_scan_reverse(masked_number);
        return it;
    }

    while (it.block-- != bits.begin()) {
        if (*it.block != 0) {
            it.bit = bit_scan_reverse(*it.block);
            // returns the index of the current block + the current bit
            return it;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | back;
        }
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return end;
    //return UINT64_MAX;
}

inline custom_bitset::iterator custom_bitset::pop_prev(iterator it) {
    it = prev(it);
    if (it != end()) unset_block_bit(it);
    return it;
}

inline void custom_bitset::negate() {
    for (unsigned long & bit : bits) {
        bit = ~bit;
    }
    bits.back() &= ~(~0ULL << (get_block_bit(_size)));
}

inline uint64_t custom_bitset::count() const {
    uint64_t tot = 0;
    for (const auto word : bits) {
        tot += std::popcount(word);
    }

    return tot;
}

inline void custom_bitset::swap(custom_bitset &other) noexcept {
    const auto tmp = *this;
    *this = other;
    other = tmp;
}

inline custom_bitset & custom_bitset::resize(const uint64_t new_size) {
    if (_size == new_size) return *this;
    _size = new_size;
    bits.resize(blocks_needed(_size));
    bits[get_last_block(_size)] &= ~(~0ULL << get_block_bit(_size));
    return *this;
}

inline bool custom_bitset::all() const {
    return std::ranges::all_of(bits, [](const uint64_t word) { return word == UINT64_MAX; });
}

inline bool custom_bitset::any() const {
    return std::ranges::any_of(bits, [](const uint64_t word) { return word != 0; });
}

inline bool custom_bitset::none() const {
    return std::ranges::none_of(bits, [](const uint64_t word) { return word != 0; });
}
