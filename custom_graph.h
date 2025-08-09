//
// Created by Beniamino Vagnarelli on 02/04/25.
//

#pragma once

#include <fstream>
#include <numeric>
#include <string>
#include <cassert>
#include "custom_bitset.h"
#include <sstream>
#include <ranges>

class custom_graph {
    std::vector<custom_bitset> graph;
    std::vector<uint64_t> order_conversion;
    uint64_t n_edges = 0;

public:
    explicit custom_graph(const std::string& filename);
    explicit custom_graph(uint64_t size);

    custom_bitset& operator[](const uint64_t pos) { return graph[pos]; };

    void add_edge(uint64_t u, uint64_t v);
    void remove_edge(uint64_t u, uint64_t v);

    [[nodiscard]] uint64_t size() const { return graph.size(); }
    [[nodiscard]] uint64_t get_n_edges() const { return n_edges; }

    [[nodiscard]] const custom_bitset& get_neighbor_set(const uint64_t v) const { return graph[v]; }
    [[nodiscard]] custom_bitset get_neighbor_set(const uint64_t v, const custom_bitset& set) const { return get_neighbor_set(v) & set; }
    [[nodiscard]] custom_bitset get_neighbor_set(const uint64_t v, const uint64_t size) const { return {get_neighbor_set(v), size}; }
    [[nodiscard]] custom_bitset get_neighbor_set(const uint64_t v, const custom_bitset& set, const uint64_t size) const { return get_neighbor_set(v, set).resize(size); }
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const uint64_t v) const { return ~graph[v]; }
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const uint64_t v, const custom_bitset& set) const { return get_anti_neighbor_set(v) & set; }

    [[nodiscard]] uint64_t degree() const;
    [[nodiscard]] uint64_t vertex_degree(uint64_t v) const;
    [[nodiscard]] float get_density() const;

    [[nodiscard]] std::vector<uint64_t> convert_back_set(const std::vector<uint64_t> &v) const;
    [[nodiscard]] custom_bitset convert_back_set(const custom_bitset &bb) const;
    [[nodiscard]] custom_graph get_complement() const;
    [[nodiscard]] custom_graph change_order(const std::vector<uint64_t>& order) const;
    [[nodiscard]] uint64_t get_subgraph_edges(const custom_bitset &subset) const;
    [[nodiscard]] std::vector<uint64_t> get_subgraph_vertices_degree(const custom_bitset &subset) const;
};

inline custom_graph::custom_graph(const std::string& filename) {
    std::ifstream inf(filename);
    assert(inf);

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

inline custom_graph::custom_graph(const uint64_t size)
    : graph(size, custom_bitset(size)), order_conversion(size) {
    std::iota(order_conversion.begin(), order_conversion.end(), 0);
}

// Rest of the implementation follows the same pattern as custom_graph
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

inline uint64_t custom_graph::degree() const {
    uint64_t degree = 0;
    for (const auto& edge: graph)
        degree = std::max(degree, edge.degree());
    return degree;
}

inline uint64_t custom_graph::vertex_degree(const uint64_t v) const {
    return graph[v].count();
}

inline float custom_graph::get_density() const {
    return 2.0f * static_cast<float>(get_n_edges()) /
           (static_cast<float>(size()) * static_cast<float>(size() - 1));
}

inline std::vector<uint64_t> custom_graph::convert_back_set(const std::vector<uint64_t> &v) const {
    std::vector<uint64_t> set;
    set.reserve(v.size());
    for (const auto vertex : v) {
        set.push_back(order_conversion[vertex]);
    }
    return set;
}

inline custom_bitset custom_graph::convert_back_set(const custom_bitset &bb) const {
    custom_bitset set(bb.size());
    for (const auto v : bb) {
        set.set_bit(order_conversion[v]);
    }
    return set;
}

inline custom_graph custom_graph::get_complement() const {
    custom_graph complement(*this);
    std::iota(complement.order_conversion.begin(), complement.order_conversion.end(), 0);

    for (uint64_t i = 0; i < complement.size(); i++) {
        complement.graph[i].negate();
        complement.graph[i].unset_bit(i);
    }

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
        auto set = static_cast<std::vector<uint64_t>>(graph[i]);
        for (auto& v : set) v = conversion_helper[v];
        new_g.graph[current_vertex] = custom_bitset(set);
    }

    return new_g;
}

inline uint64_t custom_graph::get_subgraph_edges(const custom_bitset &subset) const {
    uint64_t edges = 0;
    for (const auto v : subset) edges += (get_neighbor_set(v) & subset).count();
    return edges/2;
}

inline std::vector<uint64_t> custom_graph::get_subgraph_vertices_degree(const custom_bitset &subset) const {
    std::vector<uint64_t> d(subset.size());
    for (const auto v : subset) d[v] = vertex_degree(v);
    return d;
}