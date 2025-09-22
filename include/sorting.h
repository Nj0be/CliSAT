//
// Created by Beniamino Vagnarelli on 09/04/25.
//

#pragma once

#include "custom_graph.h"

// Minimum Weight Sort
std::vector<std::size_t> MWS(custom_graph G) {
    std::vector<std::size_t> vertices(G.size());
    std::vector<std::size_t> degrees(G.size());
    std::vector<std::size_t> neighb_deg(G.size());
    std::iota(vertices.begin(), vertices.end(), 0);

    for (std::size_t i = 0; i < G.size(); i++) {
        degrees[i] = G[i].degree();
    }
    for (std::size_t i = 0; i < G.size(); i++) {
        for (const auto v : G[i]) {
            neighb_deg[i] += degrees[v];
        }
    }

    for (std::size_t i = 1; i <= G.size(); i++) {
        for (std::size_t j = 0; j <= G.size()-i; j++) {
            const auto curr_vert = vertices[j];
            neighb_deg[curr_vert] = 0;
            for (const auto v : G[curr_vert]) {
                neighb_deg[curr_vert] += degrees[v];
            }
        }
        auto v_min = std::ranges::min_element(vertices.begin(), std::prev(vertices.end(), static_cast<std::ptrdiff_t>(i)),
            [=](const std::size_t a, const std::size_t b) {
                if (degrees[a] != degrees[b]) return degrees[a] < degrees[b];
                return neighb_deg[a] < neighb_deg[b];
            });

        // destructive to clean graph
        for (auto cursor = G[*v_min].pop_front();
             cursor != custom_bitset::npos;
             cursor = G[*v_min].pop_next(cursor)) {
            degrees[cursor]--;
            G[cursor].reset(*v_min);
        }
        std::iter_swap(v_min, std::prev(vertices.end(), static_cast<std::ptrdiff_t>(i)));
    }

    return vertices;
}

// DEG_SORT
// Minimum Weight Sort with Initial sorting
std::vector<std::size_t> MWSI(const custom_graph& G, const int p=3) {
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

        /*
         *for (auto v : G[min]) {
            // update neigh_degree (we are going to remove v_min)
            // we decrement both processed and non processed vertices, it's faster than checking
            degrees[v]--;
            // we are using MWSS (Minimum Width with Static Support) instead of MWS
            // we use static neighbor support instead of recalculating it
        }
        */
        std::swap(vertices[min_idx], vertices[i]);
    }

    // non-descending order based on original degree
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
        auto U_vec = CliSAT_no_sorting(g_complement, W);
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

std::pair<std::vector<std::size_t>, int> NEW_SORT(const custom_graph &g, const int p=3) {
    auto Odeg = MWSI(g, p);
    auto k = 0;
    //auto [Ocolor, k] = COLOUR_SORT(g);

    return {Odeg, k};
    
    /*
    if (g.get_density() <= 0.7) return {Odeg, k};

    int color_max = 0;
    const auto ordered_graph = g.change_order(Odeg);
    for (std::size_t i = 1; i < g.size(); i++) {
        auto Ubb = custom_bitset::before(ordered_graph.get_neighbor_set(i), i);
        color_max = std::max(color_max, ISEQ(ordered_graph, Ubb));
    }

    int u = 1 + color_max;

    if (k < u) { return {Ocolor, k}; }

    return {Odeg, k};
	*/
}

