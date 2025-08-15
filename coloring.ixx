//
// Created by Beniamino Vagnarelli on 05/08/25.
//

module;

#include <cstdint>
#include <vector>

export module coloring;

import custom_graph;
import custom_bitset;

// Independent Set Sequential
// computes the chromatic number of a graph using a greedy strategy (heuristic)
export inline uint64_t ISEQ(const custom_graph& g, custom_bitset Ubb) {
    custom_bitset Qbb(Ubb.size());
    int64_t k = 0;
    for (k = 0; Ubb.any(); ++k) {
        Qbb = Ubb;
        for (const auto v : Qbb) {
            // at most we can remove vertices, so we don't need to start a new scan
            Qbb -= g.get_neighbor_set(v);
        }
        Ubb -= Qbb;
    }

    return k;
}

// Used to get the independent sets from ISEQ
inline std::vector<custom_bitset> ISEQ_sets(const custom_graph& g, custom_bitset Ubb) {
    std::vector<custom_bitset> independent_sets;
    custom_bitset Qbb(Ubb.size());

    while (Ubb.any()) {
        Qbb = Ubb;
        for (const auto v : Qbb) {
            Qbb -= g.get_neighbor_set(v);
        }

        independent_sets.push_back(Qbb);  // Store the independent set
        Ubb -= Qbb;
    }

    return independent_sets;
}

