//
// Created by Beniamino Vagnarelli on 09/04/25.
//

module;

#include <algorithm>
#include <chrono>
#include <vector>
#include <cstdint>
#include <iostream>
#include <numeric>

export module sorting;

import custom_graph;
import custom_bitset;
import BBMC;
import coloring;

// Minimum Weight Sort
inline std::vector<uint64_t> MWS(custom_graph g) {
    std::vector<uint64_t> vertices(g.size());
    std::vector<uint64_t> degrees(g.size());
    std::vector<uint64_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (uint64_t i = 0; i < g.size(); i++) {
        degrees[i] = g[i].degree();
    }
    for (uint64_t i = 0; i < g.size(); i++) {
        for (const auto v : g[i]) {
            neighb_deg[i] += degrees[v];
        }
    }

    for (uint64_t i = 1; i <= g.size(); i++) {
        for (uint64_t j = 0; j <= g.size()-i; j++) {
            auto curr_vert = vertices[j];
            neighb_deg[curr_vert] = 0;
            for (const auto v : g[curr_vert]) {
                neighb_deg[curr_vert] += degrees[v];
            }
        }
        auto v_min = std::ranges::min_element(vertices.begin(), vertices.end()-i,
            [=](const uint64_t a, const uint64_t b) {
                if (degrees[a] != degrees[b]) return degrees[a] < degrees[b];
                return neighb_deg[a] < neighb_deg[b];
            });

        // destructive to clean graph
        for (auto cursor = g[*v_min].pop_front();
             cursor != g[*v_min].size();
             cursor = g[*v_min].pop_next(cursor)) {
            degrees[*cursor]--;
            g[*cursor].reset(*v_min);
        }
        std::iter_swap(v_min, vertices.end()-i);
    }

    return vertices;
}

// DEG_SORT
// Minimum Weight Sort with Initial sorting
inline std::vector<uint64_t> MWSI(custom_graph g, const uint64_t p=3) {
    std::vector<uint64_t> vertices(g.size());
    std::vector<uint64_t> degrees(g.size());
    std::vector<uint64_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (uint64_t i = 0; i < g.size(); i++) {
        degrees[i] = g[i].count();
    }

    std::vector degrees_orig = degrees;
    for (uint64_t i = 0; i < g.size(); i++) {
        for (const auto v : g[i]) {
            neighb_deg[i] += degrees[v];
        }
    }

    // const int64_t k = g.size()/p;
    const uint64_t k = g.size()/p;

    for (uint64_t i = 1; i <= g.size(); i++) {
        auto v_min = std::ranges::min_element(vertices.begin(), vertices.end()-i,
            [&degrees, &neighb_deg](const uint64_t a, const uint64_t b) {
                if (degrees[a] != degrees[b]) return degrees[a] < degrees[b];
                return neighb_deg[a] < neighb_deg[b];
            });

        const auto v_min_deg = degrees[*v_min];

        for (auto v = g[*v_min].pop_front(); v != g[*v_min].size(); v = g[*v_min].pop_next(v)) {
            g[v].reset(*v_min);
            // update neigh_degree (we are going to remove v_min)
            degrees[v]--;
            neighb_deg[v] -= v_min_deg;
            for (auto u: g[v]) {
                neighb_deg[u]--;
            }
        }
        std::iter_swap(v_min, vertices.end()-i);
    }

    std::ranges::sort(vertices.begin(), vertices.begin()+k,
        [&degrees_orig](const uint64_t a, const uint64_t b) {
            return degrees_orig[a] > degrees_orig[b];
        });

    return vertices;
}

inline std::pair<std::vector<uint64_t>, uint64_t> COLOUR_SORT(const custom_graph& g) {
    const auto g_complement = g.get_complement();

    std::vector<uint64_t> Ocolor;
    uint64_t k = 0;
    custom_bitset W(g.size(), true);

    while (W.any()) {
        auto U = run_BBMC(g_complement, W);
        std::vector<uint64_t> U_vec = static_cast<std::vector<uint64_t>>(U);

        // sort by non-increasing order
        std::ranges::sort(U_vec.begin(), U_vec.end(),
            [&g](const uint64_t a, const uint64_t b) {
                return g.vertex_degree(a) > g.vertex_degree(b);
            });
        Ocolor.insert(Ocolor.end(), U_vec.begin(), U_vec.end());
        W -= U;
        k++;
    }

    return {Ocolor, k};
}

export inline std::pair<std::vector<uint64_t>, uint64_t> NEW_SORT(const custom_graph &g, const uint64_t p=3) {
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

