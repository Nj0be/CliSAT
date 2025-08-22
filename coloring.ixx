//
// Created by Beniamino Vagnarelli on 05/08/25.
//

module;

#include <algorithm>
#include <vector>

export module coloring;

import custom_graph;
import custom_bitset;

// Independent Set Sequential
// computes the chromatic number of a graph using a greedy strategy (heuristic)
export inline int ISEQ(const custom_graph& g, custom_bitset Ubb) {
    custom_bitset Qbb(Ubb.size());
    int k = 0;
    for (k = 0; Ubb.any(); ++k) {
        Qbb = Ubb;
        for (const auto v : Qbb) {
            // at most we can remove vertices, so we don't need to start a new scan
            Qbb -= g.get_neighbor_set(v);
            Ubb.reset(v);
        }
    }

    return k;
}

// Used to get the independent sets from ISEQ
export inline std::vector<custom_bitset> ISEQ_sets(const custom_graph& g, custom_bitset Ubb) {
    std::vector<custom_bitset> independent_sets;
    custom_bitset Qbb(Ubb.size());

    while (Ubb.any()) {
        Qbb = Ubb;
        for (const auto v : Qbb) {
            Qbb -= g.get_neighbor_set(v);
            Ubb.reset(v);
        }

        independent_sets.push_back(Qbb);  // Store the independent set
    }

    return independent_sets;
}

export inline custom_bitset ISEQ_pruned(const custom_graph& g, custom_bitset Ubb, const int k_max) {
    custom_bitset pruned(g.size());
    custom_bitset Qbb(Ubb.size());
    int k = 0;
    for (k = 0; k < k_max; ++k) {
        Qbb = Ubb;
        for (const auto v : Qbb) {
            // at most, we can remove vertices, so we don't need to start a new scan
            Qbb -= g.get_neighbor_set(v);
            Ubb.reset(v);
        }
        // add vertices to pruned
        pruned |= Qbb;
    }
    return pruned;
}

// if we can't generate k independent sets, Ubb will be empty so then node will be fathomed (B empty)
export inline custom_bitset ISEQ_branching(
    const custom_graph& g,
    custom_bitset Ubb,
    std::vector<custom_bitset>& ISs,
    std::vector<std::size_t>& alpha,
    const int k_max
) {
    int k = 0;
    alpha = std::vector<std::size_t>(k_max+1);

    for (k = 0; k < k_max; ++k) {
        ISs[k] = Ubb;
        auto last_v = 0;
        for (const auto v : ISs[k]) {
            last_v = v;
            // at most, we can remove vertices, so we don't need to start a new scan
            ISs[k] -= g.get_neighbor_set(v);
            Ubb.reset(v);
        }
        alpha[k] = last_v;
    }
    return Ubb;
}

export inline custom_bitset ISEQ_branching(
    const custom_graph& g,
    custom_bitset Ubb,
    const int k_max
) {
    int k = 0;
    static custom_bitset Qbb(g.size());

    for (k = 0; k < k_max; ++k) {
        Qbb = Ubb;
        for (const auto v : Qbb) {
            // at most, we can remove vertices, so we don't need to start a new scan
            Qbb -= g.get_neighbor_set(v);
            Ubb.reset(v);
        }
    }
    return Ubb;
}

export inline bool is_IS(
    const custom_graph& g,
    const custom_bitset& Ubb
) {
    // every vertex of Ubb shouldn't be connected to any neighbors of other vertex of Ubb
    return std::ranges::all_of(Ubb, [&g, &Ubb](auto v) { return !g.get_neighbor_set(v).intersects(Ubb); });
}
