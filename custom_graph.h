//
// Created by Beniamino Vagnarelli on 02/04/25.
//

#pragma once

#include <fstream>
#include <numeric>

#include "custom_bitset.h"

class custom_graph {
    typedef custom_bitset::size_type size_type;

    std::vector<custom_bitset> _graph;
    std::vector<size_type> _order_conversion;

public:
    explicit custom_graph(const std::string& filename);
    explicit custom_graph(size_type size);
    custom_graph(custom_graph g, size_type size);

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
    [[nodiscard]] size_type adjacent(size_type u, size_type v) const;
    [[nodiscard]] float get_density() const noexcept;

    [[nodiscard]] std::vector<size_type> convert_back_set(const std::vector<size_type> &v) const;
    [[nodiscard]] std::vector<int> convert_back_set(const std::vector<int> &v) const;
    [[nodiscard]] custom_bitset convert_back_set(const custom_bitset &bb) const;
    [[nodiscard]] custom_graph get_complement() const;
    [[nodiscard]] custom_graph change_order(const std::vector<size_type>& order) const;
    [[nodiscard]] size_type get_subgraph_edges(const custom_bitset &subset) const;
    [[nodiscard]] std::vector<size_type> get_subgraph_vertices_degree(const custom_bitset &subset) const;

    void resize(size_type new_size);
};

inline custom_graph::custom_graph(const std::string& filename) {
    std::ifstream inf(filename);
    assert(inf);

    std::string strInput;
    while (std::getline(inf, strInput)) {
        if (strInput[0] == 'p') {
            size_type nodes;
            std::istringstream(strInput.substr(7)) >> nodes;

            _graph.reserve(nodes);
            _order_conversion.resize(nodes);
            std::iota(_order_conversion.begin(), _order_conversion.end(), 0);
            for (size_type i = 0; i < nodes; ++i) {
                _graph.emplace_back(nodes);
            }
        }
        else if (strInput[0] == 'e') {
            size_type node1, node2;
            std::istringstream(strInput.substr(2)) >> node1 >> node2;

            add_edge(node1-1, node2-1);
        }
    }
}

inline custom_graph::custom_graph(const size_type size)
    : _graph(size, custom_bitset(size)), _order_conversion(size) {
    std::iota(_order_conversion.begin(), _order_conversion.end(), 0);
}

inline custom_graph::custom_graph(custom_graph g, const size_type size) : _graph(std::move(g._graph)), _order_conversion(std::move(g._order_conversion)) {
    resize(size);
}

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

inline std::vector<custom_graph::size_type> custom_graph::convert_back_set(const std::vector<size_type> &v) const {
    std::vector<size_type> set;
    set.reserve(v.size());
    for (const auto vertex : v) {
        set.push_back(_order_conversion[vertex]);
    }
    return set;
}

inline std::vector<int> custom_graph::convert_back_set(const std::vector<int> &v) const {
    std::vector<int> set;
    set.reserve(v.size());
    for (const auto vertex : v) {
        set.push_back(_order_conversion[vertex]);
    }
    return set;
}

inline custom_bitset custom_graph::convert_back_set(const custom_bitset &bb) const {
    custom_bitset set(bb.size());
    for (const auto v : bb) {
        set.set(_order_conversion[v]);
    }
    return set;
}

inline custom_graph custom_graph::get_complement() const {
    custom_graph complement(*this);
    std::iota(complement._order_conversion.begin(), complement._order_conversion.end(), 0);

    for (size_type i = 0; i < complement.size(); i++) {
        complement._graph[i].flip();
        complement._graph[i].reset(i);
    }

    return complement;
}

inline custom_graph custom_graph::change_order(const std::vector<size_type> &order) const {
    custom_graph new_g(size());
    new_g._order_conversion = order;

    std::vector<size_type> conversion_helper(size());
    for (size_type i = 0; i < _graph.size(); i++) {
        conversion_helper[order[i]] = i;
    }

    for (size_type i = 0; i < size(); i++) {
        const auto current_vertex = conversion_helper[i];
        auto set = static_cast<std::vector<size_type>>(_graph[i]);
        for (auto& v : set) v = conversion_helper[v];
        new_g._graph[current_vertex] = custom_bitset(set, size());
    }

    return new_g;
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

inline void custom_graph::resize(const size_type new_size) {
    if (size() == new_size) return;

    // add new bitsets if the new size is larger than the current size
    const custom_bitset default_bitset(new_size);
    _graph.resize(new_size, default_bitset);

    // resize each bitset in the graph
    for (auto& bitset : _graph) {
        bitset.resize(new_size);
    }

    // TODO: missing order_conversion resizing
}
