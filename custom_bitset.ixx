//
// Created by Beniamino Vagnarelli on 01/04/25.
//

module;

#include <algorithm>
#include <bitset>
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

    class reference {
        size_type block;    // current block index
        size_type bit;            // bit position inside block

    public:
        constexpr reference(const size_type block, const size_type bit): block(block), bit(bit) {}
        explicit reference(const size_type pos) : block(get_block(pos)), bit(get_block_bit(pos)) {}

        bool operator==(const reference& other) const { return block == other.block && bit == other.bit; }

        size_type operator*() const { return block*block_size + bit; }
        operator size_type() const { return **this; };

        // prefix increment
        reference& operator++() { *this = reference(*this + 1); return *this; }
        // postfix increment
        reference operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        // prefix decrement
        reference& operator--() { *this = reference(*this - 1); return *this; }
        // postfix decrement
        reference operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        friend class custom_bitset;
    };

    static const reference npos;

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


    class reverse_iterator {
        custom_bitset* bs;
        reference ref;

    public:
        using value_type = reference;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;

        consteval explicit reverse_iterator() : bs(nullptr), ref(0, 0) {}
        reverse_iterator(custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        reverse_iterator(custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}

        reference operator*() const { return ref; }

        reverse_iterator& operator=(const bool value) {
            bs->set(value);
            return *this;
        }

        // Conversion to bool for reading
        explicit operator bool() const { return bs->test(ref); }

        // prefix increment
        reverse_iterator& operator--() { ref = bs->next(ref); return *this; }
        // postfix increment
        reverse_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        // prefix decrement
        reverse_iterator& operator++() { ref = bs->prev(ref); return *this; }
        // postfix decrement
        reverse_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        bool operator==(const reverse_iterator& other) const { return bs == other.bs && ref == other.ref; }
        //bool operator!=(const iterator& other) const { return !(*this == other); }

        friend class custom_bitset;
    };

    class reverse_const_iterator {
        const custom_bitset* bs;
        reference ref;

    public:
        using value_type = reference;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;

        consteval explicit reverse_const_iterator() : bs(nullptr), ref(0, 0) {}
        reverse_const_iterator(const custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        reverse_const_iterator(const custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}
        explicit reverse_const_iterator(const iterator& it) : bs(it.bs), ref(it.ref) {}

        reference operator*() const { return ref; }

        explicit operator bool() const {
            return bs->test(ref);
        }

        // prefix increment
        reverse_const_iterator& operator++() { ref = bs->next(ref); return *this; }
        // postfix increment
        reverse_const_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        // prefix decrement
        reverse_const_iterator& operator--() { ref = bs->prev(ref); return *this; }
        // postfix decrement
        reverse_const_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        bool operator==(const reverse_const_iterator& other) const { return bs == other.bs && ref == other.ref; }
        //bool operator!=(const const_iterator& other) const { return !(*this == other); }

        friend class custom_bitset;
    };

    [[nodiscard]] iterator begin() { return {this, front()}; }
    [[nodiscard]] const_iterator begin() const { return {this, front()}; }
    [[nodiscard]] iterator end() { return {this, npos}; }
    [[nodiscard]] const_iterator end() const { return {this, npos}; }
    [[nodiscard]] reverse_iterator rbegin() { return {this, back()}; }
    [[nodiscard]] reverse_const_iterator rbegin() const { return {this, back()}; }
    [[nodiscard]] reverse_iterator rend() { return {this, npos}; }
    [[nodiscard]] reverse_const_iterator rend() const { return {this, npos}; }

private:
    static constexpr size_type block_size = std::numeric_limits<block_type>::digits;
    static constexpr size_type block_size_log2 = std::countr_zero(block_size);

    size_type _size;
    std::vector<block_type> _bits;

    static size_type bit_scan_forward(const block_type x) { return std::countr_zero(x); }
    static size_type bit_scan_reverse(const block_type x) { return block_size-1 - std::countl_zero(x); }

    //static size_type get_block(const size_type pos) { return pos/block_size; };
    static size_type get_block(const size_type pos) { return pos >> block_size_log2; };
    //static size_type get_block_bit(const size_type pos) { return pos%block_size; }
    static size_type get_block_bit(const size_type pos) { return pos & below_mask(block_size_log2); }

    // Used to allocate at least one block regardless of size
    static size_type blocks_needed(const size_type size) { return get_block(size + block_size-1) | (size == 0); }

    // it's necessary because even if size it's 0, we still have a block inside bits vector
    static size_type get_last_block(const size_type size) { return get_block(size + block_size-1) - (size != 0); }

    static block_type mask_bit(const size_type bit) { return block_type{1} << bit; }
    static block_type below_mask(const size_type bit) { return ~from_mask(bit); }
    static block_type until_mask(const size_type bit) { return ~after_mask(bit); }
    static block_type from_mask(const size_type bit) { return ~block_type{0} << bit; }
    static block_type after_mask(const size_type bit) { return ~block_type{1} << bit; }

    template <bool pop>
    reference front_impl(this auto&& self);

    template <bool pop>
    reference next_impl(this auto&& self, reference ref);

    template <bool pop>
    reference back_impl(this auto&& self);

    template <bool pop>
    reference prev_impl(this auto&& self, reference ref);

public:
    custom_bitset() : custom_bitset(0) {}
    explicit custom_bitset(size_type size);
    custom_bitset(size_type size, bool default_value);
    custom_bitset(custom_bitset other, size_type size);
    custom_bitset(const std::initializer_list<block_type>& v);
    custom_bitset(const std::vector<size_type>& v, size_type size);
    custom_bitset(const custom_bitset& other) : _size(other._size), _bits(other._bits) {}

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
    custom_bitset& operator=(const custom_bitset& other) {
        assert(size() == other.size());
        [[assume(size() == other.size())]];

        for (size_type i = 0; i < _bits.size(); ++i)
            _bits[i] = other._bits[i];

        return *this;
    }
    friend constexpr custom_bitset operator&(custom_bitset lhs, const custom_bitset& rhs);
    friend constexpr custom_bitset operator|(custom_bitset lhs, const custom_bitset& rhs);
    friend constexpr custom_bitset operator^(custom_bitset lhs, const custom_bitset& rhs);
    friend constexpr custom_bitset operator-(custom_bitset lhs, const custom_bitset& rhs);
    static bool calculate_subproblem(const custom_bitset& V, const custom_bitset& B, const custom_bitset& neigh_set, const reference& bi, custom_bitset& res) {
        bool non_empty = false;
        std::size_t block = 0;
        for (; block < bi.block; ++block) {
            res._bits[block] = ((V._bits[block] - B._bits[block]) & neigh_set._bits[block]) | (neigh_set._bits[block] & V._bits[block]);
            non_empty |= (res._bits[block] != 0);
        }
        res._bits[block] = ((V._bits[block] - B._bits[block]) & neigh_set._bits[block]) | ((neigh_set._bits[block] & below_mask(bi.bit)) & V._bits[block]);
        non_empty |= (res._bits[block] != 0);
        for (++block; block < res._bits.size(); ++block) {
            res._bits[block] = ((V._bits[block] - B._bits[block]) & neigh_set._bits[block]);
            non_empty |= (res._bits[block] != 0);
        }

        return non_empty;
    }
    static bool calculate_subproblem2(const custom_bitset& V, const custom_bitset& P, const custom_bitset& neigh_set, const reference& bi, custom_bitset& res) {
        bool non_empty = false;
        std::size_t block = 0;
        for (; block < bi.block; ++block) {
            res._bits[block] = (P._bits[block] & neigh_set._bits[block]) | (neigh_set._bits[block] & V._bits[block]);
            non_empty |= (res._bits[block] != 0);
        }
        res._bits[block] = (P._bits[block] & neigh_set._bits[block]) | ((neigh_set._bits[block] & below_mask(bi.bit)) & V._bits[block]);
        non_empty |= (res._bits[block] != 0);
        for (++block; block < res._bits.size(); ++block) {
            res._bits[block] = (P._bits[block] & neigh_set._bits[block]);
            non_empty |= (res._bits[block] != 0);
        }

        return non_empty;
    }
    static bool calculate_subproblem3(const custom_bitset& V, const custom_bitset& B, const custom_bitset& neigh_set, const reference& bi, custom_bitset& res) {
        bool non_empty = false;
        std::size_t block = 0;
        for (; block < bi.block; ++block) {
            res._bits[block] = V._bits[block] & neigh_set._bits[block];
            non_empty |= (res._bits[block] != 0);
        }
        res._bits[block] = (V._bits[block] - (B._bits[block] & after_mask(bi.bit))) & neigh_set._bits[block];
        non_empty |= (res._bits[block] != 0);
        for (++block; block < res._bits.size(); ++block) {
            res._bits[block] = (V._bits[block] - B._bits[block]) & neigh_set._bits[block];
            non_empty |= (res._bits[block] != 0);
        }

        return non_empty;
    }
    static void get_prev_neighbor_set(const custom_bitset& neigh_set, const custom_bitset& V, const reference& bi, custom_bitset& res) {
        std::size_t block = 0;
        for (; block < bi.block; ++block) {
            res._bits[block] = neigh_set._bits[block] & V._bits[block];
        }
        res._bits[block] = (neigh_set._bits[block] & below_mask(bi.bit)) & V._bits[block];
        for (++block; block < res._bits.size(); ++block) {
            res._bits[block] = 0;
        }
    }
    static void get_prev_neighbor_set(const custom_bitset& neigh_set, const reference& bi, custom_bitset& res) {
        std::size_t block = 0;
        for (; block < bi.block; ++block) {
            res._bits[block] = neigh_set._bits[block];
        }
        res._bits[block] = (neigh_set._bits[block] & below_mask(bi.bit));
        for (++block; block < res._bits.size(); ++block) {
            res._bits[block] = 0;
        }
    }
    static void SUB(const custom_bitset& lhs, const custom_bitset& rhs, custom_bitset& res) {
        for (size_type i = 0; i < lhs._bits.size(); ++i)
            res._bits[i] = lhs._bits[i] & ~rhs._bits[i];
    }
    custom_bitset& operator&=(const custom_bitset& other);
    custom_bitset& operator|=(const custom_bitset& other);
    custom_bitset& operator^=(const custom_bitset &other);
    custom_bitset& operator-=(const custom_bitset& other);
    static void AND(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest);
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

    [[nodiscard]] reference front() const { return front_impl<false>(); }
    reference pop_front() { return front_impl<true>(); }

    [[nodiscard]] reference next(const reference& ref) const { return next_impl<false>(ref); }
    [[nodiscard]] reference next(const size_type pos) const { return next(reference(pos)); }
    reference pop_next(const reference& ref) { return next_impl<true>(ref); }

    [[nodiscard]] reference back() const { return back_impl<false>(); }
    reference pop_back() { return back_impl<true>(); }

    [[nodiscard]] reference prev(const reference& ref) const { return prev_impl<false>(ref); }
    [[nodiscard]] reference prev(const size_type pos) const { return prev(reference(pos)); }
    reference pop_prev(const reference& ref) { return prev_impl<true>(ref); }

    [[nodiscard]] size_type degree() const { return count(); };

    void flip();
    void reset();

    [[nodiscard]] size_type size() const { return _size; }
    [[nodiscard]] size_type count() const;

    void swap(custom_bitset& other) noexcept;

    [[nodiscard]] bool all() const;
    [[nodiscard]] bool any() const;
    [[nodiscard]] bool none() const;
    [[nodiscard]] bool intersects(const custom_bitset& other) const;
    [[nodiscard]] bool is_subset_of(const custom_bitset &other) const;
    [[nodiscard]] bool is_superset_of(const custom_bitset &other) const;

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

constexpr custom_bitset::reference custom_bitset::npos(std::numeric_limits<custom_bitset::block_type>::max(), 0);

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
    if (_size != other._size) return false;

    for (size_type i = 0; i < _bits.size(); i++)
        if (_bits[i] != other._bits[i]) return false;

    return true;
}

inline custom_bitset& custom_bitset::operator&=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    for (size_type i = 0; i < _bits.size(); ++i)
        _bits[i] &= other._bits[i];

    return *this;
}

void custom_bitset::AND(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest) {
    const auto M = std::min(lhs._bits.size(), rhs._bits.size());

    for (size_type i = 0; i < M; ++i)
        dest._bits[i] = lhs._bits[i] & rhs._bits[i];
    for (size_type i = M; i < dest._bits.size(); ++i)
        dest._bits[i] = 0;
}


inline custom_bitset& custom_bitset::operator|=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    for (size_type i = 0; i < _bits.size(); ++i)
        _bits[i] |= other._bits[i];

    return *this;
}

inline custom_bitset& custom_bitset::operator^=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    for (size_type i = 0; i < _bits.size(); ++i)
        _bits[i] ^= other._bits[i];

    return *this;
}

inline custom_bitset& custom_bitset::operator-=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    /*
    size_type i = 0;
    for (; i+3 < _bits.size(); i+=4) {
        _bits[i] &= ~other._bits[i];
        _bits[i+1] &= ~other._bits[i+1];
        _bits[i+2] &= ~other._bits[i+2];
        _bits[i+3] &= ~other._bits[i+3];
    }
    */
    for (size_type i = 0; i < _bits.size(); i++)
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

template <bool pop>
custom_bitset::reference custom_bitset::front_impl(this auto&& self) {
    for (auto ref = reference{0, 0}; ref.block < self._bits.size(); ++ref.block) {
        if (self._bits[ref.block] != 0) {
            ref.bit = bit_scan_forward(self._bits[ref.block]);
            if constexpr (pop) self.reset(ref);
            return ref;
        }
    }
    return npos;
}

template <bool pop>
custom_bitset::reference custom_bitset::next_impl(this auto&& self, reference ref) {
    assert(ref < self._size);
    [[assume(ref < self._size)]];

    // shift by 64 doesn't work!! undefined behaviour
    // ~1ULL is all 1's except for the lowest one, aka already shifted by one
    // the resulting shift is all 1's shifted by back+1
    const auto masked_number = self._bits[ref.block] & after_mask(ref.bit);
    if (masked_number != 0) {
        ref.bit = bit_scan_forward(masked_number);
        if constexpr (pop) self.reset(ref);
        return ref;
    }

    for (++ref.block; ref.block < self._bits.size(); ++ref.block) {
        if (self._bits[ref.block] != 0) {
            ref.bit = bit_scan_forward(self._bits[ref.block]);
            if constexpr (pop) self.reset(ref);
            return ref;
        }
    }

    return npos;
}

template <bool pop>
custom_bitset::reference custom_bitset::back_impl(this auto&& self) {
    reference ref(self._size-1);
    do {
        if (self._bits[ref.block] != 0) {
            ref.bit= bit_scan_reverse(self._bits[ref.block]);
            if constexpr (pop) self.reset(ref);
            return ref;
        }
    } while (ref.block-- > 0);

    return npos;
}

template <bool pop>
custom_bitset::reference custom_bitset::prev_impl(this auto&& self, reference ref) {
    assert(ref < self._size);
    [[assume(ref < self._size)]];

    const auto masked_number = self._bits[ref.block] & below_mask(ref.bit);
    if (masked_number != 0) {
        ref.bit = bit_scan_reverse(masked_number);
        if constexpr (pop) self.reset(ref);
        return ref;
    }

    while (ref.block-- > 0) {
        if (self._bits[ref.block] != 0) {
            ref.bit = bit_scan_reverse(self._bits[ref.block]);
            if constexpr (pop) self.reset(ref);
            return ref;
        }
    }

    return npos;
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

bool custom_bitset::intersects(const custom_bitset &other) const {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    for (size_type i = 0; i < _bits.size(); i++)
        if (_bits[i] & other._bits[i]) return true;

    return false;
}

bool custom_bitset::is_subset_of(const custom_bitset &other) const {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    for (size_type i = 0; i < _bits.size(); i++)
        if (_bits[i] & ~other._bits[i]) return false;

    return true;
}

bool custom_bitset::is_superset_of(const custom_bitset &other) const {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    for (size_type i = 0; i < _bits.size(); i++)
        if (other._bits[i] & ~_bits[i]) return false;

    return true;
}

// Needed to allow std::views::reverse and std::ranges::reverse_view compatibility
template<>
inline constexpr bool std::ranges::enable_view<custom_bitset> = true;// Specialize view concept for your type

namespace std::ranges {
    template<>
    class reverse_view<custom_bitset> {
    private:
        custom_bitset* base_;

    public:
        explicit reverse_view(custom_bitset& base) : base_(&base) {}

        // Use the container's actual rbegin/rend instead of make_reverse_iterator
        /*
        auto begin() { return std::make_reverse_iterator(base_->rbegin()); }
        auto end() { return std::make_reverse_iterator(base_->rend()); }
        auto begin() const { return std::make_reverse_iterator(base_->rbegin()); }
        auto end() const { return std::make_reverse_iterator(base_->rend()); }
    */
        auto begin() { return base_->rbegin(); }
        auto end() { return base_->rend(); }
        auto begin() const { return base_->rbegin(); }
        auto end() const { return base_->rend(); }
    };
}