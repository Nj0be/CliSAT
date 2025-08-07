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
        for (BitCursor cursor = Qbb.first_bit(); cursor.get_pos() != Qbb.size(); cursor = Qbb.next_bit(cursor)) {
            // at most we can remove vertices, so we don't need to start a new scan
            Qbb -= g.get_neighbor_set(cursor.get_pos());
        }
        // add vertices to pruned
        pruned |= Qbb;
        Ubb -= Qbb;
    }
    return pruned;
}

inline bool UnitPropagation(
    const custom_graph& G,
    const custom_bitset& Pc,    // pruned set that satisfy the condition: |K|+UP(Pc) <= lb (the greatest clique of K union P can't be greater than the lower bound)
    const custom_bitset& B      // branching sets used to enlarge the pruned set
) {
    return false;
    //ISEQ_sets
}


// Page 8 (143) Li et al. (2018a, 2017), section 5.2.1
// return a set B of branching vertices
inline custom_bitset FilterByColoring(
    const custom_graph& G,      // Ordered graph
    custom_bitset& P,          // Candidate set {p1, p2, ..., p|P|}
    const uint64_t r          // lower bound (largest clique found so far)
) {
    custom_bitset B(P.size());  // B, a set of branching vertices obtained from P
    std::vector<custom_bitset> IS;  // set of independent sets

    for (BitCursor cursor = P.last_bit_destructive(); cursor.get_pos() != P.size(); cursor = P.prev_bit_destructive(cursor)) {
        // Implementation goes here
    }
    return B;
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
    for (BitCursor cursor = B.first_bit(); cursor.get_pos() != B.size(); cursor = B.next_bit(cursor)) {
        const auto bi = cursor.get_pos();

        const custom_bitset bi_preced_neighbor_set = G.get_neighbor_set(bi, V, bi);

        // calculate u[bi]
        // if bi == 0, u[bi] always == 1!
        if (bi == 0) {
            u[bi] = 1;
        } else {
            uint64_t max_u = 0;
            for (BitCursor neighb_cursor = bi_preced_neighbor_set.first_bit();
                 neighb_cursor.get_pos() != bi_preced_neighbor_set.size();
                 neighb_cursor = bi_preced_neighbor_set.next_bit(neighb_cursor)) {
                max_u = std::max(max_u, u[neighb_cursor.get_pos()]);
                 }
            u[bi] = 1 + max_u;
        }


        if (u[bi] + K.n_set_bits() <= lb) {
            P.set_bit(bi);
            B.unset_bit(bi);
        } else {
            K.set_bit(bi);
            auto V_new = (P & G.get_neighbor_set(bi)) | (B & bi_preced_neighbor_set);

            if (!V_new) {
                if (K.n_set_bits() > lb) {
                    lb = K.n_set_bits();
                    K_max = K;
                    std::cout << lb << std::endl;
                }
                K.unset_bit(bi);
                // TODO: continue or return?
                return;
            }
            auto P_new = ISEQ_pruned(G, V_new, lb-K.n_set_bits());
            auto B_new = V_new - P_new;

            // TODO
            // PMAX-SAT-P-based upper bounds
            // FiltCOL
            // FiltSAT
            // SATCOL

            if (B_new) {
                FindMaxClique(G, K, K_max, lb, V_new, P_new, B_new, u);
            }
            K.unset_bit(bi);
        }
        // TODO: here or above?

        u[bi] = std::min(u[bi], lb - K.n_set_bits());
    }
}

inline custom_bitset CliSAT(const custom_graph& g) {
    auto [ordering, k] = NEW_SORT(g);
    const auto ordered_g = g.change_order(ordering);

    auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     ANTS Tabu search
    uint64_t lb = K_max.n_set_bits();

    std::vector<uint64_t> u(ordered_g.size());
    // first |k_max| values bounded by |K_max| (==lb)
    u[0] = 1;
    for (uint64_t i = 1; i < lb; i++) {
        uint64_t max_u = 0;
        const custom_bitset preced_neighb_set = ordered_g.get_neighbor_set(i, i);

        for (BitCursor cursor = preced_neighb_set.first_bit();
             cursor.get_pos() != preced_neighb_set.size();
             cursor = preced_neighb_set.next_bit(cursor)) {
            max_u = std::max(max_u, u[cursor.get_pos()]);
        }
        u[i] = std::min(1 + max_u, lb);
    }

    // remaining values bounded by k
    for (uint64_t i = lb; i < ordered_g.size(); i++) {
        uint64_t max_u = 0;
        const custom_bitset preced_neighb_set = ordered_g.get_neighbor_set(i, i);

        for (BitCursor cursor = preced_neighb_set.first_bit();
             cursor.get_pos() != preced_neighb_set.size();
             cursor = preced_neighb_set.next_bit(cursor)) {
            max_u = std::max(max_u, u[cursor.get_pos()]);
        }
        u[i] = std::min(1 + max_u, k);
    }

    for (uint64_t i = K_max.n_set_bits(); i < ordered_g.size(); ++i) {
        std::cout << "i: " << i << std::endl;
        const custom_bitset V = ordered_g.get_neighbor_set(i , i);

        // first lb vertices of V
        custom_bitset P(ordered_g.size());
        uint64_t count = 0;
        for (BitCursor cursor = V.first_bit(); cursor.get_pos() != V.size() && count < lb; cursor = V.next_bit(cursor)) {
            P.set_bit(cursor.get_pos());
            count++;
        }

        auto B = V - P;

        custom_bitset K(ordered_g.size());
        K.set_bit(i);

        FindMaxClique(ordered_g, K, K_max, lb, V, P, B, u);
        u[i] = lb;
    }

    return ordered_g.convert_back_set(K_max);
}
