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
    std::vector<uint64_t> degree_conversion;
    uint64_t n_edges = 0;

    std::vector<uint64_t> MWS(std::vector<custom_bitset> &g);
    std::vector<uint64_t> MWSI(std::vector<custom_bitset> &g, uint64_t p);

public:
    //explicit custom_graph(uint64_t size);
    explicit custom_graph(const std::string& filename);

    const custom_bitset& operator[](const uint64_t pos) const { return graph[pos]; };

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

    [[nodiscard]] std::vector<uint64_t> convert_back_set(const std::vector<uint64_t> &v) const;

    [[nodiscard]] std::vector<uint64_t> convert_back_set(custom_bitset &bb) const;
};

/*inline custom_graph::custom_graph(const uint64_t size) : degree_conversion2(size) {
    graph.reserve(size);
    for (uint64_t i = 0; i < size; ++i) {
        graph.emplace_back(size);
    }
}*/

inline std::vector<uint64_t> custom_graph::MWS(std::vector<custom_bitset>& g) {
    std::vector<uint64_t> vertices(g.size());
    std::vector<uint64_t> degrees(g.size());
    std::vector<uint64_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (auto i = 0; i < g.size(); i++) {
        degrees[i] = g[i].degree();
    }
    for (uint64_t i = 0; i < g.size(); i++) {
        auto neigb = g[i].first_bit();
        while (neigb != g.size()) {
            neighb_deg[i] += degrees[neigb];
            neigb = g[i].next_bit();
        }
    }

    for (auto i = 1; i <= g.size(); i++) {

        for (uint64_t j = 0; j <= g.size()-i; j++) {
            auto curr_vert = vertices[j];
            neighb_deg[curr_vert] = 0;
            auto neigb = g[curr_vert].first_bit();
            while (neigb != g.size()) {
                neighb_deg[curr_vert] += degrees[neigb];
                neigb = g[curr_vert].next_bit();
            }
        }
        auto v_min = std::ranges::min_element(vertices.begin(), vertices.end()-i, [=](const uint64_t a, const uint64_t b) {
            if (degrees[a] != degrees[b]) return degrees[a] < degrees[b];

            return neighb_deg[a] < neighb_deg[b];
        });

        // destructive to clean graph
        auto vertex = g[*v_min].first_bit_destructive();
        while (vertex != g.size()) {
            degrees[vertex]--;
            g[vertex].unset_bit(*v_min);
            vertex = g[*v_min].next_bit_destructive();
        }
        std::iter_swap(v_min, vertices.end()-i);
    }

    degree_conversion = vertices;

    std::vector<uint64_t> reverse_vertices(g.size());
    for (uint64_t i = 0; i < g.size(); i++) {
        reverse_vertices[vertices[i]] = i;
    }

    return reverse_vertices;
}

inline std::vector<uint64_t> custom_graph::MWSI(std::vector<custom_bitset>& g, const uint64_t p) {
    std::vector<uint64_t> vertices(g.size());
    std::vector<uint64_t> degrees(g.size());
    std::vector<uint64_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (auto i = 0; i < g.size(); i++) {
        degrees[i] = g[i].degree();
    }

    std::vector degrees_orig = degrees;
    for (uint64_t i = 0; i < g.size(); i++) {
        auto neigb = g[i].first_bit();
        while (neigb != g.size()) {
            neighb_deg[i] += degrees[neigb];
            neigb = g[i].next_bit();
        }
    }

    const int64_t k = g.size()/p;

    for (auto i = 1; i <= g.size(); i++) {

        for (uint64_t j = 0; j <= g.size()-i; j++) {
            auto curr_vert = vertices[j];
            neighb_deg[curr_vert] = 0;
            auto neigb = g[curr_vert].first_bit();
            while (neigb != g.size()) {
                neighb_deg[curr_vert] += degrees[neigb];
                neigb = g[curr_vert].next_bit();
            }
        }
        auto v_min = std::ranges::min_element(vertices.begin(), vertices.end()-i, [&degrees, &neighb_deg](const uint64_t a, const uint64_t b) {
            if (degrees[a] != degrees[b]) return degrees[a] < degrees[b];

            return neighb_deg[a] < neighb_deg[b];
        });

        auto vertex = g[*v_min].first_bit();
        while (vertex != g.size()) {
            degrees[vertex]--;
            g[vertex].unset_bit(*v_min);
            vertex = g[*v_min].next_bit();
        }
        g[*v_min].unset_all(); // to clean graph
        std::iter_swap(v_min, vertices.end()-i);
    }

    std::ranges::sort(vertices.begin(), vertices.begin()+k, [&degrees_orig](const uint64_t a, const uint64_t b) {
        return degrees_orig[a] > degrees_orig[b];
    });

    degree_conversion = vertices;

    std::vector<uint64_t> reverse_vertices(g.size());
    for (uint64_t i = 0; i < g.size(); i++) {
        reverse_vertices[vertices[i]] = i;
    }

    return reverse_vertices;
}

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
            degree_conversion.reserve(nodes);
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
    auto reverse_vertices = MWSI(graph, 3);
    /*for (auto v:vertices) std::cout << v << " ";
    std::cout << std::endl;*/


    std::ifstream inf2(filename);
    while (std::getline(inf2, strInput)) {
        if (strInput[0] == 'e') {
            uint64_t node1, node2;
            std::istringstream(strInput.substr(2)) >> node1 >> node2;

            node1 = reverse_vertices[node1-1];
            node2 = reverse_vertices[node2-1];

            add_edge(node1, node2);
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

    for (const auto& edge: graph)
        degree = std::max(degree, edge.degree());

    return degree;
}

// convert list to original naming scheme, changed because of initial ordering (based on non-increasing degree)
inline std::vector<uint64_t> custom_graph::convert_back_set(const std::vector<uint64_t> &v) const {
    std::vector<uint64_t> set;
    set.reserve(v.size());

    for (const auto vertex : v) {
        set.push_back(degree_conversion[vertex]);
    }

    return set;
}

inline std::vector<uint64_t> custom_graph::convert_back_set(custom_bitset &bb) const {
    std::vector<uint64_t> set;
    set.reserve(bb.size());

    auto vertex = bb.first_bit();
    while (vertex != bb.size()) {
        set.push_back(degree_conversion[vertex]);
        vertex = bb.next_bit();
    }

    return set;
}
