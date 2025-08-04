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
        auto v = Qbb.first_bit();

        while (v != Qbb.size()) {
            // al piu' posso togliere, quindi non serve iniziare di nuovo un'altra scansione
            Qbb -= g.get_neighbor_set(v);

            //get next vertex
            v = Qbb.next_bit();
        }
        // add vertices to pruned
        pruned |= Qbb;
        Ubb -= Qbb;
    }
    return pruned;
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
    // initialize u -> for pruned vertices, we use the father initialization

    // branching set
    //custom_bitset B = V - P;
    auto bi = B.first_bit();
    while (bi != B.size()) {

        // calculate u[bi]
        // if bi == 0, u[bi] always == 1!
        custom_bitset preced_neighb_set(bi, true);
        preced_neighb_set &= G.get_neighbor_set(bi, V);

        uint64_t max_u = 0;
        auto preced_neighb = preced_neighb_set.first_bit();
        while (preced_neighb != preced_neighb_set.size()) {
            max_u = std::max(max_u, u[preced_neighb]);
            preced_neighb = preced_neighb_set.next_bit();
        }
        u[bi] = 1 + max_u;


        if (u[bi] + K.n_set_bits() <= lb) {
            P.set_bit(bi);
            B.unset_bit(bi);
        } else {
            K.set_bit(bi);
            auto V_new = (P & G.get_neighbor_set(bi)) | (B & preced_neighb_set);

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
        bi = B.next_bit();
    }
}

inline custom_bitset CliSAT(const custom_graph& g) {
    auto [ordering, k] = NEW_SORT(g);
    auto ordered_g = g.change_order(ordering);

    auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     ANTS Tabu search
    uint64_t lb = K_max.n_set_bits();

    // TODO: make a function to handle u!
    std::vector<uint64_t> u(ordered_g.size());
    // first |k_max| values bounded by |K_max| (==lb)
    u[0] = 1;
    for (uint64_t i = 1; i < lb; i++) {
        uint64_t max_u = 0;
        custom_bitset preced_neighb_set(i, true);
        preced_neighb_set &= ordered_g.get_neighbor_set(i);
        auto preced_neighb = preced_neighb_set.first_bit();
        while (preced_neighb != preced_neighb_set.size()) {
            max_u = std::max(max_u, u[preced_neighb]);
            preced_neighb = preced_neighb_set.next_bit();
        }
        u[i] = std::min(1 + max_u, lb);
    }
    // remaining values bounded by k
    for (uint64_t i = lb; i < ordered_g.size(); i++) {
        uint64_t max_u = 0;
        custom_bitset preced_neighb_set(i, true);
        preced_neighb_set &= ordered_g.get_neighbor_set(i);
        auto preced_neighb = preced_neighb_set.first_bit();
        while (preced_neighb != preced_neighb_set.size()) {
            max_u = std::max(max_u, u[preced_neighb]);
            preced_neighb = preced_neighb_set.next_bit();
        }
        u[i] = std::min(1 + max_u, k);
    }

    for (uint64_t i = K_max.n_set_bits(); i < ordered_g.size(); ++i) {
        std::cout << "i: " << i << std::endl;
        custom_bitset V(i, true);
        V &= ordered_g.get_neighbor_set(i);
        // K_max initial or updated?

        // first lb vertices of V
        custom_bitset P(ordered_g.size());
        auto v = V.first_bit();
        while (v != V.size() && P.n_set_bits() < lb) {
            P.set_bit(v);
            v = V.next_bit();
        }

        auto B = V - P;

        custom_bitset K(ordered_g.size());
        K.set_bit(i);

        FindMaxClique(ordered_g, K, K_max, lb, V, P, B, u);
        u[i] = lb;
    }

    return ordered_g.convert_back_set(K_max);
}
