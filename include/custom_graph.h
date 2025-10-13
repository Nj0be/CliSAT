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
    std::vector<size_type> _order_conversion;

public:
    explicit custom_graph(const std::string& filename);
    explicit custom_graph(size_type size);
    //custom_graph(custom_graph g, size_type size);

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
    void change_order(const std::vector<size_type>& order);
    [[nodiscard]] size_type get_subgraph_edges(const custom_bitset &subset) const;
    [[nodiscard]] std::vector<size_type> get_subgraph_vertices_degree(const custom_bitset &subset) const;

    //void resize(size_type new_size);
};

/* In this function we are reading chunks of 8196 bytes from the file
 * based on the line type we do 3 things:
 *  - line type 'c' is a comment, skip
 *  - line type 'p edge' is the graph node and edges count, parse only the third and fourth elements
 *  - line type 'e' is and edge, parse only the second and third elements (node1 and node2)
 *  - line type '\n' is an empty line
 *
 *  We introduced a new extension to the DIMACS format, adding "p clique" problem type
 *  In "p clique" we specify number of nodes, number of edges to read and number of cliques to read.
 *  Then we add a new type of line 'q', that specify as second element the number n of nodes of the clique
 *  and as the n elements if specify the nodes of a clique
 */
inline custom_graph::custom_graph(const std::string& filename) {
    std::ifstream inf(filename);
    char buf[0x2000];
    char line_type = 'c';
    bool first_char = true;
    bool last_space_tab = false;
    std::size_t num1 = 0;
    std::size_t num2 = 0;
    std::size_t num3 = 0;
    std::string type;
    int spaces = 0;
    custom_bitset numbers;

    std::size_t remaining_edges = std::numeric_limits<std::size_t>::max();
    std::size_t remaining_cliques = std::numeric_limits<std::size_t>::max();

    while (inf) {
        inf.read(buf, sizeof(buf));
        std::streamsize bytes_read = inf.gcount();
        if (bytes_read <= 0) break;

        for (size_t i = 0; i < bytes_read; ++i) {
            auto c = buf[i];
            if (first_char) {
                line_type = c;
                first_char = (c == '\n');
                last_space_tab = false;
                spaces = 0;
                continue;
            }
            if (c == '\r') continue;
            if (c == '\n') {
                first_char = true;
                switch (line_type) {
                    case 'c':
                        break;
                    case 'p':
                        // num1 == nodes
                        // num2 == edges
                        // num3 == cliques

                        _graph.reserve(num1);
                        _order_conversion.resize(num1);
                        numbers.resize(num1);

                        remaining_edges = num2;
                        remaining_cliques = num3;

                        std::iota(_order_conversion.begin(), _order_conversion.end(), 0);
                        for (size_type j = 0; j < num1; ++j) {
                            _graph.emplace_back(num1);
                        }

                        num1 = 0;
                        num2 = 0;
                        num3 = 0;

                        break;
                    case 'e':
                        // num1 == node1
                        // num2 == node2
                        // num3 == nothing
                         
                        add_edge(num1-1, num2-1);
                        remaining_edges--;

                        num1 = 0;
                        num2 = 0;
                        break;
                    case 'q':
                        // num1 == numbers of nodes in clique
                        // num2 == last node read
                        // num3 == number of nodes read

                        // if last number
                        if (num3 < num1) {
                            numbers.set(num2-1);
                        }

                        int first = numbers.front();
                        int last = numbers.back();
                        
                        for (auto v = first; v != last; v = numbers.next(v)) {
                            custom_bitset::OR(_graph[v], numbers, first, last);
                            _graph[v].reset(v);
                        }
                        remaining_cliques--;

                        num1 = 0;
                        num2 = 0;
                        num3 = 0;
                        numbers.reset();
                        break;
                }

                // nothing to read
                if ((remaining_edges + remaining_cliques) == 0) break;

                continue;
            }

            if (line_type == 'c') continue;

            if ((c == ' ' || c == '\t')) {
                if (!last_space_tab) {
                    switch(line_type) {
                        case 'q':
                            if (spaces >= 2 && num3 < num1) {
                                numbers.set(num2-1);
                                num3++;
                                num2 = 0;
                            }
                    }
                    spaces++;
                    last_space_tab = true;
                }
                continue;
            }

            last_space_tab = false;
            if (line_type == 'p') {
                if (spaces == 1) type += c;
                else if (spaces == 2) num1 = num1*10 + (c - '0');
                else if (spaces == 3) num2 = num2*10 + (c - '0');
                else if (type == "clique" && spaces == 4) num3 = num3*10 + (c - '0');
            } else if (line_type == 'e') {
                if (spaces == 1) num1 = num1*10 + (c - '0');
                else if (spaces == 2) num2 = num2*10 + (c - '0');
            } else if (line_type == 'q') {
                if (spaces == 1) num1 = num1*10 + (c - '0');
                else num2 = num2*10 + (c - '0');
            }
        }
    }
}

inline custom_graph::custom_graph(const size_type size)
    : _graph(size, custom_bitset(size)), _order_conversion(size) {
    std::iota(_order_conversion.begin(), _order_conversion.end(), 0);
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

inline void custom_graph::change_order(const std::vector<size_type> &order) {
    custom_graph g_copy(*this);
    _order_conversion = order;

    std::vector<size_type> conversion_helper(size());
    for (size_type i = 0; i < _graph.size(); i++) {
        conversion_helper[order[i]] = i;
    }

    for (size_type i = 0; i < size(); i++) {
        const auto current_vertex = conversion_helper[i];
        auto set = static_cast<std::vector<size_type>>(g_copy._graph[i]);
        for (auto& v : set) v = conversion_helper[v];
        _graph[current_vertex] = custom_bitset(set, size());
    }
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

/*
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
*/
