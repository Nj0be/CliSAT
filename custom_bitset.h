//
// Created by Beniamino Vagnarelli on 01/04/25.
//

#pragma once

#include <algorithm>
#include <numeric>
#include <ranges>
#include <utility>

class custom_bitset {
public:
    typedef uint64_t block_type;
    typedef std::size_t size_type;

    class reference {
        size_type block;     // current block index
        block_type bit;       // bit position inside block

    public:
        reference(const block_type block, const block_type bit): block(block), bit(bit) {}
        explicit reference(const size_type pos) : block(pos/64), bit(pos%64) {}

        bool operator==(const reference& other) const { return block == other.block && bit == other.bit; }

        size_type operator*() const { return block*64 + bit; }
        operator size_type() const { return **this; };

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
        iterator(custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
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
        using value_type = size_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;

        explicit const_iterator() : bs(nullptr), ref(0, 0) {}
        const_iterator(const custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
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
    size_type _size;
    std::vector<block_type> bits;

    static block_type bit_scan_forward(const block_type x) { return std::countr_zero(x); }
    static block_type bit_scan_reverse(const block_type x) { return 63 - std::countl_zero(x); }

    static block_type get_block(const size_type pos) { return pos/64; };
    static block_type get_block_bit(const size_type pos) { return pos%64; };

    // Used to allocate at least one block regardless of size
    static block_type blocks_needed(const size_type size) { return get_block(size + 63) | (size == 0); }

    // it's necessary because even if size it's 0, we still have a block inside bits vector
    static block_type get_last_block(const size_type size) { return get_block(size + 63) - (size != 0); }

public:
    explicit custom_bitset(size_type size);
    custom_bitset(size_type size, bool default_value);
    custom_bitset(const custom_bitset& other, size_type size);
    custom_bitset(const std::initializer_list<block_type> &v);
    custom_bitset(const std::vector<block_type> &v, size_type size);

    static custom_bitset before(const custom_bitset& src, const reference& threshold);
    static custom_bitset before(const custom_bitset& src, size_type threshold);
    static custom_bitset until(const custom_bitset& src, const reference& threshold);
    static custom_bitset until(const custom_bitset& src, size_type threshold);
    static custom_bitset after(const custom_bitset& src, const reference& threshold);
    static custom_bitset after(const custom_bitset& src, size_type threshold);
    static custom_bitset from(const custom_bitset& src, const reference& threshold);
    static custom_bitset from(const custom_bitset& src, size_type threshold);

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
    bool operator[](const size_type pos) const { return test(pos); };
    iterator operator[](const reference& ref) { return {this, ref}; };
    iterator operator[](const size_type pos) { return {this, pos}; };
    friend std::ostream& operator<<(std::ostream& stream, const custom_bitset& bb);
    explicit operator std::vector<block_type>() const;

    void set_bit(const reference& ref) { bits[ref.block] |= 1ULL << ref.bit; }
    void set_bit(const size_type pos) { set_bit(reference(pos)); }
    void unset_bit(const reference& ref) { bits[ref.block] &= ~(1ULL << ref.bit); }
    void unset_bit(const size_type pos) { unset_bit(reference(pos)); }
    // we move bit to first position and we mask everything that it isn't on position 1 to 0
    [[nodiscard]] bool test(const reference& ref) const { return bits[ref.block] >> ref.bit & 1; }
    [[nodiscard]] bool test(const size_type pos) const { return test(reference(pos)); }

    [[nodiscard]] reference front() const;
    reference pop_front();

    [[nodiscard]] reference next(reference ref) const;
    reference pop_next(reference ref);

    [[nodiscard]] reference back() const;
    reference pop_back();

    [[nodiscard]] reference prev(reference ref) const;
    reference pop_prev(reference ref);

    [[nodiscard]] size_type degree() const { return count(); };
    [[nodiscard]] size_type neighbors_degree() const;

    void negate();
    void reset();

    [[nodiscard]] size_type size() const { return _size; }
    [[nodiscard]] size_type count() const;

    void swap(custom_bitset& other) noexcept;

    [[nodiscard]] bool all() const;
    [[nodiscard]] bool any() const;
    [[nodiscard]] bool none() const;

    void resize(size_type new_size);

    void mask_before(const reference& threshold);
    void mask_before(size_type threshold);
    void mask_until(const reference& threshold);
    void mask_until(size_type threshold);
    void mask_after(const reference& threshold);
    void mask_after(size_type threshold);
    void mask_from(const reference& threshold);
    void mask_from(size_type threshold);
};

template <>
struct std::formatter<custom_bitset::reference> : std::formatter<custom_bitset::size_type> {
    auto format(const custom_bitset::reference& id, std::format_context& ctx) const {
        return std::formatter<custom_bitset::size_type>::format(*id, ctx);
    }
};

inline std::ostream& operator<<(std::ostream &stream, const custom_bitset &bb) {
    stream << '[';
    std::size_t tot = 0;

    for (auto v : bb) {
        stream << v << ' ';
        ++tot;
    }

    stream << '(' << tot << ")]";
    return stream;
}

inline custom_bitset::custom_bitset(const size_type size): _size(size), bits(blocks_needed(size)) {}

// we set everything to 1 or to 0
inline custom_bitset::custom_bitset(const size_type size, const bool default_value): _size(size), bits(blocks_needed(size), default_value*~0ULL) {
    // unset last part of last block (if there is one)
    if (get_block_bit(_size)) bits.back() &= ~(~0ULL << get_block_bit(_size));
}

inline custom_bitset::custom_bitset(const custom_bitset& other, const size_type size): custom_bitset(other) {
    this->resize(size);
}

inline custom_bitset::custom_bitset(const std::initializer_list<block_type> &v): _size(*std::ranges::max_element(v) + 1), bits(blocks_needed(_size)) {
    for (const auto pos: v) {
        set_bit(pos);
    }
}

inline custom_bitset::custom_bitset(const std::vector<block_type> &v, const size_type size): _size(size), bits(blocks_needed(size)) {
    for (const auto pos: v) {
        set_bit(pos);
    }
}

inline custom_bitset custom_bitset::before(const custom_bitset &src, const reference &threshold) {
    custom_bitset result(src);
    result.mask_before(threshold);
    return result;
}

inline custom_bitset custom_bitset::before(const custom_bitset &src, const size_type threshold) {
    return before(src, reference(threshold));
}

inline custom_bitset custom_bitset::until(const custom_bitset &src, const reference &threshold) {
    custom_bitset result(src);
    result.mask_until(threshold);
    return result;
}

inline custom_bitset custom_bitset::until(const custom_bitset &src, const size_type threshold) {
    return until(src, reference(threshold));
}

inline custom_bitset custom_bitset::after(const custom_bitset &src, const reference &threshold) {
    custom_bitset result(src);
    result.mask_after(threshold);
    return result;
}

inline custom_bitset custom_bitset::after(const custom_bitset &src, const size_type threshold) {
    return after(src, reference(threshold));
}

inline custom_bitset custom_bitset::from(const custom_bitset &src, const reference &threshold) {
    custom_bitset result(src);
    result.mask_from(threshold);
    return result;
}

inline custom_bitset custom_bitset::from(const custom_bitset &src, const size_type threshold) {
    return from(src, reference(threshold));
}

inline custom_bitset custom_bitset::operator&(const custom_bitset& other) const {
    custom_bitset bb(size());

    std::ranges::transform(bits, other.bits, bb.bits.begin(), std::bit_and<>{});

    return bb;
}

inline custom_bitset custom_bitset::operator|(const custom_bitset& other) const {
    const auto M = std::min(bits.size(), other.bits.size());
    custom_bitset bb(size());

    for (size_type i = 0; i < M; ++i)
        bb.bits[i] = bits[i] | other.bits[i];
    for (size_type i = M; i < bits.size(); ++i)
        bb.bits[i] = bits[i];

    return bb;
}

inline custom_bitset custom_bitset::operator^(const custom_bitset& other) const {
    const auto M = std::min(bits.size(), other.bits.size());
    custom_bitset bb(size());

    for (size_type i = 0; i < M; ++i)
        bb.bits[i] = bits[i] ^ other.bits[i];
    for (size_type i = M; i < bits.size(); ++i)
        bb.bits[i] = bits[i] ^ 0;

    return bb;
}

inline custom_bitset custom_bitset::operator-(const custom_bitset& other) const {
    const auto M = std::min(bits.size(), other.bits.size());
    custom_bitset bb(size());

    for (size_type i = 0; i < M; ++i)
        bb.bits[i] = bits[i] & ~other.bits[i];

    return bb;
}

inline custom_bitset custom_bitset::operator~() const {
    auto bb(*this);
    bb.negate();

    return bb;
}

inline bool custom_bitset::operator==(const custom_bitset &other) const {
    if (this == &other) return true;

    const auto M = std::min(bits.size(), other.bits.size());

    for (size_type i = 0; i < M; i++)
        if (bits[i] != other.bits[i]) return false;

    // if one is smaller, we check that the remaining bits are 0
    for (size_type i = M; i < bits.size(); i++)
        if (bits[i] != 0) return false;
    for (size_type i = M; i < other.bits.size(); ++i)
        if (other.bits[i] != 0) return false;

    return true;
}

inline custom_bitset& custom_bitset::operator&=(const custom_bitset &other) {
    std::ranges::transform(bits, other.bits, bits.begin(), std::bit_and<>{});

    return *this;
}

inline custom_bitset& custom_bitset::operator|=(const custom_bitset &other) {
    std::ranges::transform(bits, other.bits, bits.begin(), std::bit_or<>{});

    return *this;
}

inline custom_bitset& custom_bitset::operator^=(const custom_bitset &other) {
    const auto M = std::min(bits.size(), other.bits.size());

    for (size_type i = 0; i < M; ++i)
        bits[i] ^= other.bits[i];
    for (size_type i = M; i < bits.size(); ++i)
        bits[i] ^= 0;

    return *this;
}

inline custom_bitset& custom_bitset::operator-=(const custom_bitset &other) {
    const auto M = std::min(bits.size(), other.bits.size());

    for (size_type i = 0; i < M; ++i)
        bits[i] &= ~other.bits[i];

    return *this;
}

inline custom_bitset::operator std::vector<unsigned long>() const {
    std::vector<block_type> list;
    list.reserve(count());

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

inline custom_bitset::reference custom_bitset::front() const {
    for (auto ref = reference(0, 0); ref.block < bits.size(); ++ref.block) {
        if (bits[ref.block] != 0) {
            ref.bit = bit_scan_forward(bits[ref.block]);
            return ref;
        }
    }

    return reference(_size);
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
    const auto masked_number = bits[ref.block] & (~1ULL << ref.bit);
    if (masked_number != 0) {
        ref.bit = bit_scan_forward(masked_number);
        return ref;
    }

    for (++ref.block; ref.block < bits.size(); ++ref.block) {
        if (bits[ref.block] != 0) {
            ref.bit = bit_scan_forward(bits[ref.block]);
            return ref;
        }
    }

    return reference(_size);
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
            return ref;
        }
    } while (ref.block-- > 0);

    return reference(_size);
}

inline custom_bitset::reference custom_bitset::pop_back() {
    const auto ref = back();
    if (ref != size()) unset_bit(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::prev(reference ref) const {
    const auto masked_number = bits[ref.block] & ~(~0ULL << ref.bit);
    if (masked_number != 0) {
        ref.bit = bit_scan_reverse(masked_number);
        return ref;
    }

    while (ref.block-- > 0) {
        if (bits[ref.block] != 0) {
            ref.bit = bit_scan_reverse(bits[ref.block]);
            return ref;
        }
    }

    return reference(_size);
}

inline custom_bitset::reference custom_bitset::pop_prev(reference ref) {
    ref = prev(ref);
    if (ref != size()) unset_bit(ref);
    return ref;
}

inline void custom_bitset::negate() {
    for (size_type i = 0; i < bits.size() - 1; ++i) {
        bits[i] = ~bits[i];
    }
    bits.back() &= ~(~0ULL << get_block_bit(_size));
}

inline custom_bitset::size_type custom_bitset::count() const {
    return std::accumulate(
        bits.begin(), bits.end(), 0,
        [](auto sum, auto word) { return sum + std::popcount(word); }
    );
}

inline void custom_bitset::swap(custom_bitset &other) noexcept {
    std::swap(_size, other._size);
    bits.swap(other.bits);
}

inline void custom_bitset::resize(const size_type new_size) {
    if (_size == new_size) return;
    _size = new_size;
    bits.resize(blocks_needed(_size));
    bits[get_last_block(_size)] &= ~(~0ULL << get_block_bit(_size));
}

inline void custom_bitset::mask_before(const reference& threshold) {
    std::ranges::fill_n(bits.begin(), threshold.block, 0);
    bits[threshold.block] &= (~0ULL << threshold.bit);
}

inline void custom_bitset::mask_before(const size_type threshold) {
    mask_before(reference(threshold));
}

inline void custom_bitset::mask_until(const reference& threshold) {
    mask_before(reference(threshold + 1));
}

inline void custom_bitset::mask_until(const size_type threshold) {
    mask_before(reference(threshold + 1));
}

inline void custom_bitset::mask_after(const reference& threshold) {
    mask_from(reference(threshold + 1));
}

inline void custom_bitset::mask_after(const size_type threshold) {
    mask_from(reference(threshold + 1));
}

inline void custom_bitset::mask_from(const reference& threshold) {
    bits[threshold.block] &= ~(~0ULL << threshold.bit);
    std::ranges::fill(bits.begin() + threshold.block + 1, bits.end()-1, 0);
}

inline void custom_bitset::mask_from(const size_type threshold) {
    mask_from(reference(threshold));
}

inline bool custom_bitset::all() const {
    const auto ref = reference(_size);
    if (ref.bit == 0) return std::ranges::all_of(bits, [](const auto word) { return word == UINT64_MAX; });

    const bool all = std::ranges::all_of(bits.begin(), bits.end()-1, [](const auto word) { return word == UINT64_MAX; });
    if (all && ((bits.back() | (~0ULL << ref.bit)) == UINT64_MAX)) return true;
    return false;
}

inline bool custom_bitset::any() const {
    return std::ranges::any_of(bits, [](const auto word) { return word != 0; });
}

inline bool custom_bitset::none() const {
    return std::ranges::none_of(bits, [](const auto word) { return word != 0; });
}
