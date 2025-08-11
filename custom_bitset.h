//
// Created by Beniamino Vagnarelli on 01/04/25.
//

#pragma once

#include <immintrin.h> //__bsrd, __bsrq, etc
#include <algorithm>

class custom_bitset {
public:
    class reference {
        uint64_t block;     // current block index
        uint8_t bit;       // bit position inside block

    public:
        reference(const uint64_t block, const uint8_t bit): block(block), bit(bit) {}
        explicit reference(const uint64_t pos) : block(pos/64), bit(pos%64) {}

        bool operator==(const reference& other) const { return block == other.block && bit == other.bit; }

        uint64_t operator*() const { return block*64 + bit; }
        operator uint64_t() const { return **this; };

        friend class custom_bitset;
    };

    class iterator {
        custom_bitset* bs;
        reference ref;

    public:
        using value_type = reference;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;

        explicit iterator() : bs(nullptr), ref(0, 0) {}
        iterator(custom_bitset* bitset, const uint64_t pos) : bs(bitset), ref(pos) {}
        iterator(custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}

        reference operator*() const { return ref; }

        iterator& operator=(const bool value) {
            if (value) bs->set_bit(ref);
            else bs->unset_bit(ref);
            return *this;
        }

        // Conversion to bool for reading
        explicit operator bool() const { return bs->test(ref); }

        // prefix increment
        iterator& operator++() { ref = bs->next(ref); return *this; }
        // postfix increment
        iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        // prefix decrement
        iterator& operator--() { ref = bs->prev(ref); return *this; }
        // postfix decrement
        iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        bool operator==(const iterator& other) const { return bs == other.bs && ref == other.ref; }
        //bool operator!=(const iterator& other) const { return !(*this == other); }

        friend class custom_bitset;
    };

    class const_iterator {
        const custom_bitset* bs;
        reference ref;

    public:
        using value_type = uint64_t;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;

        explicit const_iterator() : bs(nullptr), ref(0, 0) {}
        const_iterator(const custom_bitset* bitset, const uint64_t pos) : bs(bitset), ref(pos) {}
        const_iterator(const custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}
        explicit const_iterator(const iterator& it) : bs(it.bs), ref(it.ref) {}

        reference operator*() const { return ref; }

        explicit operator bool() const {
            return bs->test(ref);
        }

        // prefix increment
        const_iterator& operator++() { ref = bs->next(ref); return *this; }
        // postfix increment
        const_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        // prefix decrement
        const_iterator& operator--() { ref = bs->prev(ref); return *this; }
        // postfix decrement
        const_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        bool operator==(const const_iterator& other) const { return bs == other.bs && ref == other.ref; }
        //bool operator!=(const const_iterator& other) const { return !(*this == other); }

        friend class custom_bitset;
    };

    [[nodiscard]] iterator begin() { return {this, front()}; }
    [[nodiscard]] const_iterator begin() const { return {this, front()}; }
    [[nodiscard]] iterator end() { return {this, _size}; }
    [[nodiscard]] const_iterator end() const { return {this, _size}; }
    [[nodiscard]] iterator rbegin() { return {this, back()}; }
    [[nodiscard]] const_iterator rbegin() const { return {this, back()}; }
    [[nodiscard]] iterator rend() { return end(); }
    [[nodiscard]] const_iterator rend() const { return end(); }

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

public:
    explicit custom_bitset(uint64_t size);
    custom_bitset(uint64_t size, bool default_value);
    custom_bitset(const custom_bitset& other, uint64_t size);
    custom_bitset(const std::vector<uint64_t> &v, uint64_t size);

    custom_bitset operator~() const;
    bool operator==(const custom_bitset& other) const;
    custom_bitset operator&(const custom_bitset& other) const;
    custom_bitset operator|(const custom_bitset& other) const;
    custom_bitset operator^(const custom_bitset& other) const;
    custom_bitset operator-(const custom_bitset& other) const;
    custom_bitset& operator&=(const custom_bitset& other);
    custom_bitset& operator|=(const custom_bitset& other);
    custom_bitset& operator^=(const custom_bitset &other);
    custom_bitset& operator-=(const custom_bitset& other);
    bool operator[](const reference& ref) const { return test(ref); };
    bool operator[](const uint64_t pos) const { return test(pos); };
    iterator operator[](const reference& ref) { return {this, ref}; };
    iterator operator[](const uint64_t pos) { return {this, pos}; };
    friend std::ostream& operator<<(std::ostream& stream, const custom_bitset& bb);
    explicit operator std::vector<uint64_t>() const;

    // TODO: not SAFE!
    void set_bit(const reference& ref) { bits[ref.block] |= 1ULL << ref.bit; }
    void set_bit(const uint64_t pos) { set_bit(reference(pos)); }
    void unset_bit(const reference& ref) { bits[ref.block] &= ~(1ULL << ref.bit); }
    void unset_bit(const uint64_t pos) { unset_bit(reference(pos)); }
    // we move bit to first position and we mask everything that it isn't on position 1 to 0
    [[nodiscard]] bool test(const reference& ref) const { return bits[ref.block] >> ref.bit & 1; }
    [[nodiscard]] bool test(const uint64_t pos) const { return test(reference(pos)); }

    [[nodiscard]] reference front() const;
    reference pop_front();

    [[nodiscard]] reference next(reference ref) const;
    //[[nodiscard]] reference next(const reference& ref) const;
    reference pop_next(reference ref);

    [[nodiscard]] reference back() const;
    reference pop_back();

    [[nodiscard]] reference prev(reference ref) const;
    //[[nodiscard]] reference prev(const reference& ref) const;
    reference pop_prev(reference ref);

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

    void resize(uint64_t new_size);

    void mask_before(const reference& threshold);
    void mask_before(uint64_t threshold);
    void mask_until(const reference& threshold);
    void mask_until(uint64_t threshold);
    void mask_after(const reference& threshold);
    void mask_after(uint64_t threshold);
    void mask_from(const reference& threshold);
    void mask_from(uint64_t threshold);
};

inline std::ostream& operator<<(std::ostream &stream, const custom_bitset &bb) {
    std::string out = "[";
    auto tot = 0;

    for (const auto v : bb) {
        out += std::format("{} ", *v);
        tot++;
    }
    out += std::format("({})]", tot);

    return (stream << out);
}

inline custom_bitset::custom_bitset(const uint64_t size): _size(size), bits(blocks_needed(size)) {}

// we set everything to 1 or to 0
inline custom_bitset::custom_bitset(const uint64_t size, const bool default_value): _size(size), bits(blocks_needed(size), default_value*~0ULL) {
    // unset last part of last block (if there is one)
    if (get_block_bit(_size)) bits.back() &= ~(~0ULL << get_block_bit(_size));
}

inline custom_bitset::custom_bitset(const custom_bitset& other, const uint64_t size): custom_bitset(other) {
    this->resize(size);
}

inline custom_bitset::custom_bitset(const std::vector<uint64_t> &v, const uint64_t size): _size(size), bits(blocks_needed(size)) {
    for (const auto pos: v) {
        set_bit(pos);
    }
}


inline custom_bitset custom_bitset::operator&(const custom_bitset& other) const {
    const auto M = std::min(bits.size(), other.bits.size());
    custom_bitset bb(size());

    for (uint64_t i = 0; i < M; ++i)
        bb.bits[i] = bits[i] & other.bits[i];

    return bb;
}

inline custom_bitset custom_bitset::operator|(const custom_bitset& other) const {
    const auto M = std::min(bits.size(), other.bits.size());
    custom_bitset bb(size());

    for (uint64_t i = 0; i < M; ++i)
        bb.bits[i] = bits[i] | other.bits[i];
    for (uint64_t i = M; i < bits.size(); ++i)
        bb.bits[i] = bits[i];

    return bb;
}

inline custom_bitset custom_bitset::operator^(const custom_bitset& other) const {
    const auto M = std::min(bits.size(), other.bits.size());
    custom_bitset bb(size());

    for (uint64_t i = 0; i < M; ++i)
        bb.bits[i] = bits[i] ^ other.bits[i];
    for (uint64_t i = M; i < bits.size(); ++i)
        bb.bits[i] = bits[i] ^ 0;

    return bb;
}

inline custom_bitset custom_bitset::operator-(const custom_bitset& other) const {
    const auto M = std::min(bits.size(), other.bits.size());
    custom_bitset bb(size());

    for (uint64_t i = 0; i < M; ++i)
        bb.bits[i] = bits[i] & ~other.bits[i];

    return bb;
}

/*
constexpr custom_bitset operator&(const custom_bitset& lhs, const custom_bitset& rhs) {
    return custom_bitset(lhs) &= rhs;
}

constexpr custom_bitset operator|(const custom_bitset& lhs, const custom_bitset& rhs) {
    return custom_bitset(lhs) |= rhs;
}

constexpr custom_bitset operator^(const custom_bitset& lhs, const custom_bitset& rhs) {
    return custom_bitset(lhs) ^= rhs;
}

constexpr custom_bitset operator-(const custom_bitset& lhs, const custom_bitset& rhs) {
    return custom_bitset(lhs) -= rhs;
}
*/

inline custom_bitset custom_bitset::operator~() const {
    auto bb(*this);
    bb.negate();

    return bb;
}

inline bool custom_bitset::operator==(const custom_bitset &other) const {
    if (this == &other) return true;

    const auto M = std::min(bits.size(), other.bits.size());

    for (uint64_t i = 0; i < M; i++)
        if (bits[i] != other.bits[i]) return false;
    // if other is smaller, we check that the remaining bits are 0
    for (uint64_t i = M; i < bits.size(); i++)
        if (bits[i] != 0) return false;

    return true;
}

inline custom_bitset& custom_bitset::operator&=(const custom_bitset &other) {
    const auto M = std::min(bits.size(), other.bits.size());

    for (uint64_t i = 0; i < M; ++i)
        bits[i] &= other.bits[i];
    for (uint64_t i = M; i < bits.size(); ++i)
        bits[i] = 0;
        //bits[i] &= 0;

    return *this;
}

inline custom_bitset& custom_bitset::operator|=(const custom_bitset &other) {
    const auto M = std::min(bits.size(), other.bits.size());

    for (uint64_t i = 0; i < M; ++i)
        bits[i] |= other.bits[i];
    //for (uint64_t i = M; i < bits.size(); ++i)
        //bits[i] |= 0;

    return *this;
}

inline custom_bitset& custom_bitset::operator^=(const custom_bitset &other) {
    const auto M = std::min(bits.size(), other.bits.size());

    for (uint64_t i = 0; i < M; ++i)
        bits[i] ^= other.bits[i];
    for (uint64_t i = M; i < bits.size(); ++i)
        bits[i] ^= 0;

    return *this;
}

inline custom_bitset& custom_bitset::operator-=(const custom_bitset &other) {
    const auto M = std::min(bits.size(), other.bits.size());

    for (uint64_t i = 0; i < M; ++i)
        bits[i] &= ~other.bits[i];

    return *this;
}

inline custom_bitset::operator std::vector<unsigned long>() const {
    std::vector<uint64_t> list;

    for (const auto v : *this) {
        list.push_back(*v);
    }

    return list;
}

inline void custom_bitset::reset() {
    for (auto &word : bits) {
        word = 0;
    }
}

/*
inline custom_bitset::reference custom_bitset::front() const {
    auto it = bits.begin();

    do {
        if (*it != 0) {
            // returns the index of the current block + the current bit
            return reference(std::distance(bits.begin(), it), bit_scan_forward(*it));
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | back;
        }
    } while (++it != bits.end());

    // probably the latter is better, but in a loop it doesn't make a difference
    return reference(_size);
    //return UINT64_MAX;
}

inline custom_bitset::reference custom_bitset::pop_front() {
    const auto ref = front();
    if (ref != size()) unset_bit(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::next(const reference& ref) const {
    auto it = bits.begin() + ref.block;
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by back+1
    const uint64_t masked_number = *it & (~1ULL << ref.bit);
    if (masked_number != 0) {
        return {ref.block, bit_scan_forward(masked_number)};
    }

    while (++it != bits.end()) {
        if (*it != 0) {
            // returns the index of the current block + the current bit
            return reference(std::distance(bits.begin(), it), bit_scan_forward(*it));
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | back;
        }
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return reference(_size);
    //return UINT64_MAX;
}

inline custom_bitset::reference custom_bitset::pop_next(reference ref) {
    ref = next(ref);
    if (ref != size()) unset_bit(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::back() const {
    auto it = bits.end()-1;

    do {
        if (*it != 0) {
            // returns the index of the current block + the current bit
            return reference(std::distance(bits.begin(), it), bit_scan_reverse(*it));
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_back;
        }
    } while (it-- != bits.begin());

    // probably the latter is better, but in a loop it doesn't make a difference
    return reference(_size);
    //return UINT64_MAX;
}

inline custom_bitset::reference custom_bitset::pop_back() {
    const auto ref = back();
    if (ref != size()) unset_bit(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::prev(const reference& ref) const {
    auto it = bits.begin() + ref.block;
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by back+1
    const uint64_t masked_number = *it & ~(~0ULL << ref.bit);
    if (masked_number != 0) {
        return {ref.block, bit_scan_reverse(masked_number)};
    }

    while (it-- != bits.begin()) {
        if (*it != 0) {
            // returns the index of the current block + the current bit
            return reference(std::distance(bits.begin(), it), bit_scan_reverse(*it));
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | back;
        }
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return reference(_size);
    //return UINT64_MAX;
}

inline custom_bitset::reference custom_bitset::pop_prev(reference ref) {
    ref = prev(ref);
    if (ref != size()) unset_bit(ref);
    return ref;
}
*/

inline custom_bitset::reference custom_bitset::front() const {
    auto ref = reference(0, 0);

    while (ref.block < bits.size()) {
        if (bits[ref.block] != 0) {
            ref.bit = bit_scan_forward(bits[ref.block]);
            // returns the index of the current block + the current bit
            return ref;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | back;
        }
        ++ref.block;
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return reference(_size);
    //return UINT64_MAX;
}

inline custom_bitset::reference custom_bitset::pop_front() {
    const auto ref = front();
    if (ref != size()) unset_bit(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::next(reference ref) const {
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by back+1
    const uint64_t masked_number = bits[ref.block] & (~1ULL << ref.bit);
    if (masked_number != 0) {
        ref.bit = bit_scan_forward(masked_number);
        return ref;
    }

    while (++ref.block < bits.size()) {
        if (bits[ref.block] != 0) {
            ref.bit = bit_scan_forward(bits[ref.block]);
            // returns the index of the current block + the current bit
            return ref;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | back;
        }
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return reference(_size);
    //return UINT64_MAX;
}

inline custom_bitset::reference custom_bitset::pop_next(reference ref) {
    ref = next(ref);
    if (ref != size()) unset_bit(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::back() const {
    reference ref(_size-1);

    do {
        if (bits[ref.block] != 0) {
            ref.bit= bit_scan_reverse(bits[ref.block]);
            // returns the index of the current block + the current bit
            return ref;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | last_back;
        }
    } while (ref.block-- > 0);

    // probably the latter is better, but in a loop it doesn't make a difference
    return reference(_size);
    //return UINT64_MAX;
}

inline custom_bitset::reference custom_bitset::pop_back() {
    const auto ref = back();
    if (ref != size()) unset_bit(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::prev(reference ref) const {
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by back+1
    const uint64_t masked_number = bits[ref.block] & ~(~0ULL << ref.bit);
    if (masked_number != 0) {
        ref.bit = bit_scan_reverse(masked_number);
        return ref;
    }

    while (ref.block-- > 0) {
        if (bits[ref.block] != 0) {
            ref.bit = bit_scan_reverse(bits[ref.block]);
            // returns the index of the current block + the current bit
            return ref;
            // it's equivalent but not any faster
            //return std::distance(bits.begin(), last_block) << 6 | back;
        }
    }

    // probably the latter is better, but in a loop it doesn't make a difference
    return reference(_size);
    //return UINT64_MAX;
}

inline custom_bitset::reference custom_bitset::pop_prev(reference ref) {
    ref = prev(ref);
    if (ref != size()) unset_bit(ref);
    return ref;
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

inline void custom_bitset::resize(const uint64_t new_size) {
    if (_size == new_size) return;
    _size = new_size;
    bits.resize(blocks_needed(_size));
    bits[get_last_block(_size)] &= ~(~0ULL << get_block_bit(_size));
}

inline void custom_bitset::mask_before(const reference& threshold) {
    for (uint64_t block = 0; block < threshold.block; ++block) bits[block] = 0;
    bits[threshold.block] &= (~0ULL << threshold.bit);
}

inline void custom_bitset::mask_before(const uint64_t threshold) {
    mask_before(reference(threshold));
}

inline void custom_bitset::mask_until(const reference& threshold) {
    mask_before(reference(*threshold + 1));
}

inline void custom_bitset::mask_until(const uint64_t threshold) {
    mask_before(reference(threshold + 1));
}

inline void custom_bitset::mask_after(const reference& threshold) {
    mask_from(reference(*threshold + 1));
}

inline void custom_bitset::mask_after(const uint64_t threshold) {
    mask_from(reference(threshold + 1));
}

inline void custom_bitset::mask_from(const reference& threshold) {
    bits[threshold.block] &= ~(~0ULL << threshold.bit);
    for (uint64_t block = threshold.block + 1; block < bits.size(); ++block) bits[block] = 0;
}

inline void custom_bitset::mask_from(const uint64_t threshold) {
    mask_from(reference(threshold));
}

inline bool custom_bitset::all() const {
    const auto ref = reference(_size);
    if (ref.bit == 0) return std::ranges::all_of(bits, [](const uint64_t word) { return word == UINT64_MAX; });

    const bool all = std::ranges::all_of(bits.begin(), bits.end()-1, [](const uint64_t word) { return word == UINT64_MAX; });
    if (all && ((bits.back() | (~0ULL << ref.bit)) == UINT64_MAX)) return true;
    return false;
}

inline bool custom_bitset::any() const {
    return std::ranges::any_of(bits, [](const uint64_t word) { return word != 0; });
}

inline bool custom_bitset::none() const {
    return std::ranges::none_of(bits, [](const uint64_t word) { return word != 0; });
}
