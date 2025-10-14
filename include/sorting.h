//
// Created by Beniamino Vagnarelli on 09/04/25.
//

#pragma once

#include "custom_graph.h"
#include <chrono>

// DEG_SORT
// Minimum Weight Sort with Initial sorting
static std::vector<std::size_t> MWSI(const custom_graph& G, const int p) {
    std::vector<std::size_t> vertices(G.size());
    std::vector<std::size_t> degrees(G.size());
    std::vector<std::size_t> support(G.size());
	custom_bitset is_node_processed(G.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (std::size_t i = 0; i < G.size(); i++) {
        degrees[i] = G[i].count();
    }

    std::vector degrees_orig = degrees;
    for (std::size_t i = 0; i < G.size(); i++) {
        for (const auto v : G[i]) {
            support[i] += degrees[v];
        }
    }

    const int k = static_cast<int>(G.size()/p);

    custom_bitset neighbors_min(G.size());
    custom_bitset neighbors_v(G.size());

    for (std::ptrdiff_t i = G.size()-1; i > 0; i--) {
        auto min_idx = 0;
        for (int j = 1; j <= i; j++) {
            const auto node = vertices[j];
			const auto v_min = vertices[min_idx];

            if (degrees[node] != degrees[v_min]) {
                if (degrees[node] < degrees[v_min]) min_idx = j;
            } else if (support[node] < support[v_min]) {
                min_idx = j;
            }
        }

		const auto min = vertices[min_idx];

        is_node_processed.set(min);
        custom_bitset::DIFF(neighbors_min, G[min], is_node_processed);
        for (auto v : neighbors_min) {
            // update neigh_degree (we are going to remove v_min)
            degrees[v]--;

            support[v] -= degrees[min];
			custom_bitset::DIFF(neighbors_v, G[v], is_node_processed);
            for (auto u: neighbors_v) {
                support[u]--;
            }
        }

        std::swap(vertices[min_idx], vertices[i]);
    }

    // non-descending order based on original degree
    std::ranges::sort(vertices.begin(), vertices.begin()+k,
        [&degrees_orig](auto a, auto b) {
            return degrees_orig[a] > degrees_orig[b];
        });

    return vertices;
}

static std::vector<std::size_t> MWSSI(const custom_graph& G, const int p) {
    std::vector<std::size_t> vertices(G.size());
    std::vector<std::size_t> degrees(G.size());
    std::vector<std::size_t> support(G.size());
    custom_bitset is_node_processed(G.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (std::size_t i = 0; i < G.size(); i++) {
        degrees[i] = G[i].count();
    }

    std::vector degrees_orig = degrees;
    for (std::size_t i = 0; i < G.size(); i++) {
        for (const auto v : G[i]) {
            support[i] += degrees[v];
        }
    }

    const int k = static_cast<int>(G.size()/p);

    custom_bitset neighbors_min(G.size());
    custom_bitset neighbors_v(G.size());

    for (std::ptrdiff_t i = G.size()-1; i > 0; i--) {
        auto min_idx = 0;
        for (int j = 1; j <= i; j++) {
            const auto node = vertices[j];
			const auto v_min = vertices[min_idx];

            if (degrees[node] != degrees[v_min]) {
                if (degrees[node] < degrees[v_min]) min_idx = j;
            } else if (support[node] < support[v_min]) {
                min_idx = j;
            }
        }

		const auto min = vertices[min_idx];

        is_node_processed.set(min);
        custom_bitset::DIFF(neighbors_min, G[min], is_node_processed);
        for (auto v : neighbors_min) {
            // update neigh_degree (we are going to remove v_min)
            degrees[v]--;
        }

        std::swap(vertices[min_idx], vertices[i]);
    }

    // non-descending order based on original degree
    std::ranges::sort(vertices.begin(), vertices.begin()+k,
        [&degrees_orig](auto a, auto b) {
            return degrees_orig[a] > degrees_orig[b];
        });

    return vertices;
}

static std::vector<std::size_t> DEG_SORT(const custom_graph& G, const int p=5) {
    if (G.size() < 1000) return MWSI(G, p);
    return MWSSI(G, p);
}

static std::pair<std::vector<std::size_t>, int> COLOUR_SORT(const custom_graph& g, const std::chrono::milliseconds time_limit = std::chrono::milliseconds(50)) {
    const auto g_complement = g.get_complement();

    std::vector<std::size_t> Ocolor;
    int k = 0;
    custom_bitset W(g.size(), true);

    while (W.any()) {
        auto U_vec = CliSAT_no_sorting(g_complement, W, time_limit);
        const custom_bitset U(U_vec, g.size());

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

inline std::vector<std::size_t> NEW_SORT(const custom_graph &G, const int p=5) {
    std::vector<std::size_t> Odeg;
    Odeg = DEG_SORT(G, p);

    if (G.get_density() <= 0.7) return Odeg;

    auto [Ocolor, k] = COLOUR_SORT(G);
    int color_max = 0;
    auto ordered_graph = G;
    ordered_graph.change_order(Odeg);
    for (std::size_t i = 1; i < G.size(); i++) {
        auto Ubb = custom_bitset::before(ordered_graph.get_neighbor_set(i), i);
        color_max = std::max(color_max, ISEQ(ordered_graph, Ubb));
    }

    int u = 1 + color_max;

    if (k < u) { return Ocolor; }

    return Odeg;
}

