//
// Created by Beniamino Vagnarelli on 09/04/25.
//

#pragma once

#include "BBMC.h"
#include "custom_graph.h"

// Minimum Weight Sort
std::vector<std::size_t> MWS(custom_graph g) {
    std::vector<std::size_t> vertices(g.size());
    std::vector<std::size_t> degrees(g.size());
    std::vector<std::size_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (std::size_t i = 0; i < g.size(); i++) {
        degrees[i] = g[i].degree();
    }
    for (std::size_t i = 0; i < g.size(); i++) {
        for (const auto v : g[i]) {
            neighb_deg[i] += degrees[v];
        }
    }

    for (std::size_t i = 1; i <= g.size(); i++) {
        for (std::size_t j = 0; j <= g.size()-i; j++) {
            const auto curr_vert = vertices[j];
            neighb_deg[curr_vert] = 0;
            for (const auto v : g[curr_vert]) {
                neighb_deg[curr_vert] += degrees[v];
            }
        }
        auto v_min = std::ranges::min_element(vertices.begin(), std::prev(vertices.end(), static_cast<std::ptrdiff_t>(i)),
            [=](const std::size_t a, const std::size_t b) {
                if (degrees[a] != degrees[b]) return degrees[a] < degrees[b];
                return neighb_deg[a] < neighb_deg[b];
            });

        // destructive to clean graph
        for (auto cursor = g[*v_min].pop_front();
             cursor != custom_bitset::npos;
             cursor = g[*v_min].pop_next(cursor)) {
            degrees[cursor]--;
            g[cursor].reset(*v_min);
        }
        std::iter_swap(v_min, std::prev(vertices.end(), static_cast<std::ptrdiff_t>(i)));
    }

    return vertices;
}

// DEG_SORT
// Minimum Weight Sort with Initial sorting
std::vector<std::size_t> MWSI(custom_graph g, const int p=3) {
    std::vector<std::size_t> vertices(g.size());
    std::vector<std::size_t> degrees(g.size());
    std::vector<std::size_t> neighb_deg(g.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (std::size_t i = 0; i < g.size(); i++) {
        degrees[i] = g[i].count();
    }

    std::vector degrees_orig = degrees;
    for (std::size_t i = 0; i < g.size(); i++) {
        for (const auto v : g[i]) {
            neighb_deg[i] += degrees[v];
        }
    }

    // const int64_t k = g.size()/p;
    const int k = static_cast<int>(g.size()/p);

    for (std::size_t i = 1; i <= g.size(); i++) {
        auto v_min = std::ranges::min_element(vertices.begin(), std::prev(vertices.end(), static_cast<std::ptrdiff_t>(i)),
            [&degrees, &neighb_deg](auto a, auto b) {
                if (degrees[a] != degrees[b]) return degrees[a] < degrees[b];
                return neighb_deg[a] < neighb_deg[b];
            });

        const auto v_min_deg = degrees[*v_min];

        for (auto v = g[*v_min].pop_front(); v != custom_bitset::npos; v = g[*v_min].pop_next(v)) {
            g[v].reset(*v_min);
            // update neigh_degree (we are going to remove v_min)
            degrees[v]--;
            neighb_deg[v] -= v_min_deg;
            for (auto u: g[v]) {
                neighb_deg[u]--;
            }
        }
        std::iter_swap(v_min, std::prev(vertices.end(), static_cast<std::ptrdiff_t>(i)));
    }

    std::ranges::sort(vertices.begin(), vertices.begin()+k,
        [&degrees_orig](auto a, auto b) {
            return degrees_orig[a] > degrees_orig[b];
        });

    return vertices;
}

std::pair<std::vector<std::size_t>, int> COLOUR_SORT(const custom_graph& g) {
    const auto g_complement = g.get_complement();

    std::vector<std::size_t> Ocolor;
    int k = 0;
    custom_bitset W(g.size(), true);

    while (W.any()) {
        auto U = run_BBMC(g_complement, W);
        auto U_vec = static_cast<std::vector<std::size_t>>(U);
        //auto U_vec = CliSAT_no_sorting(g_complement, W);
        //const custom_bitset U(U_vec, g.size());

        // sort by non-increasing order
        std::ranges::sort(U_vec.begin(), U_vec.end(),
            [&g](auto a, auto b) {
                return g.vertex_degree(a) > g.vertex_degree(b);
            });
        Ocolor.insert(Ocolor.end(), U_vec.begin(), U_vec.end());
        W -= U;
        k++;
    }

    return {Ocolor, k};
}

std::pair<std::vector<std::size_t>, int> NEW_SORT(const custom_graph &g, const int p=3) {
    auto Odeg = MWSI(g, p);
    auto [Ocolor, k] = COLOUR_SORT(g);

    return {Odeg, k};
    if (g.get_density() <= 0.7) return {Odeg, k};

    int color_max = 0;
    const auto ordered_graph = g.change_order(Odeg);
    for (std::size_t i = 1; i < g.size(); i++) {
        custom_bitset Ubb(i, true);
        Ubb &= g.get_neighbor_set(i);
        color_max = std::max(color_max, ISEQ(g, Ubb));
    }

    int u = 1 + color_max;

    if (k < u) { return {Ocolor, k}; }

    return {Odeg, k};
}

