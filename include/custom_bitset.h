//
// Created by Beniamino Vagnarelli on 01/04/25.
//
#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <ranges>
#include <vector>
#include <print>
#include <ostream>

#include "aligned_allocator.h"
#include "instructions.h"

// Primary concept for integer types
template<typename T>
concept Integer = std::integral<T> && !std::same_as<T, bool> && !std::same_as<T, char>;

// Concept for containers that hold integers
template<typename Container>
concept IntegerContainer = std::ranges::range<Container> && Integer<std::ranges::range_value_t<Container>>;

class custom_bitset {
public:
    typedef std::uint64_t block_type;
    typedef std::size_t bit_type;
    typedef std::size_t size_type;

    class reference {
        size_type block;    // current block index
        bit_type bit;            // bit position inside block

    public:
        constexpr reference(const size_type block, const bit_type bit): block(block), bit(bit) {}
        constexpr explicit reference(const size_type pos) : block(get_block(pos)), bit(get_block_bit(pos)) {}

        size_type operator*() const { return block*block_size + bit; }
        operator size_type() const { return **this; };

        reference& operator++() { *this = reference(*this + 1); return *this; }
        reference operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        reference& operator--() { *this = reference(*this - 1); return *this; }
        reference operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        friend bool operator==(const reference& lhs, const reference& rhs) noexcept { return lhs.block == rhs.block && lhs.bit == rhs.bit; }
        friend bool operator< (const reference& lhs, const reference& rhs) noexcept {
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

        explicit iterator() : bs(nullptr), ref(0, 0) {}
        iterator(custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        iterator(custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}

        reference operator*() const { return ref; }

        iterator& operator=(const bool value) {
            bs->set(ref, value);
            return *this;
        }

        operator bool() const { return bs->test(ref); }

        iterator& operator++() { ref = bs->next(ref); return *this; }
        iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        iterator& operator--() { ref = bs->prev(ref); return *this; }
        iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        bool operator==(const iterator& other) const { return bs == other.bs && ref == other.ref; }

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

        explicit const_iterator() : bs(nullptr), ref(0, 0) {}
        const_iterator(const custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        const_iterator(const custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}
        explicit const_iterator(const iterator& it) : bs(it.bs), ref(it.ref) {}

        reference operator*() const { return ref; }

        operator bool() const { return bs->test(ref); }

        const_iterator& operator++() { ref = bs->next(ref); return *this; }
        const_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        const_iterator& operator--() { ref = bs->prev(ref); return *this; }
        const_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        bool operator==(const const_iterator& other) const { return bs == other.bs && ref == other.ref; }

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

        explicit reverse_iterator() : bs(nullptr), ref(0, 0) {}
        reverse_iterator(custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        reverse_iterator(custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}

        reference operator*() const { return ref; }

        reverse_iterator& operator=(const bool value) {
            bs->set(ref, value);
            return *this;
        }

        operator bool() const { return bs->test(ref); }

        reverse_iterator& operator--() { ref = bs->next(ref); return *this; }
        reverse_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        reverse_iterator& operator++() { ref = bs->prev(ref); return *this; }
        reverse_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        bool operator==(const reverse_iterator& other) const { return bs == other.bs && ref == other.ref; }

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

        explicit reverse_const_iterator() : bs(nullptr), ref(0, 0) {}
        reverse_const_iterator(const custom_bitset* bitset, const size_type pos) : bs(bitset), ref(pos) {}
        reverse_const_iterator(const custom_bitset* bitset, const reference& ref) : bs(bitset), ref(ref) {}
        explicit reverse_const_iterator(const iterator& it) : bs(it.bs), ref(it.ref) {}

        reference operator*() const { return ref; }

        operator bool() const { return bs->test(ref); }

        reverse_const_iterator& operator++() { ref = bs->next(ref); return *this; }
        reverse_const_iterator operator++(int) { const auto tmp = *this; ++(*this); return tmp; }

        reverse_const_iterator& operator--() { ref = bs->prev(ref); return *this; }
        reverse_const_iterator operator--(int) { const auto tmp = *this; --(*this); return tmp; }

        bool operator==(const reverse_const_iterator& other) const { return bs == other.bs && ref == other.ref; }

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
    static constexpr size_type alignment = 32; //bytes

    const size_type _size;
    std::vector<block_type, aligned_allocator<block_type, alignment>> _bits;

    static constexpr size_type get_block(const size_type pos) noexcept { return pos >> block_size_log2; };
    static constexpr size_type get_block_bit(const size_type pos) noexcept { return pos & below_mask(block_size_log2); }

    // Used to allocate at least one block regardless of size
    static constexpr size_type blocks_needed(const size_type size) noexcept { return get_block(size + block_size-1) | (size == 0); }

    // it's necessary because even if size it's 0, we still have a block inside bits vector
    static constexpr size_type get_last_block(const size_type size) noexcept { return get_block(size + block_size-1) - (size != 0); }

    static constexpr block_type mask_bit(bit_type bit) noexcept;
    static constexpr block_type below_mask(bit_type bit) noexcept;
    static constexpr block_type until_mask(bit_type bit) noexcept;
    static constexpr block_type from_mask(bit_type bit) noexcept;
    static constexpr block_type after_mask(bit_type bit) noexcept;

    template <bool pop>
    reference front_impl(this auto&& self) noexcept;

    template <bool pop>
    reference next_impl(this auto&& self, reference ref);

    template <bool pop>
    reference back_impl(this auto&& self) noexcept;

    template <bool pop>
    reference prev_impl(this auto&& self, reference ref);

public:
    custom_bitset() : custom_bitset(0) {}
    explicit custom_bitset(size_type size, bool default_value = false);

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
    static void BEFORE(custom_bitset& dest, const custom_bitset& src, const reference& ref);
    static void BEFORE(custom_bitset& dest, const custom_bitset& src, size_type pos);
    static void UNTIL(custom_bitset& dest, const custom_bitset& src, const reference& ref);
    static void UNTIL(custom_bitset& dest, const custom_bitset& src, size_type pos);
    static void AFTER(custom_bitset& dest, const custom_bitset& src, const reference& ref);
    static void AFTER(custom_bitset& dest, const custom_bitset& src, size_type pos);
    static void FROM(custom_bitset& dest, const custom_bitset& src, const reference& ref);
    static void FROM(custom_bitset& dest, const custom_bitset& src, size_type pos);
    static custom_bitset complement(custom_bitset src);

    custom_bitset operator~() const;
    bool operator==(const custom_bitset& other) const;
    custom_bitset& operator=(const custom_bitset& other);
    custom_bitset& operator=(custom_bitset&& other) noexcept;

    friend custom_bitset operator&(custom_bitset lhs, const custom_bitset& rhs);
    friend custom_bitset operator|(custom_bitset lhs, const custom_bitset& rhs);
    friend custom_bitset operator^(custom_bitset lhs, const custom_bitset& rhs);
    friend custom_bitset operator-(custom_bitset lhs, const custom_bitset& rhs);

    custom_bitset& operator&=(const custom_bitset& other);
    custom_bitset& operator|=(const custom_bitset& other);
    custom_bitset& operator^=(const custom_bitset &other);
    custom_bitset& operator-=(const custom_bitset& other);

    static void AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2);
    static void AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference& end);
    static void AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, size_type end_pos);
    static void AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference& start, const reference& end);
    static void AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, size_type start_pos, size_type end_pos);
    static void OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2);
    static void OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference& end);
    static void OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, size_type end_pos);
    static void OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference& start, const reference& end);
    static void OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, size_type start_pos, size_type end_pos);
    static void XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2);
    static void XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference& end);
    static void XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, size_type end_pos);
    static void XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference& start, const reference& end);
    static void XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, size_type start_pos, size_type end_pos);
    static void DIFF(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2);
    static void DIFF(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference& end);
    static void DIFF(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, size_type end_pos);
    static void DIFF(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference& start, const reference& end);
    static void DIFF(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, size_type start_pos, size_type end_pos);

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

    [[nodiscard]] reference front() const noexcept { return front_impl<false>(); }
    inline reference pop_front() noexcept { return front_impl<true>(); }

    [[nodiscard]] reference next(const reference& ref) const { return next_impl<false>(ref); }
    [[nodiscard]] reference next(const size_type pos) const { return next(reference(pos)); }
    inline reference pop_next(const reference& ref) { return next_impl<true>(ref); }

    [[nodiscard]] reference back() const noexcept { return back_impl<false>(); }
    inline reference pop_back() noexcept { return back_impl<true>(); }

    [[nodiscard]] reference prev(const reference& ref) const { return prev_impl<false>(ref); }
    [[nodiscard]] reference prev(const size_type pos) const { return prev(reference(pos)); }
    inline reference pop_prev(const reference& ref) { return prev_impl<true>(ref); }

    [[nodiscard]] size_type degree() const noexcept { return count(); };

    void flip() noexcept;
    void set() noexcept;
    void reset() noexcept;

    [[nodiscard]] size_type size() const noexcept { return _size; }
    [[nodiscard]] size_type count() const noexcept;

    //void swap(custom_bitset& other) noexcept;

    [[nodiscard]] bool all() const noexcept;
    [[nodiscard]] bool any() const noexcept;
    [[nodiscard]] bool none() const noexcept;
    [[nodiscard]] bool intersects(const custom_bitset& other) const;
    [[nodiscard]] bool is_subset_of(const custom_bitset &other) const;
    [[nodiscard]] bool is_superset_of(const custom_bitset &other) const;
    [[nodiscard]] reference front_difference(const custom_bitset &other) const;

    //void resize(size_type new_size);

    void clear_before(const reference& ref);
    void clear_before(size_type pos);
    void clear_until(const reference& ref);
    void clear_until(size_type pos);
    void clear_after(const reference& ref);
    void clear_after(size_type pos);
    void clear_from(const reference& ref);
    void clear_from(size_type pos);
};

inline constexpr custom_bitset::reference custom_bitset::npos(std::numeric_limits<size_type>::max(), 0);

constexpr custom_bitset::block_type custom_bitset::mask_bit(const bit_type bit) noexcept {
    assert(bit < block_size);
    [[assume(bit < block_size)]];

    return block_type{1} << bit;
}

constexpr custom_bitset::block_type custom_bitset::below_mask(const bit_type bit) noexcept {
    return ~from_mask(bit);
}

constexpr custom_bitset::block_type custom_bitset::until_mask(const bit_type bit) noexcept {
    return ~after_mask(bit);
}

constexpr custom_bitset::block_type custom_bitset::from_mask(const bit_type bit) noexcept {
    assert(bit < block_size);
    [[assume(bit < block_size)]];

    return ~block_type{0} << bit;
}

constexpr custom_bitset::block_type custom_bitset::after_mask(const bit_type bit) noexcept {
    assert(bit < block_size);
    [[assume(bit < block_size)]];

    return ~block_type{1} << bit;
}

template <>
struct std::formatter<custom_bitset::reference> : std::formatter<custom_bitset::size_type> {
    auto format(const custom_bitset::reference& id, std::format_context& ctx) const {
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

/*
inline custom_bitset::custom_bitset(custom_bitset other, const size_type size): custom_bitset(std::move(other)) {
    this->resize(size);
}
*/

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

inline void custom_bitset::BEFORE(custom_bitset& dest, const custom_bitset& src, const reference &ref) {
    const auto a = std::assume_aligned<alignment>(dest._bits.data());
    const auto b = std::assume_aligned<alignment>(src._bits.data());

    for (size_type i = 0; i < ref.block+1; ++i)
        a[i] = b[i];
    dest.clear_from(ref);
}

inline void custom_bitset::BEFORE(custom_bitset& dest, const custom_bitset& src, const size_type pos) {
    return BEFORE(dest, src, reference(pos));
}

inline void custom_bitset::UNTIL(custom_bitset& dest, const custom_bitset& src, const reference &ref) {
    const auto a = std::assume_aligned<alignment>(dest._bits.data());
    const auto b = std::assume_aligned<alignment>(src._bits.data());

    for (size_type i = 0; i < ref.block+1; ++i)
        a[i] = b[i];
    dest.clear_after(ref);
}

inline void custom_bitset::UNTIL(custom_bitset& dest, const custom_bitset& src, const size_type pos) {
    return UNTIL(dest, src, reference(pos));
}

inline void custom_bitset::AFTER(custom_bitset& dest, const custom_bitset& src, const reference &ref) {
    const auto a = std::assume_aligned<alignment>(dest._bits.data());
    const auto b = std::assume_aligned<alignment>(src._bits.data());

    for (size_type i = ref.block; i < dest._bits.size(); ++i)
        a[i] = b[i];
    dest.clear_until(ref);
}

inline void custom_bitset::AFTER(custom_bitset& dest, const custom_bitset& src, const size_type pos) {
    return AFTER(dest, src, reference(pos));
}

inline void custom_bitset::FROM(custom_bitset& dest, const custom_bitset& src, const reference &ref) {
    const auto a = std::assume_aligned<alignment>(dest._bits.data());
    const auto b = std::assume_aligned<alignment>(src._bits.data());

    for (size_type i = ref.block; i < dest._bits.size(); ++i)
        a[i] = b[i];
    dest.clear_before(ref);
}

inline void custom_bitset::FROM(custom_bitset& dest, const custom_bitset& src, const size_type pos) {
    return FROM(dest, src, reference(pos));
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

inline custom_bitset& custom_bitset::operator=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    // using this functions results in a call to memcpy, slower than manual loop
    // few elements. -fno-tree-loop-distribute-patterns could resolve it
    //instructions::memcpy<alignment>(_bits.data(), other._bits.data(), _bits.size());

    const auto a = std::assume_aligned<alignment>(_bits.data());
    const auto b = std::assume_aligned<alignment>(other._bits.data());

    for (size_type i = 0; i < _bits.size(); ++i)
        a[i] = b[i];

    return *this;
}

inline custom_bitset& custom_bitset::operator=(custom_bitset&& other) noexcept {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

	//_size = other._size;
	_bits = std::move(other._bits);

    return *this;
}

inline custom_bitset& custom_bitset::operator&=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    instructions::and_inplace<alignment>(_bits.data(), other._bits.data(), _bits.size());

    return *this;
}

inline custom_bitset& custom_bitset::operator|=(const custom_bitset &other) {
    assert(size() == other.size());
    [[assume(size() == other.size())]];

    instructions::or_inplace<alignment>(_bits.data(), other._bits.data(), _bits.size());

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

    instructions::diff_inplace<alignment>(_bits.data(), other._bits.data(), _bits.size());

    return *this;
}

inline void custom_bitset::AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];

    instructions::and_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), dest._bits.size());
}

inline void custom_bitset::AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference &end) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    assert(end <= dest.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];
    [[assume(end <= dest.size())]];

    instructions::and_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), end.block);
    dest._bits[end.block] = (src1._bits[end.block] & src2._bits[end.block]) & below_mask(end.bit);
    instructions::memset<alignment>(dest._bits.data(), 0, end.block+1, dest._bits.size());
}

inline void custom_bitset::AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2,
    const size_type end_pos) {
    AND(dest, src1, src2, reference(end_pos));
}

inline void custom_bitset::AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference &start, const reference &end) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    assert(start < dest.size());
    assert(end <= dest.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];
    [[assume(start < dest.size())]];
    [[assume(end <= dest.size())]];

    instructions::memset<alignment>(dest._bits.data(), 0, start.block);
    dest._bits[start.block] = (src1._bits[start.block] & src2._bits[end.block]) & from_mask(start.bit);
    instructions::and_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), start.block+1, end.block);
    dest._bits[end.block] = (src1._bits[end.block] & src2._bits[end.block]) & below_mask(end.bit);
    instructions::memset<alignment>(dest._bits.data(), 0, end.block+1, dest._bits.size());
}

inline void custom_bitset::AND(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2,
    const size_type start_pos, const size_type end_pos) {
    AND(dest, src1, src2, reference(start_pos), reference(end_pos));
}

inline void custom_bitset::OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];

    instructions::or_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), dest._bits.size());
}

inline void custom_bitset::OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference &end) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    assert(end <= dest.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];
    [[assume(end <= dest.size())]];

    instructions::or_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), end.block);
    dest._bits[end.block] = (src1._bits[end.block] | src2._bits[end.block]) & below_mask(end.bit);
    instructions::memset<alignment>(dest._bits.data(), 0, end.block+1, dest._bits.size());
}

inline void custom_bitset::OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2,
    const size_type end_pos) {
    OR(dest, src1, src2, reference(end_pos));
}

inline void custom_bitset::OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference &start, const reference &end) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    assert(start < dest.size());
    assert(end <= dest.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];
    [[assume(start < dest.size())]];
    [[assume(end <= dest.size())]];

    instructions::memset<alignment>(dest._bits.data(), 0, start.block);
    dest._bits[start.block] = (src1._bits[start.block] | src2._bits[end.block]) & from_mask(start.bit);
    instructions::or_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), start.block+1, end.block);
    dest._bits[end.block] = (src1._bits[end.block] | src2._bits[end.block]) & below_mask(end.bit);
    instructions::memset<alignment>(dest._bits.data(), 0, end.block+1, dest._bits.size());
}

inline void custom_bitset::OR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2,
    const size_type start_pos, const size_type end_pos) {
    OR(dest, src1, src2, reference(start_pos), reference(end_pos));
}

inline void custom_bitset::XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];

    instructions::xor_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), dest._bits.size());
}

inline void custom_bitset::XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference &end) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    assert(end <= dest.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];
    [[assume(end <= dest.size())]];

    instructions::xor_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), end.block);
    dest._bits[end.block] = (src1._bits[end.block] ^ src2._bits[end.block]) & below_mask(end.bit);
    instructions::memset<alignment>(dest._bits.data(), 0, end.block+1, dest._bits.size());
}

inline void custom_bitset::XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2,
    const size_type end_pos) {
    XOR(dest, src1, src2, reference(end_pos));
}

inline void custom_bitset::XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference &start, const reference &end) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    assert(start < dest.size());
    assert(end <= dest.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];
    [[assume(start < dest.size())]];
    [[assume(end <= dest.size())]];

    instructions::memset<alignment>(dest._bits.data(), 0, start.block);
    dest._bits[start.block] = (src1._bits[start.block] ^ src2._bits[end.block]) & from_mask(start.bit);
    instructions::xor_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), start.block+1, end.block);
    dest._bits[end.block] = (src1._bits[end.block] ^ src2._bits[end.block]) & below_mask(end.bit);
    instructions::memset<alignment>(dest._bits.data(), 0, end.block+1, dest._bits.size());
}

inline void custom_bitset::XOR(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2,
    const size_type start_pos, const size_type end_pos) {
    XOR(dest, src1, src2, reference(start_pos), reference(end_pos));
}

inline void custom_bitset::DIFF(custom_bitset &dest, const custom_bitset &src1, const custom_bitset &src2) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];

    instructions::diff_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), dest._bits.size());
}

inline void custom_bitset::DIFF(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference &end) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    assert(end <= dest.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];
    [[assume(end <= dest.size())]];

    instructions::diff_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), end.block);
    dest._bits[end.block] = (src1._bits[end.block] & ~src2._bits[end.block]) & below_mask(end.bit);
    instructions::memset<alignment>(dest._bits.data(), 0, end.block+1, dest._bits.size());
}

inline void custom_bitset::DIFF(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const size_type end_pos) {
    DIFF(dest, src1, src2, reference(end_pos));
}

inline void custom_bitset::DIFF(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2, const reference &start, const reference &end) {
    assert(dest.size() == src1.size());
    assert(src1.size() == src2.size());
    assert(start < dest.size());
    assert(end <= dest.size());
    [[assume(dest.size() == src1.size())]];
    [[assume(src1.size() == src2.size())]];
    [[assume(start < dest.size())]];
    [[assume(end <= dest.size())]];

    instructions::memset<alignment>(dest._bits.data(), 0, start.block);
    dest._bits[start.block] = (src1._bits[start.block] & ~src2._bits[end.block]) & from_mask(start.bit);
    instructions::diff_store<alignment>(dest._bits.data(), src1._bits.data(), src2._bits.data(), start.block+1, end.block);
    dest._bits[end.block] = (src1._bits[end.block] & ~src2._bits[end.block]) & below_mask(end.bit);
    instructions::memset<alignment>(dest._bits.data(), 0, end.block+1, dest._bits.size());
}

inline void custom_bitset::DIFF(custom_bitset& dest, const custom_bitset& src1, const custom_bitset& src2,
    const size_type start_pos, const size_type end_pos) {
    DIFF(dest, src1, src2, reference(start_pos), reference(end_pos));
}

inline custom_bitset::operator std::vector<custom_bitset::size_type>() const {
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
    instructions::memset<alignment>(_bits.data(), std::numeric_limits<block_type>::max(), _bits.size());
}

inline void custom_bitset::reset() noexcept {
    instructions::memset<alignment>(_bits.data(), 0, _bits.size());
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
    instructions::flip<alignment>(_bits.data(), _bits.size());
    _bits.back() &= below_mask(get_block_bit(_size));
}

inline custom_bitset::size_type custom_bitset::count() const noexcept {
    return instructions::popcount<alignment>(_bits.data(), _bits.size());
}

/*
inline void custom_bitset::swap(custom_bitset &other) noexcept {
    std::swap(_size, other._size);
    _bits.swap(other._bits);
}
*/

/*
inline void custom_bitset::resize(const size_type new_size) {
    if (_size == new_size) return;
    _size = new_size;
    _bits.resize(blocks_needed(_size));
    if (get_block_bit(_size)) _bits[get_last_block(_size)] &= below_mask(get_block_bit(_size));
}
*/

inline void custom_bitset::clear_before(const reference& ref) {
    assert(ref < _size);
    [[assume(ref < _size)]];

    instructions::memset<alignment>(_bits.data(), 0, ref.block);
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
    instructions::memset<alignment>(_bits.data(), 0, ref.block+1, _bits.size());
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

    for (size_type i = 0; i < _bits.size(); i++) {
        if (a[i] & ~b[i]) {
            auto bit = instructions::bit_scan_forward(a[i] & ~b[i]);
            return {i, bit};
        }
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
        explicit reverse_view(custom_bitset& base) : base_(&base) {}

        auto begin() { return base_->rbegin(); }
        auto end() { return base_->rend(); }
        [[nodiscard]] auto begin() const { return base_->rbegin(); }
        [[nodiscard]] auto end() const { return base_->rend(); }
    };
}
