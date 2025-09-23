//
// Created by Beniamino Vagnarelli on 05/08/25.
//

#pragma once

#include <algorithm>

#include "custom_bitset.h"
#include "custom_graph.h"

// Independent Set Sequential
// computes the chromatic number of a graph using a greedy strategy (heuristic)
inline int ISEQ(const custom_graph& g, custom_bitset Ubb) {
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
inline std::vector<custom_bitset> ISEQ_sets(const custom_graph& g, custom_bitset Ubb) {
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

inline custom_bitset ISEQ_pruned(const custom_graph& g, custom_bitset Ubb, const int k_max) {
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
inline int ISEQ_branching(
    const custom_graph& g,
    const custom_bitset& Ubb,
    std::vector<custom_bitset>& ISs,
    std::vector<int>& color_class,
    const int k_max
) {
    int k = 0;

    ISs[k_max] = Ubb;

    for (k = 0; k < k_max; ++k) {
        auto v = ISs[k_max].front();
        if (v == custom_bitset::npos) return k;

        ISs[k] = ISs[k_max];

        for (; v != custom_bitset::npos; v = ISs[k].next(v)) {
            // at most, we can remove vertices, so we don't need to start a new scan
            ISs[k] -= g.get_neighbor_set(v);
            color_class[v] = k;
            ISs[k_max].reset(v);
        }
    }

    auto v = ISs[k_max].front();
    if (v == custom_bitset::npos) return k_max;

    for (; v != custom_bitset::npos; v = ISs[k_max].next(v)) {
        // at most, we can remove vertices, so we don't need to start a new scan
        color_class[v] = k_max;
    }

    return k_max+1;
}

inline custom_bitset ISEQ_branching(
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

// methods that tries to create the largest number of independent sets
inline int ISEQ_all(
    const custom_graph& g,
    custom_bitset Ubb,
    std::vector<custom_bitset>& ISs
) {
    int k = 0;

    for (k = 0; Ubb.any(); ++k) {
        ISs[k] = Ubb;
        for (const auto v : ISs[k]) {
            // at most, we can remove vertices, so we don't need to start a new scan
            ISs[k] -= g.get_neighbor_set(v);
            Ubb.reset(v);
        }
    }
    return k;
}

// methods that tries to create the largest number of independent sets
inline void ISEQ_one(
    const custom_graph& g,
    const custom_bitset &Ubb,
    custom_bitset& IS
) {
    IS = Ubb;
    for (const auto v : IS) {
        // at most, we can remove vertices, so we don't need to start a new scan
        IS -= g.get_neighbor_set(v);
    }
}

inline bool is_IS(
    const custom_graph& g,
    const custom_bitset& Ubb
) {
    /*
    for (auto v : Ubb)
        if (g.get_neighbor_set(v).intersects(Ubb)) return false;
    return true;
    */
    // every vertex of Ubb shouldn't be connected to any neighbors of other vertex of Ubb
    return std::ranges::all_of(Ubb, [&g, &Ubb](auto v) { return !g.get_neighbor_set(v).intersects(Ubb); });
}
