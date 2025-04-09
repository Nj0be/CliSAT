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

#include "BBMC.h"

// TODO: calculate graph degree

class custom_graph {
    std::vector<custom_bitset> graph;
    std::vector<uint64_t> order_conversion;
    uint64_t n_edges = 0;

    std::vector<uint64_t> MWS(custom_graph &g);
    std::vector<uint64_t> MWSI(custom_graph g, uint64_t p);

    std::pair<std::vector<uint64_t>, uint64_t> COLOUR_SORT(custom_graph g);

    uint64_t SEQ(const custom_graph &g, custom_bitset Ubb);

    std::vector<uint64_t> NEW_SORT(custom_graph &g, uint64_t p);

public:
    //explicit custom_graph(uint64_t size);
    inline custom_graph(const std::string& filename, const uint64_t p=3);
    explicit custom_graph(const uint64_t size);
    custom_graph(const custom_graph& g) : graph(g.graph), order_conversion(g.order_conversion), n_edges(g.n_edges) {}

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
    [[nodiscard]] float get_density() const { return static_cast<float>(2*get_n_edges())/(size()*(size()-1)); }

    [[nodiscard]] std::vector<uint64_t> convert_back_set(const std::vector<uint64_t> &v) const;

    [[nodiscard]] custom_bitset convert_back_set(custom_bitset &bb) const;

    [[nodiscard]] custom_graph get_complement() const;

    [[nodiscard]] custom_graph change_order(const std::vector<uint64_t>& order);
};

/*inline custom_graph::custom_graph(const uint64_t size) : degree_conversion2(size) {
    graph.reserve(size);
    for (uint64_t i = 0; i < size; ++i) {
        graph.emplace_back(size);
    }
}*/

inline std::vector<uint64_t> custom_graph::MWS(custom_graph& g) {
    std::vector<uint64_t> vertices(g.size());
    std::vector<uint64_t> degrees(g.size());
    std::vector<uint64_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (auto i = 0; i < g.size(); i++) {
        degrees[i] = g.graph[i].degree();
    }
    for (uint64_t i = 0; i < g.size(); i++) {
        auto neigb = g.graph[i].first_bit();
        while (neigb != g.size()) {
            neighb_deg[i] += degrees[neigb];
            neigb = g.graph[i].next_bit();
        }
    }

    for (auto i = 1; i <= g.size(); i++) {

        for (uint64_t j = 0; j <= g.size()-i; j++) {
            auto curr_vert = vertices[j];
            neighb_deg[curr_vert] = 0;
            auto neigb = g.graph[curr_vert].first_bit();
            while (neigb != g.size()) {
                neighb_deg[curr_vert] += degrees[neigb];
                neigb = g.graph[curr_vert].next_bit();
            }
        }
        auto v_min = std::ranges::min_element(vertices.begin(), vertices.end()-i, [=](const uint64_t a, const uint64_t b) {
            if (degrees[a] != degrees[b]) return degrees[a] < degrees[b];

            return neighb_deg[a] < neighb_deg[b];
        });

        // destructive to clean graph
        auto vertex = g.graph[*v_min].first_bit_destructive();
        while (vertex != g.size()) {
            degrees[vertex]--;
            g.graph[vertex].unset_bit(*v_min);
            vertex = g.graph[*v_min].next_bit_destructive();
        }
        std::iter_swap(v_min, vertices.end()-i);
    }

    order_conversion = vertices;

    std::vector<uint64_t> reverse_vertices(g.size());
    for (uint64_t i = 0; i < g.size(); i++) {
        reverse_vertices[vertices[i]] = i;
    }

    return reverse_vertices;
}

// DEG_SORT
inline std::vector<uint64_t> custom_graph::MWSI(custom_graph g, const uint64_t p) {
    std::vector<uint64_t> vertices(g.size());
    std::vector<uint64_t> degrees(g.size());
    std::vector<uint64_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (auto i = 0; i < g.size(); i++) {
        degrees[i] = g[i].degree();
    }

    std::vector degrees_orig = degrees;
    for (uint64_t i = 0; i < g.size(); i++) {
        auto neigb = g.graph[i].first_bit();
        while (neigb != g.size()) {
            neighb_deg[i] += degrees[neigb];
            neigb = g.graph[i].next_bit();
        }
    }

    const int64_t k = g.size()/p;

    for (auto i = 1; i <= g.size(); i++) {

        for (uint64_t j = 0; j <= g.size()-i; j++) {
            auto curr_vert = vertices[j];
            neighb_deg[curr_vert] = 0;
            auto neigb = g.graph[curr_vert].first_bit();
            while (neigb != g.size()) {
                neighb_deg[curr_vert] += degrees[neigb];
                neigb = g.graph[curr_vert].next_bit();
            }
        }
        auto v_min = std::ranges::min_element(vertices.begin(), vertices.end()-i, [&degrees, &neighb_deg](const uint64_t a, const uint64_t b) {
            if (degrees[a] != degrees[b]) return degrees[a] < degrees[b];

            return neighb_deg[a] < neighb_deg[b];
        });

        auto vertex = g.graph[*v_min].first_bit_destructive();
        while (vertex != g.size()) {
            degrees[vertex]--;
            g.graph[vertex].unset_bit(*v_min);
            vertex = g.graph[*v_min].next_bit_destructive();
        }
        std::iter_swap(v_min, vertices.end()-i);
    }

    std::ranges::sort(vertices.begin(), vertices.begin()+k, [&degrees_orig](const uint64_t a, const uint64_t b) {
        return degrees_orig[a] > degrees_orig[b];
    });

    return vertices;
}

inline std::pair<std::vector<uint64_t>, uint64_t> custom_graph::COLOUR_SORT(custom_graph g) {
    auto g_complement = g.get_complement();
    // clear graph

    std::vector<uint64_t> Ocolor;
    uint64_t k = 0;
    custom_bitset W(g.size(), 1);

    while (W) {
        auto U = run_BBMC(g_complement, W);
        std::vector<uint64_t> U_vec = static_cast<std::vector<uint64_t>>(U);
        Ocolor.insert(Ocolor.end(), U_vec.begin(), U_vec.end());
        //U_vec.insert(U_vec.end(), Ocolor.begin(), Ocolor.end());
        //Ocolor = U_vec;
        W -= U;
        k++;
    }

    return {Ocolor, k};
}

inline uint64_t custom_graph::SEQ(const custom_graph& g, custom_bitset Ubb) {
    custom_bitset Qbb(Ubb.size());
    int64_t k = 0;
    for (k = 0; Ubb; ++k) {
        Qbb = Ubb;
        auto v = Qbb.first_bit();

        while (v != Qbb.size()) {
            // al piu' posso togliere, quindi non serve iniziare di nuovo un'altra scansione
            Qbb -= g.get_neighbor_set(v);

            //get next vertex
            v = Qbb.next_bit();
        }
        Ubb -= Qbb;
    }
    return k;
}

inline std::vector<uint64_t> custom_graph::NEW_SORT(custom_graph &g, const uint64_t p) {
    auto Odeg = MWSI(g, p);
    if (get_density() <= 0.7) return Odeg;

    auto [Ocolor, k] = COLOUR_SORT(*this);

    uint64_t color_max = 0;
    auto ordered_graph = g.change_order(Odeg);
    for (uint64_t i = 1; i < g.size(); i++) {
        custom_bitset Ubb(i, 1);
        Ubb &= g.get_neighbor_set(i);
        color_max = std::max(color_max, SEQ(g, Ubb));
    }

    uint64_t u = 1 + color_max;

    if (k < u) { return Ocolor; }

    return Odeg;
}

// TODO: change assert
inline custom_graph::custom_graph(const std::string& filename, const uint64_t p) {
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

    auto order = NEW_SORT(*this, p);

    graph = change_order(order).graph;
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

inline custom_graph custom_graph::change_order(const std::vector<uint64_t> &order) {
    custom_graph new_g(size());
    for (uint64_t i = 0; i < graph.size(); i++) {
        order_conversion[order[i]] = i;
    }

    for (auto i = 0; i < size(); i++) {
        auto current_vertex = order_conversion[i];
        std::vector<uint64_t> set = static_cast<std::vector<uint64_t>>(graph[i]);
        for (auto& v : set) v = order_conversion[v];
        new_g.graph[current_vertex] = custom_bitset(set);
    }

    return new_g;
}
