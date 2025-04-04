//
// Created by Beniamino Vagnarelli on 02/04/25.
//

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include "custom_bitset.h"

// TODO: calculate graph degree

class custom_graph {
    std::vector<custom_bitset> graph;
    uint64_t n_edges = 0;

public:
    explicit custom_graph(uint64_t size);
    explicit custom_graph(const std::string& filename);

    void add_edge(uint64_t u, uint64_t v);
    void remove_edge(uint64_t u, uint64_t v);

    [[nodiscard]] uint64_t size() const { return graph.size(); }
    [[nodiscard]] uint64_t get_n_edges() const { return n_edges; }

    custom_bitset get_neighbor_set(uint64_t vertex);

    [[nodiscard]] custom_bitset get_neighbor_set(const uint64_t vertex) const { return { graph[vertex] }; }
    [[nodiscard]] custom_bitset get_neighbor_set(const uint64_t vertex, const custom_bitset& set) const { return { graph[vertex] & set }; }
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const uint64_t vertex) const { return { ~graph[vertex] }; }
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const uint64_t vertex, const custom_bitset& set) const { return { ~(graph[vertex] & set) }; }

    uint64_t degree() const;
};

inline custom_graph::custom_graph(const uint64_t size) {
    graph.reserve(size);
    for (uint64_t i = 0; i < size; ++i) {
        graph.emplace_back(size);
    }
}

// TODO: change assert
inline custom_graph::custom_graph(const std::string& filename) {
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

    for (auto edge: graph)
        degree = max(degree, edge.degree());

    return degree;
}
