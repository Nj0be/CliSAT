//
// Created by Beniamino Vagnarelli on 02/04/25.
//

#pragma once

#include <fstream>
#include <numeric>
#include <string>
#include "custom_bitset.h"
#include <sstream>
#include <ranges>

// TODO: calculate graph degree

class custom_graph {
    std::vector<custom_bitset> graph;
    std::vector<uint64_t> order_conversion;
    uint64_t n_edges = 0;

public:
    //explicit custom_graph(uint64_t size);
    inline custom_graph(const std::string& filename);
    explicit custom_graph(const uint64_t size);

    custom_bitset& operator[](const uint64_t pos) { return graph[pos]; };

    void add_edge(uint64_t u, uint64_t v);
    void remove_edge(uint64_t u, uint64_t v);

    [[nodiscard]] uint64_t size() const { return graph.size(); }
    [[nodiscard]] uint64_t get_n_edges() const { return n_edges; }

    [[nodiscard]] const custom_bitset& get_neighbor_set(const uint64_t v) const { return { graph[v] }; }
    [[nodiscard]] custom_bitset get_neighbor_set(const uint64_t v, const custom_bitset& set) const { return { get_neighbor_set(v) & set }; }
    // we unset v from the anti_neighbor (don't include it)
    // I think that it doesn't matter anyway
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const uint64_t v) const { return ~graph[v]; }
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const uint64_t v, const custom_bitset& set) const { return { get_anti_neighbor_set(v) & set }; }

    [[nodiscard]] uint64_t degree() const;
    [[nodiscard]] uint64_t vertex_degree(const uint64_t v) const;
    [[nodiscard]] float get_density() const { return static_cast<float>(2*get_n_edges())/(size()*(size()-1)); }

    [[nodiscard]] std::vector<uint64_t> convert_back_set(const std::vector<uint64_t> &v) const;

    [[nodiscard]] custom_bitset convert_back_set(custom_bitset &bb) const;

    [[nodiscard]] custom_graph get_complement() const;

    [[nodiscard]] custom_graph change_order(const std::vector<uint64_t>& order) const;

    [[nodiscard]] uint64_t get_subgraph_edges(custom_bitset &subset) const;
    [[nodiscard]] std::vector<uint64_t> get_subgraph_vertices_degree(custom_bitset &subset) const;
};

/*inline custom_graph::custom_graph(const uint64_t size) : degree_conversion2(size) {
    graph.reserve(size);
    for (uint64_t i = 0; i < size; ++i) {
        graph.emplace_back(size);
    }
}*/

// TODO: change assert
inline custom_graph::custom_graph(const std::string& filename) {
    std::vector<std::vector<uint64_t>> adjacency_list;
    std::vector<uint64_t> degree_conversion2;
    std::ifstream inf(filename);

    // If we couldn't open the input file stream for reading
    assert(inf);

    // While there's still stuff left to read
    std::string strInput;
    while (std::getline(inf, strInput)) {
        if (strInput[0] == 'p') {
            uint64_t nodes;
            std::istringstream(strInput.substr(7)) >> nodes;

            graph.reserve(nodes);
            order_conversion.resize(nodes);
            std::iota(order_conversion.begin(), order_conversion.end(), 0);
            for (uint64_t i = 0; i < nodes; ++i) {
                graph.emplace_back(nodes);
            }
        }
        else if (strInput[0] == 'e') {
            uint64_t node1, node2;
            std::istringstream(strInput.substr(2)) >> node1 >> node2;

            add_edge(node1-1, node2-1);
        }
    }
}

inline custom_graph::custom_graph(const uint64_t size) : graph(size, custom_bitset(size)), order_conversion(size)  {}

inline void custom_graph::add_edge(const uint64_t u, const uint64_t v) {
    if (u >= size() || v >= size() || graph[u][v]) return;

    graph[u].set_bit(v);
    graph[v].set_bit(u);

    n_edges++;
}

inline void custom_graph::remove_edge(const uint64_t u, const uint64_t v) {
    if (u >= size() || v >= size() || !graph[u][v]) return;

    graph[u].unset_bit(v);
    graph[v].unset_bit(u);

    n_edges--;
}

// TODO: optimize (use variable and update?)
inline uint64_t custom_graph::degree() const {
    uint64_t degree = 0;

    for (const auto& edge: graph)
        degree = std::max(degree, edge.degree());

    return degree;
}

inline uint64_t custom_graph::vertex_degree(const uint64_t v) const {
    return graph[v].n_set_bits();
}

// convert list to original naming scheme, changed because of initial ordering (based on non-increasing degree)
inline std::vector<uint64_t> custom_graph::convert_back_set(const std::vector<uint64_t> &v) const {
    std::vector<uint64_t> set;
    set.reserve(v.size());

    for (const auto vertex : v) {
        set.push_back(order_conversion[vertex]);
    }

    return set;
}

inline custom_bitset custom_graph::convert_back_set(custom_bitset &bb) const {
    custom_bitset set(bb.size());

    auto vertex = bb.first_bit();
    while (vertex != bb.size()) {
        set.set_bit(order_conversion[vertex]);
        vertex = bb.next_bit();
    }

    return set;
}

// TODO: optimize
inline custom_graph custom_graph::get_complement() const {
    custom_graph complement(*this);
    std::iota(complement.order_conversion.begin(), complement.order_conversion.end(), 0);

    for (uint64_t i = 0; i < complement.size(); i++) {
        complement.graph[i].negate();
        complement.graph[i].unset_bit(i);
    }

    // calculate complement edges number
    complement.n_edges = complement.size()*(complement.size()-1)/2 - get_n_edges();

    return complement;
}

inline custom_graph custom_graph::change_order(const std::vector<uint64_t> &order) const {
    custom_graph new_g(size());
    new_g.order_conversion = order;

    std::vector<uint64_t> conversion_helper(size());
    for (uint64_t i = 0; i < graph.size(); i++) {
        conversion_helper[order[i]] = i;
    }

    for (uint64_t i = 0; i < size(); i++) {
        const auto current_vertex = conversion_helper[i];
        std::vector<uint64_t> set = static_cast<std::vector<uint64_t>>(graph[i]);
        for (auto& v : set) v = conversion_helper[v];
        new_g.graph[current_vertex] = custom_bitset(set);
    }


    return new_g;
}

inline uint64_t custom_graph::get_subgraph_edges(custom_bitset &subset) const {
    uint64_t edges = 0;
    auto v = subset.first_bit();
    while (v != subset.size()) {
        edges += (get_neighbor_set(v) & subset).n_set_bits(); // we count double in this way, because the adjacency matrix is full
        v = subset.next_bit();
    }
    return edges/2;
}

inline std::vector<uint64_t> custom_graph::get_subgraph_vertices_degree(custom_bitset &subset) const {
    std::vector<uint64_t> d(subset.size());
    auto v = subset.first_bit();
    while (v != subset.size()) {
        d[v] = vertex_degree(v);
        v = subset.next_bit();
    }
    return d;
}
