//
// Created by benia on 08/04/25.
//

#pragma once

#include <vector>
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


inline bool BB_ReCol(const custom_graph& g, const uint64_t v, std::vector<custom_bitset>& C_sets, const int64_t k_min) {
    static custom_bitset inters(g.size());

    // we use k < k_min because in the last iteration we could do a single swap anyway
    // it clearly doesn't enter the second for (with k2) because k1+1 == k_min -> exit
    for (int64_t k1 = 0; k1 < k_min; ++k1) {
        // c = a; c &= b; is faster than assign c = (a & b) !!!
        // less memory copy
        inters = C_sets[k1];
        inters &= g.get_neighbor_set(v);
        const auto w = inters.first_bit();
        // if the intersection between Ck1 and N(v) = 0 then we can put v in Ck1
        if (w == inters.size()) { // empty set - single swap
            // Ck1 = Ck1 U v
            //C[v] = k1;
            C_sets[k1].set_bit(v);
            return true;
        }
        // if the intersection between |Ck1 and N(v)| = 1 then we could search another set where to put w (the only vertex adjacent to v)
        if (inters.next_bit() == inters.size()) {  // |set| = 1 -> double swap
            for (int64_t k2 = k1+1; k2 < k_min; ++k2) {
                inters = C_sets[k2];
                inters &= g.get_neighbor_set(w);
                // if the intersection between Ck2 and N(w) = 0 then we can put w in Ck2 and v in Ck1
                if (!inters) {
                    // Ck1 = (Ck1 \ w) U v
                    //C[v] = k1;
                    C_sets[k1].unset_bit(w);
                    C_sets[k1].set_bit(v);
                    // Ck2 = Ck2 U w
                    //C[w] = k2;
                    C_sets[k2].set_bit(w);
                    return true;
                }
            }
        }
    }
    return false;
}


inline void ISEQ(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, const int64_t k_min=0) {
    static std::vector C_sets(g.size(), custom_bitset(g.size()));

    for (int64_t k = 0; Ubb;) {
        C_sets[k] = Ubb;
        auto v = C_sets[k].first_bit();

        while (v != C_sets[k].size()) {
            C_sets[k] -= g.get_neighbor_set(v);

            //prefetch next vertex to check if the current one is the last
            const auto next_v = C_sets[k].next_bit();

            if (k >= k_min) {
                // if v is the last element remaining to color
                if (next_v == C_sets[k].size() && BB_ReCol(g, v, C_sets, k_min)) break;
                C[v] = k;
                Ul.push_back(v);
            }

            v = next_v;
        }
        // C_sets[k] contains every colored vertex
        Ubb -= C_sets[k];

        // if we recolored v (we didn't run v = next that equals C_sets[k].size()), we remove v from current set
        // indeed v isn't part of current color (has been recolored)
        // Cnew = Cnew \ v
        if (v != C_sets[k].size()) {
            C_sets[k].unset_bit(v);
            // if, after the above operation, C_sets[k] remains empty, we don't increase k
            if (!C_sets[k]) continue;
        }
        // otherwise we increase as usual
        ++k;
    }
}

// TODO: P pass by reference or not? I don't think so
// we pass u by copy, not reference!
inline void FindMaxClique(const custom_graph& G, custom_bitset V, custom_bitset& K, custom_bitset& K_max, uint64_t& lb, custom_bitset P, std::vector<uint64_t> u) {
    // initialize u -> for pruned vertices, we use the father initialization

    // branching set
    custom_bitset B = ~P & V;
    auto bi = B.first_bit();
    while (bi != B.size()) {
        // calculate u[bi]
        // if bi == 0, u[bi] always == 1!
        if (bi != 0) {
            uint64_t max_u = 0;
            custom_bitset preced_neighb_set(bi, 1);
            preced_neighb_set &= G.get_neighbor_set(bi);
            auto preced_neighb = preced_neighb_set.first_bit();
            while (preced_neighb != preced_neighb_set.size()) {
                max_u = std::max(max_u, u[preced_neighb]);
                preced_neighb = preced_neighb_set.next_bit();
            }
            u[bi] = 1 + max_u;
        }

        if (u[bi] + K.n_set_bits() <= lb) {
            P.set_bit(bi);
            B.unset_bit(bi);
        } else {
            V = (P & G.get_neighbor_set(bi));
            // if needed because empty custom bitset cannot exists
            if (bi > 0) V |=  G.get_neighbor_set(bi) & B & custom_bitset(bi, 1);

            if (!V) {
                if (K.n_set_bits() > lb) {
                    lb = K.n_set_bits();
                    K_max = K;
                }
                return;
            }
            // TODO
            // FiltCOL
            // FiltSAT
            // SATCOL

            if (B) {
                K.set_bit(bi);
                FindMaxClique(G, V, K, K_max, lb, P, u);
                K.unset_bit(bi);
            }
        }
        u[bi] = std::min(u[bi], lb - K.n_set_bits());

        bi = B.next_bit();
    }
}

inline custom_bitset CliSAT(const custom_graph& g) {
    auto [ordering, k] = NEW_SORT(g);
    auto ordered_g = g.change_order(ordering);

    // TODO: AMTS
    // K_max = FindClique(V), lb <- |K|    ->     ANTS Tabu search
    custom_bitset K_max(1);
    uint64_t lb = K_max.n_set_bits();

    // TODO: make a function to handle u!
    std::vector<uint64_t> u(ordered_g.size());
    // first |k_max| values bounded by |K_max| (==lb)
    u[0] = 1;
    for (uint64_t i = 1; i < lb; i++) {
        uint64_t max_u = 0;
        custom_bitset preced_neighb_set(i, 1);
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
        custom_bitset preced_neighb_set(i, 1);
        preced_neighb_set &= ordered_g.get_neighbor_set(i);
        auto preced_neighb = preced_neighb_set.first_bit();
        while (preced_neighb != preced_neighb_set.size()) {
            max_u = std::max(max_u, u[preced_neighb]);
            preced_neighb = preced_neighb_set.next_bit();
        }
        u[i] = std::min(1 + max_u, k);
    }


    for (uint64_t i = K_max.n_set_bits() + 1; i < g.size(); ++i) {
        custom_bitset V(i, 1);
        V &= ordered_g.get_neighbor_set(i);
        // K_max initial or updated?
        custom_bitset P(K_max.n_set_bits()+1, 1);
        custom_bitset K = custom_bitset(ordered_g.size());
        K.set_bit(i);
        FindMaxClique(ordered_g, V, K, K_max, lb, P, u);
    }

    return g.convert_back_set(K_max);
}
