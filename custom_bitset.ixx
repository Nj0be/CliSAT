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
#include <memory>

export module custom_bitset;

import instructions;
import aligned_allocator_module;

// Primary concept for integer types
template<typename T>
concept Integer = std::integral<T> && !std::same_as<T, bool> && !std::same_as<T, char>;

// Concept for containers that hold integers
template<typename Container>
concept IntegerContainer = std::ranges::range<Container> &&
                          Integer<std::ranges::range_value_t<Container>>;

export class custom_bitset {
public:
    // 32 bit architecture will use uint32_t, 64 bit will use uint64_t
    typedef unsigned long long block_type;
    typedef std::size_t size_type;
    static constexpr size_t alignment = 32;

    class reference {
        size_type block;    // current block index
        size_type bit;            // bit position inside block

    public:
        constexpr reference(const size_type block, const size_type bit): block(block), bit(bit) {}
        constexpr explicit reference(const size_type pos) : block(get_block(pos)), bit(get_block_bit(pos)) {}

        inline size_type operator*() const { return block*block_size + bit; }
        inline operator size_type() const { return **this; };

        inline reference& operator++() { *this = reference(*this + 1); return *this; }
        inline reference operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        inline reference& operator--() { *this = reference(*this - 1); return *this; }
        inline reference operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        inline friend bool operator==(const reference& lhs, const reference& rhs) noexcept { return lhs.block == rhs.block && lhs.bit == rhs.bit; }
        inline friend bool operator< (const reference& lhs, const reference& rhs) noexcept {
            if (lhs.block != rhs.block) return lhs.block < rhs.block;
            return lhs.bit < rhs.bit;
        }

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

        inline explicit iterator() : bs(nullptr), ref(0, 0) {}
        inline iterator(custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        inline iterator(custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}

        inline reference operator*() const { return ref; }

        inline iterator& operator=(const bool value) {
            bs->set(ref, value);
            return *this;
        }

        inline explicit operator bool() const { return bs->test(ref); }

        inline iterator& operator++() { ref = bs->next(ref); return *this; }
        inline iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        inline iterator& operator--() { ref = bs->prev(ref); return *this; }
        inline iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        inline bool operator==(const iterator& other) const { return bs == other.bs && ref == other.ref; }

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

        inline explicit const_iterator() : bs(nullptr), ref(0, 0) {}
        inline const_iterator(const custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        inline const_iterator(const custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}
        inline explicit const_iterator(const iterator& it) : bs(it.bs), ref(it.ref) {}

        inline reference operator*() const { return ref; }

        inline explicit operator bool() const { return bs->test(ref); }

        inline const_iterator& operator++() { ref = bs->next(ref); return *this; }
        inline const_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        inline const_iterator& operator--() { ref = bs->prev(ref); return *this; }
        inline const_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        inline bool operator==(const const_iterator& other) const { return bs == other.bs && ref == other.ref; }

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

        inline explicit reverse_iterator() : bs(nullptr), ref(0, 0) {}
        inline reverse_iterator(custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        inline reverse_iterator(custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}

        inline reference operator*() const { return ref; }

        inline reverse_iterator& operator=(const bool value) {
            bs->set(ref, value);
            return *this;
        }

        inline explicit operator bool() const { return bs->test(ref); }

        inline reverse_iterator& operator--() { ref = bs->next(ref); return *this; }
        inline reverse_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        inline reverse_iterator& operator++() { ref = bs->prev(ref); return *this; }
        inline reverse_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        inline bool operator==(const reverse_iterator& other) const { return bs == other.bs && ref == other.ref; }

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

        inline explicit reverse_const_iterator() : bs(nullptr), ref(0, 0) {}
        inline reverse_const_iterator(const custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        inline reverse_const_iterator(const custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}
        inline explicit reverse_const_iterator(const iterator& it) : bs(it.bs), ref(it.ref) {}

        inline reference operator*() const { return ref; }

        inline explicit operator bool() const { return bs->test(ref); }

        inline reverse_const_iterator& operator++() { ref = bs->next(ref); return *this; }
        inline reverse_const_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        inline reverse_const_iterator& operator--() { ref = bs->prev(ref); return *this; }
        inline reverse_const_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        inline bool operator==(const reverse_const_iterator& other) const { return bs == other.bs && ref == other.ref; }

        friend class custom_bitset;
    };

    [[nodiscard]] inline iterator begin() { return {this, front()}; }
    [[nodiscard]] inline const_iterator begin() const { return {this, front()}; }
    [[nodiscard]] inline iterator end() { return {this, npos}; }
    [[nodiscard]] inline const_iterator end() const { return {this, npos}; }
    [[nodiscard]] inline reverse_iterator rbegin() { return {this, back()}; }
    [[nodiscard]] inline reverse_const_iterator rbegin() const { return {this, back()}; }
    [[nodiscard]] inline reverse_iterator rend() { return {this, npos}; }
    [[nodiscard]] inline reverse_const_iterator rend() const { return {this, npos}; }

private:
    static constexpr size_type block_size = std::numeric_limits<block_type>::digits;
    static constexpr size_type block_size_log2 = std::countr_zero(block_size);

    size_type _size;
    std::vector<block_type, aligned_allocator<block_type, alignment>> _bits;

    static constexpr size_type get_block(const size_type pos) noexcept { return pos >> block_size_log2; };
    static constexpr size_type get_block_bit(const size_type pos) noexcept { return pos & below_mask(block_size_log2); }

    // Used to allocate at least one block regardless of size
    static constexpr size_type blocks_needed(const size_type size) noexcept { return get_block(size + block_size-1) | (size == 0); }

    // it's necessary because even if size it's 0, we still have a block inside bits vector
    static constexpr size_type get_last_block(const size_type size) noexcept { return get_block(size + block_size-1) - (size != 0); }

    static constexpr block_type mask_bit(size_type bit) noexcept;
    static constexpr block_type below_mask(size_type bit) noexcept;
    static constexpr block_type until_mask(size_type bit) noexcept;
    static constexpr block_type from_mask(size_type bit) noexcept;
    static constexpr block_type after_mask(size_type bit) noexcept;

    template <bool pop>
    reference front_impl(this auto&& self) noexcept;

    template <bool pop>
    reference next_impl(this auto&& self, reference ref);

    template <bool pop>
    reference back_impl(this auto&& self) noexcept;

    template <bool pop>
    reference prev_impl(this auto&& self, reference ref);

public:
    inline custom_bitset() : custom_bitset(0) {}
    explicit custom_bitset(size_type size, bool default_value = false);
    custom_bitset(custom_bitset other, size_type size);

    template<IntegerContainer Container>
    explicit custom_bitset(const Container &container);

    template<IntegerContainer Container>
    explicit custom_bitset(const Container &container, size_type c_size);

    custom_bitset(const custom_bitset& other) = default;
    custom_bitset(custom_bitset&& other) noexcept = default;

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
    custom_bitset& operator=(const custom_bitset& other);

    friend custom_bitset operator&(custom_bitset lhs, const custom_bitset& rhs);
    friend custom_bitset operator|(custom_bitset lhs, const custom_bitset& rhs);
    friend custom_bitset operator^(custom_bitset lhs, const custom_bitset& rhs);
    friend custom_bitset operator-(custom_bitset lhs, const custom_bitset& rhs);

    custom_bitset& operator&=(const custom_bitset& other);
    custom_bitset& operator|=(const custom_bitset& other);
    custom_bitset& operator^=(const custom_bitset &other);
    custom_bitset& operator-=(const custom_bitset& other);

    static void AND(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest);
    static void AND(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, const reference& end);
    static void AND(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, size_type end_pos);
    static void AND(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, const reference& start, const reference& end);
    static void AND(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, size_type start_pos, size_type end_pos);
    static void OR(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest);
    static void OR(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, const reference& end);
    static void OR(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, size_type end_pos);
    static void OR(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, const reference& start, const reference& end);
    static void OR(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, size_type start_pos, size_type end_pos);
    static void SUB(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest);
    static void SUB(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, const reference& end);
    static void SUB(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, size_type end_pos);
    static void SUB(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, const reference& start, const reference& end);
    static void SUB(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest, size_type start_pos, size_type end_pos);

    inline bool operator[](const reference& ref) const { return test(ref); };
    inline bool operator[](const size_type pos) const { return test(pos); };
    inline iterator operator[](const reference& ref) { return {this, ref}; };
    inline iterator operator[](const size_type pos) { return {this, pos}; };
    friend std::ostream& operator<<(std::ostream& stream, const custom_bitset& bb);
    explicit operator std::vector<size_type>() const;

    void set(const reference& ref);
    inline void set(const size_type pos) { set(reference(pos)); }
    void set(const reference& ref, bool value);
    inline void set(const size_type pos, const bool value) { set(reference(pos), value); }
    void reset(const reference& ref);
    inline void reset(const size_type pos) { reset(reference(pos)); }
    void flip(const reference& ref);
    inline void flip(const size_type pos) { flip(reference(pos)); }
    [[nodiscard]] bool test(const reference& ref) const;
    [[nodiscard]] inline bool test(const size_type pos) const { return test(reference(pos)); }

    [[nodiscard]] inline reference front() const noexcept { return front_impl<false>(); }
    inline reference pop_front() noexcept { return front_impl<true>(); }

    [[nodiscard]] inline reference next(const reference& ref) const { return next_impl<false>(ref); }
    [[nodiscard]] inline reference next(const size_type pos) const { return next(reference(pos)); }
    inline reference pop_next(const reference& ref) { return next_impl<true>(ref); }

    [[nodiscard]] inline reference back() const noexcept { return back_impl<false>(); }
    inline reference pop_back() noexcept { return back_impl<true>(); }

    [[nodiscard]] inline reference prev(const reference& ref) const { return prev_impl<false>(ref); }
    [[nodiscard]] inline reference prev(const size_type pos) const { return prev(reference(pos)); }
    inline reference pop_prev(const reference& ref) { return prev_impl<true>(ref); }

    [[nodiscard]] inline size_type degree() const noexcept { return count(); };

    void flip() noexcept;
    void set() noexcept;
    void reset() noexcept;

    [[nodiscard]] inline size_type size() const noexcept { return _size; }
    [[nodiscard]] size_type count() const noexcept;

    void swap(custom_bitset& other) noexcept;

    [[nodiscard]] bool all() const noexcept;
    [[nodiscard]] bool any() const noexcept;
    [[nodiscard]] bool none() const noexcept;
    [[nodiscard]] bool intersects(const custom_bitset& other) const;
    [[nodiscard]] bool is_subset_of(const custom_bitset &other) const;
    [[nodiscard]] bool is_superset_of(const custom_bitset &other) const;
    [[nodiscard]] custom_bitset::reference front_difference(const custom_bitset &other) const;

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

constexpr custom_bitset::block_type custom_bitset::mask_bit(const size_type bit) noexcept {
    assert(bit < block_size);
    [[assume(bit < block_size)]];

    return block_type{1} << bit;
}

constexpr custom_bitset::block_type custom_bitset::below_mask(const size_type bit) noexcept {
    return ~from_mask(bit);
}

constexpr custom_bitset::block_type custom_bitset::until_mask(const size_type bit) noexcept {
    return ~after_mask(bit);
}

constexpr custom_bitset::block_type custom_bitset::from_mask(const size_type bit) noexcept {
    assert(bit < block_size);
    [[assume(bit < block_size)]];

    return ~block_type{0} << bit;
}

constexpr custom_bitset::block_type custom_bitset::after_mask(const size_type bit) noexcept {
    assert(bit < block_size);
    [[assume(bit < block_size)]];

    return ~block_type{1} << bit;
}

constexpr custom_bitset::reference custom_bitset::npos(std::numeric_limits<custom_bitset::size_type>::max(), 0);

template <>
struct std::formatter<custom_bitset::reference> : std::formatter<custom_bitset::size_type> {
    inline auto format(const custom_bitset::reference& id, std::format_context& ctx) const {
        return std::formatter<custom_bitset::size_type>::format(*id, ctx);
    }
};

inline std::ostream& operator<<(std::ostream &stream, const custom_bitset &bb) {
    stream << '[';
    std::size_t tot = 0;

    for (const auto v : bb) {
        stream << v << ' ';
        ++tot;
    }

    stream << '(' << tot << ")]";
    return stream;
}

// we set everything to 1 or to 0
inline custom_bitset::custom_bitset(const size_type size, const bool default_value)
        : _size(size),
          _bits(blocks_needed(size), default_value*std::numeric_limits<block_type>::max())
{
    // unset last part of last block (if there is one)
    if (get_block_bit(_size)) _bits.back() &= below_mask(get_block_bit(_size));
}

inline custom_bitset::custom_bitset(custom_bitset other, const size_type size): custom_bitset(std::move(other)) {
    this->resize(size);
}

template<IntegerContainer Container>
custom_bitset::custom_bitset(const Container &container)
        : _size(std::ranges::empty(container) ? 0 : static_cast<size_type>(*std::ranges::max_element(container)) + 1),
          _bits(blocks_needed(_size))
{
    for (const auto pos : container) set(static_cast<size_type>(pos));
}

template<IntegerContainer Container>
custom_bitset::custom_bitset(const Container &container, const size_type c_size): custom_bitset(c_size) {
    for (const auto pos : container) {
        if (pos < size()) set(static_cast<size_type>(pos));
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

inline custom_bitset operator&(custom_bitset lhs, const custom_bitset& rhs) {
    return lhs &= rhs;
}

inline custom_bitset operator|(custom_bitset lhs, const custom_bitset& rhs) {
    return lhs |= rhs;
}

inline custom_bitset operator^(custom_bitset lhs, const custom_bitset& rhs) {
    return lhs ^= rhs;
}

inline custom_bitset operator-(custom_bitset lhs, const custom_bitset& rhs) {
    return lhs -= rhs;
}

inline custom_bitset custom_bitset::operator~() const {
    return custom_bitset::complement(*this);
}

inline bool custom_bitset::operator==(const custom_bitset &other) const {
    if (this == &other) return true;
    if (_size != other._size) return false;

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); i++)
        if (a[i] != b[i]) return false;

    return true;
}

inline custom_bitset & custom_bitset::operator=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); ++i)
        a[i] = b[i];

    return *this;
}

inline custom_bitset& custom_bitset::operator&=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    // used for vectorization
    // TODO: clean that shit
    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); ++i)
        a[i] &= b[i];

    return *this;
}

inline void custom_bitset::AND(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest) {
    assert(lhs.size() == rhs.size());
    assert(rhs.size() == dest.size());
    [[assume(lhs.size() == rhs.size())]];
    [[assume(rhs.size() == dest.size())]];

    const auto a = std::assume_aligned<alignment>(lhs._bits.data());
    const auto b = std::assume_aligned<alignment>(rhs._bits.data());
    const auto dst = std::assume_aligned<alignment>(dest._bits.data());

    for (size_type i = 0; i < dest._bits.size(); ++i)
        dst[i] = a[i] & b[i];
}

inline void custom_bitset::AND(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest, const reference &end) {
    assert(lhs.size() == rhs.size());
    assert(rhs.size() == dest.size());
    assert(end <= dest.size());
    [[assume(lhs.size() == rhs.size())]];
    [[assume(rhs.size() == dest.size())]];
    [[assume(end <= dest.size())]];

    const auto a = std::assume_aligned<alignment>(lhs._bits.data());
    const auto b = std::assume_aligned<alignment>(rhs._bits.data());
    const auto dst = std::assume_aligned<alignment>(dest._bits.data());

    for (size_type i = 0; i < end.block; ++i)
        dst[i] = a[i] & b[i];

    dst[end.block] = (a[end.block] & b[end.block]) & below_mask(end.bit);

    for (size_type i = end.block+1; i < dest._bits.size(); ++i)
        dst[i] = 0;
}

inline void custom_bitset::AND(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest,
    const size_type end_pos) {
    AND(lhs, rhs, dest, reference(end_pos));
}

inline void custom_bitset::AND(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest, const reference &start, const reference &end) {
    assert(lhs.size() == rhs.size());
    assert(rhs.size() == dest.size());
    assert(start < dest.size());
    assert(end <= dest.size());
    [[assume(lhs.size() == rhs.size())]];
    [[assume(rhs.size() == dest.size())]];
    [[assume(start < dest.size())]];
    [[assume(end <= dest.size())]];

    const auto a = std::assume_aligned<alignment>(lhs._bits.data());
    const auto b = std::assume_aligned<alignment>(rhs._bits.data());
    const auto dst = std::assume_aligned<alignment>(dest._bits.data());

    for (size_type i = 0; i < start.block; ++i)
        dst[i] = 0;

    dst[start.block] = (a[start.block] & b[end.block]) & from_mask(start.bit);

    for (size_type i = start.block+1; i < end.block; ++i)
        dst[i] = a[i] & b[i];

    dst[end.block] = (a[end.block] & b[end.block]) & below_mask(end.bit);

    for (size_type i = end.block+1; i < dest._bits.size(); ++i)
        dst[i] = 0;
}

inline void custom_bitset::AND(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest,
    const size_type start_pos, const size_type end_pos) {
    AND(lhs, rhs, dest, reference(start_pos), reference(end_pos));
}

inline void custom_bitset::OR(const custom_bitset &lhs, const custom_bitset& rhs, custom_bitset& dest) {
    assert(lhs.size() == rhs.size());
    assert(rhs.size() == dest.size());
    [[assume(lhs.size() == rhs.size())]];
    [[assume(rhs.size() == dest.size())]];

    const auto a = std::assume_aligned<alignment>(lhs._bits.data());
    const auto b = std::assume_aligned<alignment>(rhs._bits.data());
    const auto dst = std::assume_aligned<alignment>(dest._bits.data());

    for (size_type i = 0; i < dest._bits.size(); ++i)
        dst[i] = a[i] | b[i];
}

inline void custom_bitset::OR(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest, const reference &end) {
    assert(lhs.size() == rhs.size());
    assert(rhs.size() == dest.size());
    assert(end <= dest.size());
    [[assume(lhs.size() == rhs.size())]];
    [[assume(rhs.size() == dest.size())]];
    [[assume(end <= dest.size())]];

    const auto a = std::assume_aligned<alignment>(lhs._bits.data());
    const auto b = std::assume_aligned<alignment>(rhs._bits.data());
    const auto dst = std::assume_aligned<alignment>(dest._bits.data());

    for (size_type i = 0; i < end.block; ++i)
        dst[i] = a[i] | b[i];

    dst[end.block] = (a[end.block] | b[end.block]) & below_mask(end.bit);

    for (size_type i = end.block+1; i < dest._bits.size(); ++i)
        dst[i] = a[i];
}

inline void custom_bitset::OR(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest,
    const size_type end_pos) {
    AND(lhs, rhs, dest, reference(end_pos));
}

inline void custom_bitset::OR(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest, const reference &start, const reference &end) {
    assert(lhs.size() == rhs.size());
    assert(rhs.size() == dest.size());
    assert(start < dest.size());
    assert(end <= dest.size());
    [[assume(lhs.size() == rhs.size())]];
    [[assume(rhs.size() == dest.size())]];
    [[assume(start < dest.size())]];
    [[assume(end <= dest.size())]];

    const auto a = std::assume_aligned<alignment>(lhs._bits.data());
    const auto b = std::assume_aligned<alignment>(rhs._bits.data());
    const auto dst = std::assume_aligned<alignment>(dest._bits.data());

    for (size_type i = 0; i < start.block; ++i)
        dst[i] = a[i];

    dst[start.block] = (a[start.block] | b[end.block]) & from_mask(start.bit);

    for (size_type i = start.block+1; i < end.block; ++i)
        dst[i] = a[i] | b[i];

    dst[end.block] = (a[end.block] | b[end.block]) & below_mask(end.bit);

    for (size_type i = end.block+1; i < dest._bits.size(); ++i)
        dst[i] = a[i];
}

inline void custom_bitset::OR(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest,
    const size_type start_pos, const size_type end_pos) {
    AND(lhs, rhs, dest, reference(start_pos), reference(end_pos));
}

inline void custom_bitset::SUB(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest) {
    assert(lhs.size() == rhs.size());
    assert(rhs.size() == dest.size());
    [[assume(lhs.size() == rhs.size())]];
    [[assume(rhs.size() == dest.size())]];

    const auto a = std::assume_aligned<alignment>(lhs._bits.data());
    const auto b = std::assume_aligned<alignment>(rhs._bits.data());
    const auto dst = std::assume_aligned<alignment>(dest._bits.data());

    for (size_type i = 0; i < dest._bits.size(); ++i)
        dst[i] = a[i] & ~b[i];
}

inline void custom_bitset::SUB(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest, const reference &end) {
    assert(lhs.size() == rhs.size());
    assert(rhs.size() == dest.size());
    assert(end <= dest.size());
    [[assume(lhs.size() == rhs.size())]];
    [[assume(rhs.size() == dest.size())]];
    [[assume(end <= dest.size())]];

    const auto a = std::assume_aligned<alignment>(lhs._bits.data());
    const auto b = std::assume_aligned<alignment>(rhs._bits.data());
    const auto dst = std::assume_aligned<alignment>(dest._bits.data());

    for (size_type i = 0; i < end.block; ++i)
        dst[i] = a[i] & ~b[i];

    dst[end.block] = (a[end.block] & ~b[end.block]) & below_mask(end.bit);

    for (size_type i = end.block+1; i < dest._bits.size(); ++i)
        dst[i] = 0;
}

inline void custom_bitset::SUB(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest, const size_type end_pos) {
    SUB(lhs, rhs, dest, reference(end_pos));
}

inline void custom_bitset::SUB(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest, const reference &start, const reference &end) {
    assert(lhs.size() == rhs.size());
    assert(rhs.size() == dest.size());
    assert(start < dest.size());
    assert(end <= dest.size());
    [[assume(lhs.size() == rhs.size())]];
    [[assume(rhs.size() == dest.size())]];
    [[assume(start < dest.size())]];
    [[assume(end <= dest.size())]];

    const auto a = std::assume_aligned<alignment>(lhs._bits.data());
    const auto b = std::assume_aligned<alignment>(rhs._bits.data());
    const auto dst = std::assume_aligned<alignment>(dest._bits.data());

    for (size_type i = 0; i < start.block; ++i)
        dst[i] = 0;

    dst[start.block] = (a[start.block] & ~b[end.block]) & from_mask(start.bit);

    for (size_type i = start.block+1; i < end.block; ++i)
        dst[i] = a[i] & ~b[i];

    dst[end.block] = (a[end.block] & ~b[end.block]) & below_mask(end.bit);

    for (size_type i = end.block+1; i < dest._bits.size(); ++i)
        dst[i] = 0;
}

inline void custom_bitset::SUB(const custom_bitset &lhs, const custom_bitset &rhs, custom_bitset &dest,
    const size_type start_pos, const size_type end_pos) {
    SUB(lhs, rhs, dest, reference(start_pos), reference(end_pos));
}

inline custom_bitset& custom_bitset::operator|=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); ++i)
        a[i] |= b[i];

    return *this;
}

inline custom_bitset& custom_bitset::operator^=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); ++i)
        a[i] ^= b[i];

    return *this;
}

inline custom_bitset& custom_bitset::operator-=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_t i = 0; i < _bits.size(); ++i)
        a[i] &= ~b[i];

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

inline void custom_bitset::set(const reference &ref) {
    assert(ref < _size);
    [[assume(ref < _size)]];

    _bits[ref.block] |= mask_bit(ref.bit);
}

inline void custom_bitset::set(const reference &ref, const bool value) {
    if (value) set(ref);
    else reset(ref);
}

inline void custom_bitset::reset(const reference &ref) {
    assert(ref < _size);
    [[assume(ref < _size)]];

    _bits[ref.block] &= ~mask_bit(ref.bit);
}

inline void custom_bitset::set() noexcept {
    const auto a = std::assume_aligned<alignment>(_bits.data());

    for (size_type i = 0; i < _bits.size(); i++)
        a[i] = std::numeric_limits<block_type>::max();
}

inline void custom_bitset::reset() noexcept {
    const auto a = std::assume_aligned<alignment>(_bits.data());

    for (size_type i = 0; i < _bits.size(); i++)
        a[i] = 0;
}

inline void custom_bitset::flip(const reference &ref) {
    assert(ref < _size);
    [[assume(ref < _size)]];

    _bits[ref.block] ^= mask_bit(ref.bit);
}

inline bool custom_bitset::test(const reference &ref) const {
    assert(ref < _size);
    [[assume(ref < _size)]];

    return _bits[ref.block] & mask_bit(ref.bit);
}

template <bool pop>
custom_bitset::reference custom_bitset::front_impl(this auto&& self) noexcept {
    for (auto ref = reference{0, 0}; ref.block < self._bits.size(); ++ref.block) {
        if (self._bits[ref.block] != 0) {
            ref.bit = instructions::bit_scan_forward(self._bits[ref.block]);
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
        ref.bit = instructions::bit_scan_forward(masked_number);
        if constexpr (pop) self.reset(ref);
        return ref;
    }

    for (++ref.block; ref.block < self._bits.size(); ++ref.block) {
        if (self._bits[ref.block] != 0) {
            ref.bit = instructions::bit_scan_forward(self._bits[ref.block]);
            if constexpr (pop) self.reset(ref);
            return ref;
        }
    }

    return npos;
}

template <bool pop>
custom_bitset::reference custom_bitset::back_impl(this auto&& self) noexcept {
    reference ref(self._size-1);
    do {
        if (self._bits[ref.block] != 0) {
            ref.bit = instructions::bit_scan_reverse(self._bits[ref.block]);
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
        ref.bit = instructions::bit_scan_reverse(masked_number);
        if constexpr (pop) self.reset(ref);
        return ref;
    }

    while (ref.block-- > 0) {
        if (self._bits[ref.block] != 0) {
            ref.bit = instructions::bit_scan_reverse(self._bits[ref.block]);
            if constexpr (pop) self.reset(ref);
            return ref;
        }
    }

    return npos;
}

inline void custom_bitset::flip() noexcept {
    std::ranges::transform(_bits.begin(), _bits.end(), _bits.begin(), std::bit_not<>{});
    _bits.back() &= below_mask(get_block_bit(_size));
}

custom_bitset::size_type custom_bitset::count() const noexcept {
    size_type sum = 0;

    const auto a = std::assume_aligned<alignment>(_bits.data());

    for (size_type i = 0; i < _bits.size(); ++i)
        sum += instructions::popcount(a[i]);

    return sum;
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

inline bool custom_bitset::all() const noexcept {
    const auto ref = reference(_size);
    if (ref.bit == 0) return std::ranges::all_of(_bits, [](const auto word) { return word == std::numeric_limits<block_type>::max(); });

    const bool all = std::ranges::all_of(_bits.begin(), _bits.end()-1, [](const auto word) { return word == std::numeric_limits<block_type>::max(); });
    if (all && ((_bits.back() | from_mask(ref.bit)) == std::numeric_limits<block_type>::max())) return true;
    return false;
}

inline bool custom_bitset::any() const noexcept {
    const auto a = std::assume_aligned<alignment>(_bits.data());

    for (size_type i = 0; i < _bits.size(); i++)
        if (a[i]) return true;

    return false;
}

inline bool custom_bitset::none() const noexcept {
    const auto a = std::assume_aligned<alignment>(_bits.data());

    for (size_type i = 0; i < _bits.size(); i++)
        if (a[i]) return false;

    return true;
}

inline bool custom_bitset::intersects(const custom_bitset &other) const {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); i++)
        if (a[i] & b[i]) return true;

    return false;
}

inline bool custom_bitset::is_subset_of(const custom_bitset &other) const {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); i++)
        if (a[i] & ~b[i]) return false;

    return true;
}

inline bool custom_bitset::is_superset_of(const custom_bitset &other) const {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); i++)
        if (b[i] & ~a[i]) return false;

    return true;
}


inline custom_bitset::reference custom_bitset::front_difference(const custom_bitset &other) const {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); i++)
        if (a[i] & ~b[i]) {
            auto bit = instructions::bit_scan_forward(a[i] & ~b[i]);
            return {i, bit};
        }

    return npos;
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
        inline explicit reverse_view(custom_bitset& base) : base_(&base) {}

        // Use the container's actual rbegin/rend instead of make_reverse_iterator
        /*
        auto begin() { return std::make_reverse_iterator(base_->rbegin()); }
        auto end() { return std::make_reverse_iterator(base_->rend()); }
        auto begin() const { return std::make_reverse_iterator(base_->rbegin()); }
        auto end() const { return std::make_reverse_iterator(base_->rend()); }
    */
        inline auto begin() { return base_->rbegin(); }
        inline auto end() { return base_->rend(); }
        [[nodiscard]] inline auto begin() const { return base_->rbegin(); }
        [[nodiscard]] inline auto end() const { return base_->rend(); }
    };
}
