//
// Created by Beniamino Vagnarelli on 05/08/25.
//

#pragma once

#include <algorithm>

#include "custom_bitset.h"
#include "custom_graph.h"

// Independent Set Sequential
// computes the chromatic number of a graph using a greedy strategy (heuristic)
inline int ISEQ(const custom_graph& G, custom_bitset Ubb) {
    static custom_bitset Qbb(G.size());

    int k = 0;
    for (k = 0; Ubb.any(); ++k) {
        Qbb = Ubb;
        for (const auto v : Qbb) {
            // at most we can remove vertices, so we don't need to start a new scan
            Qbb -= G.get_neighbor_set(v);
            Ubb.reset(v);
        }
    }

    return k;
}

// if we can't generate k independent sets, Ubb will be empty so then node will be fathomed (B empty)
inline int ISEQ_branching(
    const custom_graph& g,
    const custom_bitset& Ubb,
    std::vector<custom_bitset>& ISs,
    std::vector<int>& color_class,
    const int k_max
) {
    assert(k_max >= 0);

    int k = 0;

    ISs[k_max].copy_same_size(Ubb);

    for (k = 0; k < k_max; ++k) {
        auto v = ISs[k_max].front();
        if (v == custom_bitset::npos) return k;

        ISs[k].copy_same_size(ISs[k_max]);

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


// if we can't generate k independent sets, Ubb will be empty so then node will be fathomed (B empty)
inline int ISEQ_branching_recol(
    const custom_graph& g,
    const custom_bitset& Ubb,
    std::vector<custom_bitset>& ISs,
    std::vector<int>& color_class,
    const int k_max
) {
    assert(k_max >= 0);

    int k = 0;

    ISs[k_max].copy_same_size(Ubb);

    for (k = 0; k < k_max; ++k) {
        auto v = ISs[k_max].front();
        if (v == custom_bitset::npos) return k;

        ISs[k].copy_same_size(ISs[k_max]);

        for (; v != custom_bitset::npos; v = ISs[k].next(v)) {
            // at most, we can remove vertices, so we don't need to start a new scan
            ISs[k] -= g.get_neighbor_set(v);
            color_class[v] = k;
            ISs[k_max].reset(v);
        }
    }

    // try to recolor
    for (auto v = ISs[k_max].front(); v != custom_bitset::npos; v = ISs[k_max].next(v)) {
        bool recolored = false;

        // we use k < k_min because in the last iteration we could do a single swap anyway
        // it clearly doesn't enter the second for (with k2) because k1+1 == k_min -> exit
        for (int64_t k1 = 0; k1 < k_max; ++k1) {
            // c = a; c &= b; is faster than assign c = (a & b) !!!
            // less memory copy
            ISs[k_max+1] = ISs[k1];
            ISs[k_max+1] &= g.get_neighbor_set(v);
            const auto w = ISs[k_max].front();
            // if the intersection between Ck1 and N(v) = 0 then we can put v in Ck1
            if (w == custom_bitset::npos) { // empty set - single swap
                // Ck1 = Ck1 U v
                //C[v] = k1;
                ISs[k1].set(v);
                ISs[k_max].reset(v);
                recolored = true;
                break;
            }
            // if the intersection between |Ck1 and N(v)| = 1 then we could search another set where to put w (the only vertex adjacent to v)
            if (ISs[k_max+1].next(w) == custom_bitset::npos) {  // |set| = 1 -> double swap
                for (int64_t k2 = k1+1; k2 < k_max; ++k2) {
                    ISs[k_max+1] = ISs[k2];
                    ISs[k_max+1] &= g.get_neighbor_set(w);
                    // if the intersection between Ck2 and N(w) = 0 then we can put w in Ck2 and v in Ck1
                    if (ISs[k_max+1].none()) {
                        // Ck1 = (Ck1 \ w) U v
                        //C[v] = k1;
                        ISs[k1].reset(w);
                        ISs[k1].set(v);
                        // Ck2 = Ck2 U w
                        //C[w] = k2;
                        ISs[k2].set(w);
                        ISs[k_max].reset(v);
                        recolored = true;
                        break;
                    }
                }
                if (recolored) break;
            }
        }

        if (!recolored) color_class[v] = k_max;
    }

    if (ISs[k_max].none()) return k_max;
    return k_max+1;
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
    const custom_graph& G,
    const custom_bitset& Ubb
) {
    /*
    for (auto v : Ubb)
        if (g.get_neighbor_set(v).intersects(Ubb)) return false;
    return true;
    */
    // every vertex of Ubb shouldn't be connected to any neighbors of other vertex of Ubb
    return std::ranges::all_of(Ubb, [&G, &Ubb](auto v) { return !G.get_neighbor_set(v).intersects(Ubb); });
}

inline bool is_clique(
    const custom_graph& G,
    const custom_bitset& Ubb
) {
    custom_bitset neighbors(G.size());
    for (auto v : Ubb) {
        neighbors.copy_same_size(G.get_neighbor_set(v));
        neighbors.set(v);
        if (!neighbors.is_superset_of(Ubb)) return false;
    }
    return true;
}
