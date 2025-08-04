//
// Created by Beniamino Vagnarelli on 09/04/25.
//

#pragma once

#include <vector>
#include <cstdint>
#include "custom_graph.h"
#include "BBMC.h"

inline std::vector<uint64_t> MWS(custom_graph g) {
    std::vector<uint64_t> vertices(g.size());
    std::vector<uint64_t> degrees(g.size());
    std::vector<uint64_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (uint64_t i = 0; i < g.size(); i++) {
        degrees[i] = g[i].degree();
    }
    for (uint64_t i = 0; i < g.size(); i++) {
        auto neigb = g[i].first_bit();
        while (neigb != g.size()) {
            neighb_deg[i] += degrees[neigb];
            neigb = g[i].next_bit();
        }
    }

    for (uint64_t i = 1; i <= g.size(); i++) {

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

    return vertices;
}

// DEG_SORT
inline std::vector<uint64_t> MWSI(custom_graph g, const uint64_t p=3) {
    std::vector<uint64_t> vertices(g.size());
    std::vector<uint64_t> degrees(g.size());
    std::vector<uint64_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (uint64_t i = 0; i < g.size(); i++) {
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

    for (uint64_t i = 1; i <= g.size(); i++) {

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

        auto vertex = g[*v_min].first_bit_destructive();
        while (vertex != g.size()) {
            degrees[vertex]--;
            g[vertex].unset_bit(*v_min);
            vertex = g[*v_min].next_bit_destructive();
        }
        std::iter_swap(v_min, vertices.end()-i);
    }

    std::ranges::sort(vertices.begin(), vertices.begin()+k, [&degrees_orig](const uint64_t a, const uint64_t b) {
        return degrees_orig[a] > degrees_orig[b];
    });

    return vertices;
}

inline std::pair<std::vector<uint64_t>, uint64_t> COLOUR_SORT(const custom_graph& g) {
    const auto g_complement = g.get_complement();
    // clear graph

    std::vector<uint64_t> Ocolor;
    uint64_t k = 0;
    custom_bitset W(g.size(), true);

    while (W) {
        // TODO: USE CliSAT
        auto U = run_BBMC(g_complement, W);
        std::vector<uint64_t> U_vec = static_cast<std::vector<uint64_t>>(U);

        // sort by non-increasing order
        std::ranges::sort(U_vec.begin(), U_vec.end(), [&g](const uint64_t a, const uint64_t b) {
            return g.vertex_degree(a) > g.vertex_degree(b);
        });
        Ocolor.insert(Ocolor.end(), U_vec.begin(), U_vec.end());
        //U_vec.insert(U_vec.end(), Ocolor.begin(), Ocolor.end());
        //Ocolor = U_vec;
        W -= U;
        k++;
    }

    return {Ocolor, k};
}

inline uint64_t ISEQ(const custom_graph& g, custom_bitset Ubb) {
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

// TODO: NEW_SORT with COLOUR_SORT can be extremely slow compared to MWSI
// Maybe it isn't implemented well
inline std::pair<std::vector<uint64_t>, uint64_t> NEW_SORT(const custom_graph &g, const uint64_t p=3) {
    auto Odeg = MWSI(g, p);
    auto [Ocolor, k] = COLOUR_SORT(g);

    return {Odeg, k};
    if (g.get_density() <= 0.7) return {Odeg, k};

    uint64_t color_max = 0;
    const auto ordered_graph = g.change_order(Odeg);
    for (uint64_t i = 1; i < g.size(); i++) {
        custom_bitset Ubb(i, true);
        Ubb &= g.get_neighbor_set(i);
        color_max = std::max(color_max, ISEQ(g, Ubb));
    }

    uint64_t u = 1 + color_max;

    if (k < u) { return {Ocolor, k}; }

    return {Odeg, k};
}
