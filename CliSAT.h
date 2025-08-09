//
// Created by Beniamino Vagnarelli on 08/04/25.
//

#pragma once

#include <vector>

#include "AMTS.h"
#include "custom_bitset.h"
#include "custom_graph.h"
#include "sorting.h"


// TODO: implement
inline void SATCOL() {

}

// TODO: implement
inline void FiltCOL() {

}

// TODO: implement
inline void FiltSAL() {

}

inline custom_bitset ISEQ_pruned(const custom_graph& g, custom_bitset Ubb, const uint64_t k_max) {
    custom_bitset pruned(g.size());
    custom_bitset Qbb(Ubb.size());
    uint64_t k = 0;
    for (k = 0; k < k_max; ++k) {
        Qbb = Ubb;
        for (const auto v : Qbb) {
            // at most, we can remove vertices, so we don't need to start a new scan
            Qbb -= g.get_neighbor_set(v);
        }
        // add vertices to pruned
        pruned |= Qbb;
        Ubb -= Qbb;
    }
    return pruned;
}

/**
 * \brief Inserts the biggest vertices of P into r independent sets.
 *
 * Page 8 (143) Li et al. (2018a, 2017), section 5.2.1
 *
 * @param G Ordered graph
 * @param P Candidate set {p1, p2, ..., p|P|} of vertices to be inserted into the independent sets
 * @param r lower bound related to the largest clique found so far
 * @param VertexUB incremental upper bounds for each vertex
 * @return a set B of branching vertices that cannot be inserted into any of the r ISs and the r ISs themselves
 */
inline std::pair<custom_bitset, std::vector<custom_bitset>> FilterByColoring(
    const custom_graph& G,
    const custom_bitset& P,
    const uint64_t r,
    std::vector<uint64_t>& VertexUB
) {
    custom_bitset B(P.size());  // B, a set of branching vertices obtained from P
    std::vector<custom_bitset> ISs;  // set of independent sets

    // We insert the biggest vertices of P into r ISs (decreasing order)
    for (const auto pi : std::views::reverse(P)) {
        bool inserted = false;

        // If there is an IS in which pi is not adjacent to any vertex, we insert pi into that IS
        for (auto &is: ISs) {
            auto common_neighbors = G.get_neighbor_set(pi, is);
            if (common_neighbors.none()) {
                is.set_bit(pi);
                if (B.none()) {
                    VertexUB[pi] = std::min(VertexUB[pi], ISs.size());
                }
                inserted = true;
                break;
            }
        }
        if (inserted) continue; // pi has been inserted into an IS, we can skip it

        if (ISs.size() < r) {
            // If there is no IS, we create a new one
            custom_bitset new_is(G.size());
            new_is.set_bit(pi);
            ISs.push_back(new_is);
            if (B.none()) {
                VertexUB[pi] = std::min(VertexUB[pi], ISs.size());
            }
        } else {
            // if there is an IS in which pi has only one adjacent vertex u, and u can be inserted into another IS
            // RECOLOR (RE-NUMBER of MCS)
            for (uint64_t i = 0; i < ISs.size(); ++i) {
                auto common_neighbors = G.get_neighbor_set(pi, ISs[i]);
                auto u = common_neighbors.front();

                // if there are more than one common neighbor, we can't insert pi into this IS
                if (common_neighbors.next(u) != common_neighbors.end()) continue;

                // u is the only neighbor of pi in is
                for (uint64_t j = 0; j < ISs.size(); ++j) {
                    if (j == i) continue; // skip the current IS

                    auto intersection = G.get_neighbor_set(*u, ISs[j]);

                    // if the intersection is not none, we can't insert u into this IS
                    if (!intersection.none()) continue;

                    // we can insert u in this IS, removing u from the previous IS and then insert pi into it
                    ISs[i].unset_bit(u);
                    ISs[i].set_bit(pi);
                    ISs[j].set_bit(u);

                    if (B.none()) { VertexUB[pi] = std::min(VertexUB[pi], ISs.size()); }

                    inserted = true;
                    break;
                }
                if (inserted) break;
            }
            if (!inserted) B.set_bit(pi);
        }
    }

    return {B, ISs};
}

/**
 *
 */
inline custom_bitset IncMaxSat(
    const custom_graph& G,
    const custom_bitset& P,
    // const custom_bitset& A,
    const custom_bitset& B,
    std::vector<custom_bitset>& ISs,
    const uint64_t r,
    std::vector<uint64_t>& VertexUB
) {
    const custom_bitset A = P - B; // A is the set of vertices that can be inserted into the independent sets

    return B;
    //
}

// TODO: P pass by reference or not? I don't think so
// we pass u by copy, not reference!
inline void FindMaxClique(
    const custom_graph& G,  // graph
    custom_bitset& K,       // current branch
    custom_bitset& K_max,   // max branch
    uint64_t& lb,           // lower bound
    const custom_bitset& V, // vertices set
    custom_bitset &P,       // pruned set
    custom_bitset &B,       // branching set
    std::vector<uint64_t> u // incremental upper bounds
) {
    //auto B = V - P; // branching set, vertices not in P
    for (const auto bi : B) {
        const custom_bitset bi_preced_neighbor_set = G.get_neighbor_set(bi, V, bi);

        // calculate u[bi]
        // if bi == 0, u[bi] always == 1!
        if (bi == 0) {
            u[bi] = 1;
        } else {
            uint64_t max_u = 0;
            for (const auto neighbor : bi_preced_neighbor_set) {
                max_u = std::max(max_u, u[neighbor]);
            }
            u[bi] = 1 + max_u;
        }


        if (u[bi] + K.count() <= lb) {
            P.set_bit(bi);
            //B.unset_bit(bi);
        } else {
            K.set_bit(bi);
            auto V_new = (P & G.get_neighbor_set(bi)) | (B & bi_preced_neighbor_set);

            if (V_new.none()) {
                if (K.count() > lb) {
                    lb = K.count();
                    K_max = K;
                    //std::cout << lb << std::endl;
                }
                K.unset_bit(bi);
                // TODO: continue or return?
                return;
            }
            auto P_new = ISEQ_pruned(G, V_new, lb-K.count());
            auto B_new = V_new - P_new;

            /*
            // TODO
            // PMAX-SAT-P-based upper bounds (we improve UP and find new branching nodes)
            auto [B_new, ISs] = FilterByColoring(G, V_new, lb - K.count(), u);
            if (B_new.none()) continue;

            B_new = IncMaxSat(G, V_new, B_new, ISs, lb - K.count(), u);
            if (B_new.none()) continue;
            */

            // FiltCOL
            // FiltSAT
            // SATCOL

            // if B is not none
            if (!B_new.none()) {
                // auto P_new = V_new - B_new;
                FindMaxClique(G, K, K_max, lb, V_new, P_new, B_new, u);
            }
            K.unset_bit(bi);
        }
        // TODO: here or above?

        u[bi] = std::min(u[bi], lb - K.count());
    }
}

inline custom_bitset CliSAT(const custom_graph& g) {
    auto [ordering, k] = NEW_SORT(g);
    const auto ordered_g = g.change_order(ordering);

    auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     ANTS Tabu search
    uint64_t lb = K_max.count();

    std::vector<uint64_t> u(ordered_g.size());
    // first |k_max| values bounded by |K_max| (==lb)
    u[0] = 1;
    for (uint64_t i = 1; i < lb; i++) {
        uint64_t max_u = 0;
        const custom_bitset preced_neighb_set = ordered_g.get_neighbor_set(i, i);

        for (const auto neighbor : preced_neighb_set) {
            max_u = std::max(max_u, u[neighbor]);
        }
        u[i] = std::min(1 + max_u, lb);
    }

    // remaining values bounded by k
    for (uint64_t i = lb; i < ordered_g.size(); i++) {
        uint64_t max_u = 0;
        const custom_bitset preced_neighb_set = ordered_g.get_neighbor_set(i, i);

        for (const auto neighbor : preced_neighb_set) {
            max_u = std::max(max_u, u[neighbor]);
        }
        u[i] = std::min(1 + max_u, k);
    }

    for (uint64_t i = K_max.count(); i < ordered_g.size(); ++i) {
        //std::cout << "i: " << i << std::endl;
        const custom_bitset V = ordered_g.get_neighbor_set(i , i);

        // first lb vertices of V
        custom_bitset P(ordered_g.size());
        uint64_t count = 0;
        for (const auto v : V) {
            P.set_bit(v);
            count++;
            if (count == lb) break;
        }

        auto B = V - P;

        custom_bitset K(ordered_g.size());
        K.set_bit(i);

        FindMaxClique(ordered_g, K, K_max, lb, V, P, B, u);
        u[i] = lb;
    }

    return ordered_g.convert_back_set(K_max);
}
