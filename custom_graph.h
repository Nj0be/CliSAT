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
    std::vector<std::pair<uint64_t, uint64_t>> degree_conversion;
    std::vector<uint64_t> degree_conversion2;
    uint64_t n_edges = 0;

public:
    //explicit custom_graph(uint64_t size);
    explicit custom_graph(const std::string& filename);

    void add_edge(uint64_t u, uint64_t v);
    void remove_edge(uint64_t u, uint64_t v);

    [[nodiscard]] uint64_t size() const { return graph.size(); }
    [[nodiscard]] uint64_t get_n_edges() const { return n_edges; }

    [[nodiscard]] const custom_bitset& get_neighbor_set(const uint64_t v) const { return { graph[v] }; }
    [[nodiscard]] const custom_bitset& get_neighbor_set(const uint64_t v, const custom_bitset& set) const { return { get_neighbor_set(v) & set }; }
    // we unset v from the anti_neighbor (don't include it)
    // I think that it doesn't matter anyway
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const uint64_t v) const { return ~graph[v]; }
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const uint64_t v, const custom_bitset& set) const { return { get_anti_neighbor_set(v) & set }; }

    [[nodiscard]] uint64_t degree() const;

    std::vector<uint64_t> convert_back_set(const std::vector<uint64_t> &v) const;

    std::vector<uint64_t> convert_back_set(custom_bitset &bb) const;
};

/*inline custom_graph::custom_graph(const uint64_t size) : degree_conversion2(size) {
    graph.reserve(size);
    for (uint64_t i = 0; i < size; ++i) {
        graph.emplace_back(size);
    }
}*/

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
            degree_conversion.reserve(nodes);
            degree_conversion2.resize(nodes);
            for (uint64_t i = 0; i < nodes; ++i) {
                graph.emplace_back(nodes);
                degree_conversion.emplace_back(0, i);
            }
        }
        else if (strInput[0] == 'e') {
            uint64_t node1, node2;
            std::istringstream(strInput.substr(2)) >> node1 >> node2;

            degree_conversion[node1-1].first++;
            degree_conversion[node2-1].first++;
        }
    }

    std::ranges::sort(degree_conversion, ranges::greater());
    /*for (int i = 0; i < degree_conversion.size(); i++) {
        std::cout << degree_conversion[i].first << " " << degree_conversion[i].second << "  ";
    }
    std::cout << std::endl;*/

    for (int i = 0; i < degree_conversion.size(); i++) {
        degree_conversion2[degree_conversion[i].second] = i;
    }

    std::ifstream inf2(filename);
    while (std::getline(inf2, strInput)) {
        if (strInput[0] == 'e') {
            uint64_t node1, node2;
            std::istringstream(strInput.substr(2)) >> node1 >> node2;

            node1 = degree_conversion2[node1 - 1];
            node2 = degree_conversion2[node2 - 1];

            add_edge(node1, node2);
        }
    }
    /*for (auto bits : graph) {
        std::cout << bits << std::endl;
    }
    std::cout << std::endl;*/
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

// convert list to original naming scheme, changed because of initial ordering (based on non-increasing degree)
inline std::vector<uint64_t> custom_graph::convert_back_set(const std::vector<uint64_t> &v) const {
    std::vector<uint64_t> set;
    set.reserve(v.size());

    for (const auto vertex : v) {
        set.push_back(degree_conversion[vertex].second);
    }

    return set;
}

inline std::vector<uint64_t> custom_graph::convert_back_set(custom_bitset &bb) const {
    std::vector<uint64_t> set;
    set.reserve(bb.size());

    auto vertex = bb.first_bit();
    while (vertex != bb.size()) {
        set.push_back(degree_conversion[vertex].second);
        vertex = bb.next_bit();
    }

    return set;
}
