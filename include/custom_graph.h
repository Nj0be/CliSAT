//
// Created by Beniamino Vagnarelli on 02/04/25.
//

#pragma once

#include <cstring>
#include <fstream>
#include <limits>
#include <numeric>
#include <iostream>

#include "custom_bitset.h"

class custom_graph {
    typedef custom_bitset::size_type size_type;

    std::vector<custom_bitset> _graph;

public:
    explicit custom_graph() {};
    explicit custom_graph(size_type size);
    //custom_graph(custom_graph g, size_type size);
    //
    
    using iterator = std::vector<custom_bitset>::iterator;
    using const_iterator = std::vector<custom_bitset>::const_iterator;

    inline iterator begin() noexcept { return _graph.begin(); }
    inline iterator end() noexcept { return _graph.end(); }
    [[nodiscard]] inline const_iterator begin() const noexcept { return _graph.begin(); }
    [[nodiscard]] inline const_iterator end() const noexcept { return _graph.end(); }

    custom_bitset& operator[](size_type pos);
    const custom_bitset& operator[](size_type pos) const;

    void add_edge(size_type u, size_type v);
    void remove_edge(size_type u, size_type v);

    [[nodiscard]] inline size_type size() const noexcept { return _graph.size(); }
    [[nodiscard]] size_type get_n_edges() const noexcept;

    [[nodiscard]] const custom_bitset& get_neighbor_set(size_type v) const;
    [[nodiscard]] custom_bitset get_neighbor_set(size_type v, size_type threshold) const;
    [[nodiscard]] custom_bitset get_prev_neighbor_set(size_type v) const;
    [[nodiscard]] custom_bitset get_prev_neighbor_set(size_type v, const custom_bitset& set) const;
    [[nodiscard]] custom_bitset get_complement_neighbor_set(size_type v) const;
    [[nodiscard]] custom_bitset get_complement_neighbor_set(size_type v, const custom_bitset& set) const;

    [[nodiscard]] size_type degree() const noexcept;
    [[nodiscard]] size_type complement_degree() const noexcept;
    [[nodiscard]] size_type degree(size_type v) const;
    [[nodiscard]] size_type complement_degree(size_type v) const;
    [[nodiscard]] size_type vertex_degree(size_type v) const;
    [[nodiscard]] size_type complement_vertex_degree(size_type v) const;
    [[nodiscard]] size_type adjacent(size_type u, size_type v) const;
    [[nodiscard]] float get_density() const noexcept;
    [[nodiscard]] size_type get_degeneracy() const noexcept;

    [[nodiscard]] std::vector<size_type> convert_back_set(const std::vector<size_type> &v, const std::vector<size_type> &ordering) const;
    [[nodiscard]] std::vector<int> convert_back_set(const std::vector<int> &v, const std::vector<size_type> &ordering) const;
    [[nodiscard]] custom_bitset convert_back_set(const custom_bitset &bb, const std::vector<size_type> &ordering) const;
    [[nodiscard]] custom_graph get_complement() const;
    void complement();
    void change_order(const std::vector<size_type>& order);
    void restore_order(const std::vector<size_type>& order);
    [[nodiscard]] size_type get_subgraph_edges(const custom_bitset &subset) const;
    [[nodiscard]] std::vector<size_type> get_subgraph_vertices_degree(const custom_bitset &subset) const;

    void resize(size_type new_size, bool default_value = false);
};

inline custom_graph::custom_graph(const size_type size)
    : _graph(size, custom_bitset(size)) {
}

/*
inline custom_graph::custom_graph(custom_graph g, const size_type size) : _graph(std::move(g._graph)), _order_conversion(std::move(g._order_conversion)) {
    resize(size);
}
*/

inline custom_bitset& custom_graph::operator[](const size_type pos) {
    assert(pos < size());
    [[assume(pos < size())]];

    return _graph[pos];
}

inline const custom_bitset & custom_graph::operator[](const size_type pos) const {
    assert(pos < size());
    [[assume(pos < size())]];

    return _graph[pos];
}

// Rest of the implementation follows the same pattern as custom_graph
inline void custom_graph::add_edge(const size_type u, const size_type v) {
    assert (u < size());
    assert (v < size());
    [[assume(u < size())]];
    [[assume(v < size())]];

    _graph[u].set(v);
    _graph[v].set(u);
}

inline void custom_graph::remove_edge(const size_type u, const size_type v) {
    assert (u < size());
    assert (v < size());
    [[assume(u < size())]];
    [[assume(v < size())]];

    _graph[u].reset(v);
    _graph[v].reset(u);
}

inline custom_graph::size_type custom_graph::get_n_edges() const noexcept {
    size_type n_edges = 0;
    for (const auto& bitset : _graph) {
        n_edges += bitset.count();
    }
    return n_edges / 2; // Each edge is counted twice
}

inline const custom_bitset & custom_graph::get_neighbor_set(const size_type v) const {
    assert(v < size());
    [[assume(v < size())]];

    return _graph[v];
}

inline custom_bitset custom_graph::get_neighbor_set(const size_type v, const size_type threshold) const {
    // the first one maintains the size, the second one reduces it to improve performance, but it's less safe
    assert(threshold < size());
    [[assume(threshold < size())]];

    return custom_bitset::before(get_neighbor_set(v), threshold);

    //return {get_neighbor_set(v), threshold};
}

inline custom_bitset custom_graph::get_prev_neighbor_set(const size_type v) const {
    return get_neighbor_set(v, v);
}

inline custom_bitset custom_graph::get_prev_neighbor_set(const size_type v, const custom_bitset& set) const {
    //return get_prev_neighbor_set(v) & set;
    return get_prev_neighbor_set(v) & set;
}

inline custom_bitset custom_graph::get_complement_neighbor_set(const size_type v) const {
    assert(v < size());
    [[assume(v < size())]];

    return ~_graph[v];
}

inline custom_bitset custom_graph::get_complement_neighbor_set(const size_type v, const custom_bitset& set) const {
    return get_complement_neighbor_set(v) & set;
}

inline custom_graph::size_type custom_graph::degree() const noexcept {
    size_type degree = 0;
    for (const auto& edge: _graph)
        degree = std::max(degree, edge.degree());
    return degree;
}

inline custom_graph::size_type custom_graph::complement_degree() const noexcept {
    return size() - degree() - 1;
}

inline custom_graph::size_type custom_graph::degree(size_type v) const {
    assert(v < size());
    [[assume(v < size())]];

    return _graph[v].count();
}

inline custom_graph::size_type custom_graph::complement_degree(const size_type v) const {
    assert(v < size());
    [[assume(v < size())]];

    return size() - degree(v) - 1;
}

inline custom_graph::size_type custom_graph::vertex_degree(const size_type v) const {
    assert(v < size());
    [[assume(v < size())]];

    return _graph[v].count();
}

inline custom_graph::size_type custom_graph::complement_vertex_degree(const size_type v) const {
    assert(v < size());
    [[assume(v < size())]];

    return size() - _graph[v].count() - 1;
}


inline custom_graph::size_type custom_graph::adjacent(const size_type u, const size_type v) const {
    assert(u < size());
    [[assume(u < size())]];
    assert(v < size());
    [[assume(v < size())]];

    return _graph[u].test(v);
}

inline float custom_graph::get_density() const noexcept {
    return 2.0f * static_cast<float>(get_n_edges()) /
           (static_cast<float>(size()) * static_cast<float>(size() - 1));
}

// Is the same as the k-core
inline custom_graph::size_type custom_graph::get_degeneracy() const noexcept {
    std::vector<std::size_t> vertices(size());
    std::vector<std::size_t> degrees(size());
    custom_bitset is_node_processed(size());
    std::iota(vertices.begin(), vertices.end(), 0);

    std::size_t max = 0;

    for (std::size_t i = 0; i < size(); i++) {
        degrees[i] = _graph[i].count();
    }

    std::vector<std::size_t> degrees_orig = degrees;

    custom_bitset neighbors_min(size());

    for (std::ptrdiff_t i = _graph.size()-1; i > 0; i--) {
        auto min_idx = 0;
        for (int j = 1; j <= i; j++) {
            const auto node = vertices[j];
            const auto v_min = vertices[min_idx];

            if (degrees[node] < degrees[v_min]) min_idx = j;
        }

        const auto min = vertices[min_idx];

        max = std::max(max, degrees[min]);

        is_node_processed.set(min);
        custom_bitset::DIFF(neighbors_min, _graph[min], is_node_processed);
        for (auto v : neighbors_min) {
            // update neigh_degree (we are going to remove v_min)
            degrees[v]--;
        }

        std::swap(vertices[min_idx], vertices[i]);
    }

    return max;
}

inline std::vector<custom_graph::size_type> custom_graph::convert_back_set(const std::vector<size_type> &v, const std::vector<size_type> &ordering) const {
    std::vector<size_type> set;
    set.reserve(v.size());
    for (const auto vertex : v) {
        set.push_back(ordering[vertex]);
    }
    return set;
}

inline std::vector<int> custom_graph::convert_back_set(const std::vector<int> &v, const std::vector<size_type> &ordering) const {
    std::vector<int> set;
    set.reserve(v.size());
    for (const auto vertex : v) {
        set.push_back(ordering[vertex]);
    }
    return set;
}

inline custom_bitset custom_graph::convert_back_set(const custom_bitset &bb, const std::vector<size_type> &ordering) const {
    custom_bitset set(bb.size());
    for (const auto v : bb) {
        set.set(ordering[v]);
    }
    return set;
}

inline custom_graph custom_graph::get_complement() const {
    custom_graph complement(*this);

    for (size_type i = 0; i < complement.size(); i++) {
        complement._graph[i].flip();
        complement._graph[i].reset(i);
    }

    return complement;
}


inline void custom_graph::complement() {
    for (size_type i = 0; i < size(); i++) {
        _graph[i].flip();
        _graph[i].reset(i);
    }
}

inline void custom_graph::change_order(const std::vector<size_type> &order) {
    constexpr size_type invalid = std::numeric_limits<size_type>::max();

    custom_bitset temp(size());
    custom_bitset temp2(size());
    std::vector<bool> visited(size());

    // Maps original vertex -> new position
    std::vector<size_type> old_to_new(size());
    for (size_type i = 0; i < size(); ++i) {
        old_to_new[order[i]] = i;
    }

    size_type temp_pos = invalid;

    for (size_type i = 0; i < size(); ++i) {
        while (temp_pos != invalid) {
            const size_type orig_pos = temp_pos;

            if (visited[orig_pos]) {
                temp_pos = invalid;
                break;
            }

            visited[orig_pos] = true;
            const size_type new_pos = old_to_new[orig_pos];

            temp2.swap(temp);
            temp.swap(_graph[new_pos]);
            _graph[new_pos].reset();

            for (auto v : temp2) _graph[new_pos].set(old_to_new[v]);
            if (new_pos != orig_pos) temp_pos = new_pos;
            else temp_pos = invalid;
        }

        const size_type orig_pos = i;
        if (visited[orig_pos]) continue;

        visited[orig_pos] = true;
        const size_type new_pos = old_to_new[orig_pos];

        temp.swap(_graph[new_pos]);
        _graph[new_pos].reset();

        if (new_pos != orig_pos) {
            for (auto v : _graph[orig_pos]) _graph[new_pos].set(old_to_new[v]);
            temp_pos = new_pos;
        } else {
            for (auto v : temp) _graph[new_pos].set(old_to_new[v]);
            temp_pos = invalid;
        }
    }
}

inline void custom_graph::restore_order(const std::vector<size_type> &old_order) {
    std::vector<size_type> order(size());
    for (size_type i = 0; i < size(); ++i) {
        order[old_order[i]] = i;
    }
    change_order(order);
}

inline custom_graph::size_type custom_graph::get_subgraph_edges(const custom_bitset &subset) const {
    size_type edges = 0;
    for (const auto v : subset) edges += (get_neighbor_set(v) & subset).count();
    return edges/2;
}

inline std::vector<custom_graph::size_type> custom_graph::get_subgraph_vertices_degree(const custom_bitset &subset) const {
    std::vector<size_type> d(subset.size());
    for (const auto v : subset) d[v] = vertex_degree(v);
    return d;
}

inline void custom_graph::resize(const size_type new_size, const bool default_value) {
    if (size() == new_size) return;

    if (new_size > size()) {
        // resize each bitset already in the graph
        for (auto& bitset : _graph) {
            bitset.resize(new_size);
        }

        // add new bitsets if the new size is larger than the current size
        const custom_bitset default_bitset(new_size, default_value);

        auto old_size = size();

        _graph.resize(new_size, default_bitset);

        if (default_value) {
            for (int i = old_size; i < new_size; i++) {
                _graph[i].reset(i);
            }
        }
    } else {
        _graph.resize(new_size);

        // resize each bitset in the graph
        for (auto& bitset : _graph) {
            bitset.resize(new_size);
        }
    }
}
