//
// Created by Beniamino Vagnarelli on 01/04/25.
//

module;

#include <algorithm>
#include <cassert>
#include <numeric>
#include <ranges>
#include <utility>
#include <vector>
#include <format>
#include <iostream>

export module custom_bitset;

export class custom_bitset {
public:
    // 32 bit architecture will use uint32_t, 64 bit will use uint64_t
    typedef uint_fast32_t block_type;
    typedef std::size_t size_type;
    static constexpr int block_size = std::numeric_limits<block_type>::digits;

    class reference {
        size_type block;    // current block index
        size_type bit;            // bit position inside block

    public:
        reference(const size_type block, const size_type bit): block(block), bit(bit) {}
        explicit reference(const size_type pos) : block(get_block(pos)), bit(get_block_bit(pos)) {}

        bool operator==(const reference& other) const { return block == other.block && bit == other.bit; }

        size_type operator*() const { return block*block_size + bit; }
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

        consteval explicit iterator() : bs(nullptr), ref(0, 0) {}
        iterator(custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        iterator(custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}

        reference operator*() const { return ref; }

        iterator& operator=(const bool value) {
            bs->set(value);
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
        using value_type = reference;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;

        consteval explicit const_iterator() : bs(nullptr), ref(0, 0) {}
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
    std::vector<block_type> _bits;

    static size_type bit_scan_forward(const block_type x) { return std::countr_zero(x); }
    static size_type bit_scan_reverse(const block_type x) { return block_size-1 - std::countl_zero(x); }

    static size_type get_block(const size_type pos) { return pos/block_size; };
    static size_type get_block_bit(const size_type pos) { return pos%block_size; };

    // Used to allocate at least one block regardless of size
    static size_type blocks_needed(const size_type size) { return get_block(size + block_size-1) | (size == 0); }

    // it's necessary because even if size it's 0, we still have a block inside bits vector
    static size_type get_last_block(const size_type size) { return get_block(size + block_size-1) - (size != 0); }

    static block_type mask_bit(const size_type bit) { return block_type{1} << bit; }
    static block_type below_mask(const size_type bit) { return ~from_mask(bit); }
    static block_type until_mask(const size_type bit) { return ~after_mask(bit); }
    static block_type from_mask(const size_type bit) { return ~block_type{0} << bit; }
    static block_type after_mask(const size_type bit) { return ~block_type{1} << bit; }

public:
    custom_bitset() : custom_bitset(0) {}
    explicit custom_bitset(size_type size);
    custom_bitset(size_type size, bool default_value);
    custom_bitset(custom_bitset other, size_type size);
    custom_bitset(const std::initializer_list<block_type>& v);
    custom_bitset(const std::vector<size_type>& v, size_type size);

    static custom_bitset before(custom_bitset src, const reference& ref);
    static custom_bitset before(const custom_bitset& src, size_type pos);
    static custom_bitset until(custom_bitset src, const reference& ref);
    static custom_bitset until(const custom_bitset& src, size_type pos);
    static custom_bitset after(custom_bitset src, const reference& ref);
    static custom_bitset after(const custom_bitset& src, size_type pos);
    static custom_bitset from(custom_bitset src, const reference& ref);
    static custom_bitset from(const custom_bitset& src, size_type pos);
    static custom_bitset complement(custom_bitset src);

    custom_bitset operator~() const;
    bool operator==(const custom_bitset& other) const;
    friend constexpr custom_bitset operator&(custom_bitset lhs, const custom_bitset& rhs);
    friend constexpr custom_bitset operator|(custom_bitset lhs, const custom_bitset& rhs);
    friend constexpr custom_bitset operator^(custom_bitset lhs, const custom_bitset& rhs);
    friend constexpr custom_bitset operator-(custom_bitset lhs, const custom_bitset& rhs);
    custom_bitset& operator&=(const custom_bitset& other);
    custom_bitset& operator|=(const custom_bitset& other);
    custom_bitset& operator^=(const custom_bitset &other);
    custom_bitset& operator-=(const custom_bitset& other);
    bool operator[](const reference& ref) const { return test(ref); };
    bool operator[](const size_type pos) const { return test(pos); };
    iterator operator[](const reference& ref) { return {this, ref}; };
    iterator operator[](const size_type pos) { return {this, pos}; };
    friend std::ostream& operator<<(std::ostream& stream, const custom_bitset& bb);
    explicit operator std::vector<size_type>() const;

    void set(const reference& ref);
    void set(const size_type pos) { set(reference(pos)); }
    void set(const reference& ref, bool value);
    void set(const size_type pos, const bool value) { set(reference(pos), value); }
    void reset(const reference& ref);
    void reset(const size_type pos) { reset(reference(pos)); }
    void flip(const reference& ref);
    void flip(const size_type pos) { flip(reference(pos)); }
    [[nodiscard]] bool test(const reference& ref) const;
    [[nodiscard]] bool test(const size_type pos) const { return test(reference(pos)); }

    [[nodiscard]] reference front() const;
    reference pop_front();

    [[nodiscard]] reference next(reference ref) const;
    [[nodiscard]] reference next(const size_type pos) const { return next(reference(pos)); }
    reference pop_next(reference ref);

    [[nodiscard]] reference back() const;
    reference pop_back();

    [[nodiscard]] reference prev(reference ref) const;
    [[nodiscard]] reference prev(const size_type pos) const { return prev(reference(pos)); }
    reference pop_prev(reference ref);

    [[nodiscard]] size_type degree() const { return count(); };

    void flip();
    void reset();

    [[nodiscard]] size_type size() const { return _size; }
    [[nodiscard]] size_type count() const;

    void swap(custom_bitset& other) noexcept;

    [[nodiscard]] bool all() const;
    [[nodiscard]] bool any() const;
    [[nodiscard]] bool none() const;

    void resize(size_type new_size);

    void clear_before(const reference& ref);
    void clear_before(size_type pos);
    void clear_until(const reference& ref);
    void clear_until(size_type pos);
    void clear_after(const reference& ref);
    void clear_after(size_type pos);
    void clear_from(const reference& ref);
    void clear_from(size_type pos);
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

inline custom_bitset::custom_bitset(const size_type size): _size(size), _bits(blocks_needed(size)) {}

// we set everything to 1 or to 0
inline custom_bitset::custom_bitset(const size_type size, const bool default_value): _size(size), _bits(blocks_needed(size), default_value*~block_type{0}) {
    // unset last part of last block (if there is one)
    if (get_block_bit(_size)) _bits.back() &= below_mask(get_block_bit(_size));
}

inline custom_bitset::custom_bitset(custom_bitset other, const size_type size): custom_bitset(std::move(other)) {
    this->resize(size);
}

inline custom_bitset::custom_bitset(const std::initializer_list<block_type> &v): _size(*std::ranges::max_element(v) + 1), _bits(blocks_needed(_size)) {
    for (const auto pos: v) {
        set(pos);
    }
}

inline custom_bitset::custom_bitset(const std::vector<size_type> &v, const size_type size): _size(size), _bits(blocks_needed(size)) {
    for (const auto pos: v) {
        set(pos);
    }
}

inline custom_bitset custom_bitset::before(custom_bitset src, const reference &ref) {
    src.clear_from(ref);
    return src;
}

inline custom_bitset custom_bitset::before(const custom_bitset &src, const size_type pos) {
    return before(src, reference(pos));
}

inline custom_bitset custom_bitset::until(custom_bitset src, const reference &ref) {
    src.clear_after(ref);
    return src;
}

inline custom_bitset custom_bitset::until(const custom_bitset &src, const size_type pos) {
    return until(src, reference(pos));
}

inline custom_bitset custom_bitset::after(custom_bitset src, const reference &ref) {
    src.clear_until(ref);
    return src;
}

inline custom_bitset custom_bitset::after(const custom_bitset &src, const size_type pos) {
    return after(src, reference(pos));
}

inline custom_bitset custom_bitset::from(custom_bitset src, const reference &ref) {
    src.clear_before(ref);
    return src;
}

inline custom_bitset custom_bitset::from(const custom_bitset &src, const size_type pos) {
    return from(src, reference(pos));
}

inline custom_bitset custom_bitset::complement(custom_bitset src) {
    src.flip();
    return src;
}

constexpr custom_bitset operator&(custom_bitset lhs, const custom_bitset& rhs) {
    return lhs &= rhs;
}

constexpr custom_bitset operator|(custom_bitset lhs, const custom_bitset& rhs) {
    return lhs |= rhs;
}

constexpr custom_bitset operator^(custom_bitset lhs, const custom_bitset& rhs) {
    return lhs ^= rhs;
}

constexpr custom_bitset operator-(custom_bitset lhs, const custom_bitset& rhs) {
    return lhs -= rhs;
}

inline custom_bitset custom_bitset::operator~() const {
    return custom_bitset::complement(*this);
}

inline bool custom_bitset::operator==(const custom_bitset &other) const {
    if (this == &other) return true;

    const auto M = std::min(_bits.size(), other._bits.size());

    for (size_type i = 0; i < M; i++)
        if (_bits[i] != other._bits[i]) return false;

    // if one is smaller, we check that the remaining bits are 0
    for (size_type i = M; i < _bits.size(); i++)
        if (_bits[i] != 0) return false;
    for (size_type i = M; i < other._bits.size(); ++i)
        if (other._bits[i] != 0) return false;

    return true;
}

inline custom_bitset& custom_bitset::operator&=(const custom_bitset &other) {
    std::ranges::transform(_bits, other._bits, _bits.begin(), std::bit_and<>{});

    return *this;
}

inline custom_bitset& custom_bitset::operator|=(const custom_bitset &other) {
    std::ranges::transform(_bits, other._bits, _bits.begin(), std::bit_or<>{});

    return *this;
}

inline custom_bitset& custom_bitset::operator^=(const custom_bitset &other) {
    const auto M = std::min(_bits.size(), other._bits.size());

    for (size_type i = 0; i < M; ++i)
        _bits[i] ^= other._bits[i];
    for (size_type i = M; i < _bits.size(); ++i)
        _bits[i] ^= 0;

    return *this;
}

inline custom_bitset& custom_bitset::operator-=(const custom_bitset &other) {
    const auto M = std::min(_bits.size(), other._bits.size());

    for (size_type i = 0; i < M; ++i)
        _bits[i] &= ~other._bits[i];

    return *this;
}

inline custom_bitset::operator std::vector<size_type>() const {
    std::vector<size_type> list;
    list.reserve(count());

    for (const auto v : *this) {
        list.push_back(v);
    }

    return list;
}

void custom_bitset::set(const reference &ref) {
    assert(ref < _size);
    [[assume(ref < _size)]];

    _bits[ref.block] |= mask_bit(ref.bit);
}

void custom_bitset::set(const reference &ref, const bool value) {
    if (value) set(ref);
    else reset(ref);
}

void custom_bitset::reset(const reference &ref) {
    assert(ref < _size);
    [[assume(ref < _size)]];

    _bits[ref.block] &= ~mask_bit(ref.bit);
}

inline void custom_bitset::reset() {
    std::ranges::fill(_bits, 0);
}

void custom_bitset::flip(const reference &ref) {
    if (test(ref)) reset(ref);
    else set(ref);
}

bool custom_bitset::test(const reference &ref) const {
    assert(ref < _size);
    [[assume(ref < _size)]];

    return _bits[ref.block] & mask_bit(ref.bit);
}

inline custom_bitset::reference custom_bitset::front() const {
    for (auto ref = reference(0, 0); ref.block < _bits.size(); ++ref.block) {
        if (_bits[ref.block] != 0) {
            ref.bit = bit_scan_forward(_bits[ref.block]);
            return ref;
        }
    }

    return reference(_size);
}

inline custom_bitset::reference custom_bitset::pop_front() {
    const auto ref = front();
    if (ref != size()) reset(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::next(reference ref) const {
    assert(ref < _size);
    [[assume(ref < _size)]];
    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by back+1
    const auto masked_number = _bits[ref.block] & after_mask(ref.bit);
    if (masked_number != 0) {
        ref.bit = bit_scan_forward(masked_number);
        return ref;
    }

    for (++ref.block; ref.block < _bits.size(); ++ref.block) {
        if (_bits[ref.block] != 0) {
            ref.bit = bit_scan_forward(_bits[ref.block]);
            return ref;
        }
    }

    return reference(_size);
}

inline custom_bitset::reference custom_bitset::pop_next(reference ref) {
    ref = next(ref);
    if (ref != size()) reset(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::back() const {
    for (reference ref(_size-1); ref.block-- > 0;) {
        if (_bits[ref.block] != 0) {
            ref.bit= bit_scan_reverse(_bits[ref.block]);
            return ref;
        }
    }

    return reference(_size);
}

inline custom_bitset::reference custom_bitset::pop_back() {
    const auto ref = back();
    if (ref != size()) reset(ref);
    return ref;
}

inline custom_bitset::reference custom_bitset::prev(reference ref) const {
    assert(ref < _size);
    [[assume(ref < _size)]];

    const auto masked_number = _bits[ref.block] & below_mask(ref.bit);
    if (masked_number != 0) {
        ref.bit = bit_scan_reverse(masked_number);
        return ref;
    }

    while (ref.block-- > 0) {
        if (_bits[ref.block] != 0) {
            ref.bit = bit_scan_reverse(_bits[ref.block]);
            return ref;
        }
    }

    return reference(_size);
}

inline custom_bitset::reference custom_bitset::pop_prev(reference ref) {
    ref = prev(ref);
    if (ref != size()) reset(ref);
    return ref;
}

inline void custom_bitset::flip() {
    std::ranges::transform(_bits.begin(), _bits.end(), _bits.begin(), std::bit_not<>{});
    _bits.back() &= below_mask(get_block_bit(_size));
}

inline custom_bitset::size_type custom_bitset::count() const {
    return std::accumulate(
        _bits.begin(), _bits.end(), 0,
        [](auto sum, auto word) { return sum + std::popcount(word); }
    );
}

inline void custom_bitset::swap(custom_bitset &other) noexcept {
    std::swap(_size, other._size);
    _bits.swap(other._bits);
}

inline void custom_bitset::resize(const size_type new_size) {
    if (_size == new_size) return;
    _size = new_size;
    _bits.resize(blocks_needed(_size));
    if (get_block_bit(_size)) _bits[get_last_block(_size)] &= below_mask(get_block_bit(_size));
}

inline void custom_bitset::clear_before(const reference& ref) {
    assert(ref < _size);
    [[assume(ref < _size)]];

    std::ranges::fill_n(_bits.begin(), static_cast<std::ptrdiff_t>(ref.block), 0);
    _bits[ref.block] &= from_mask(ref.bit);
}

inline void custom_bitset::clear_before(const size_type pos) {
    clear_before(reference(pos));
}

inline void custom_bitset::clear_until(const reference& ref) {
    clear_before(reference(ref + 1));
}

inline void custom_bitset::clear_until(const size_type pos) {
    clear_before(reference(pos + 1));
}

inline void custom_bitset::clear_after(const reference& ref) {
    clear_from(reference(ref + 1));
}

inline void custom_bitset::clear_after(const size_type pos) {
    clear_from(reference(pos + 1));
}

inline void custom_bitset::clear_from(const reference& ref) {
    assert(ref < _size);
    [[assume(ref < _size)]];

    _bits[ref.block] &= below_mask(ref.bit);
    std::ranges::fill(std::next(_bits.begin(), static_cast<std::ptrdiff_t>(ref.block) + 1), _bits.end(), 0);
}

inline void custom_bitset::clear_from(const size_type pos) {
    clear_from(reference(pos));
}

inline bool custom_bitset::all() const {
    const auto ref = reference(_size);
    if (ref.bit == 0) return std::ranges::all_of(_bits, [](const auto word) { return word == std::numeric_limits<block_type>::max(); });

    const bool all = std::ranges::all_of(_bits.begin(), _bits.end()-1, [](const auto word) { return word == std::numeric_limits<block_type>::max(); });
    if (all && ((_bits.back() | from_mask(ref.bit)) == std::numeric_limits<block_type>::max())) return true;
    return false;
}

inline bool custom_bitset::any() const {
    return std::ranges::any_of(_bits, [](const auto word) { return word != 0; });
}

inline bool custom_bitset::none() const {
    return std::ranges::none_of(_bits, [](const auto word) { return word != 0; });
}
