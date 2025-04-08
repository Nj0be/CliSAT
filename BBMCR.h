//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <vector>
#include "custom_bitset.h"
#include "custom_graph.h"

inline bool BB_ReCol(const custom_graph& g, const uint64_t v, std::vector<uint64_t>& C, std::vector<custom_bitset>& C_sets, const int64_t k_min) {
    // we use k < k_min because in the last iteration we could do a single swap anyway
    // it clearly doesn't enter the second for (with k2) because k1+1 == k_min -> exit
    for (int64_t k1 = 0; k1 < k_min; ++k1) {
        auto inters = C_sets[k1] & g.get_neighbor_set(v);
        const auto w = inters.first_bit();
        // if the intersection between Ck1 and N(v) = 0 then we can put v in Ck1
        if (w == inters.size()) { // empty set - single swap
            // Ck1 = Ck1 U v
            C[v] = k1;
            C_sets[k1].set_bit(v);
            return true;
        }
        // if the intersection between |Ck1 and N(v)| = 1 then we could search another set where to put w (the only vertex adjacent to v)
        if (inters.next_bit() == inters.size()) {  // |set| = 1 -> double swap
            for (int64_t k2 = k1+1; k2 < k_min; ++k2) {
                // if the intersection between Ck2 and N(w) = 0 then we can put w in Ck2 and v in Ck1
                if (!(C_sets[k2] & g.get_neighbor_set(w))) {
                    // Ck1 = (Ck1 \ w) U v
                    C[v] = k1;
                    C_sets[k1].unset_bit(w);
                    C_sets[k1].set_bit(v);
                    // Ck2 = Ck2 U w
                    C[w] = k2;
                    C_sets[k2].set_bit(w);
                    return true;
                }
            }
        }
    }
    return false;
}

inline void BB_ColorR(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, std::vector<custom_bitset>& C_sets, const int64_t k_min=0) {
    for (int64_t k = 0; Ubb;) {
        C_sets[k] = Ubb;
        auto v = C_sets[k].first_bit();

        while (v != C_sets[k].size()) {
            C_sets[k] -= g.get_neighbor_set(v);

            //prefetch next vertex to check if the current one is the last
            const auto next_v = C_sets[k].next_bit();

            if (k >= k_min) {
                // if v is the last element remaining to color
                if (next_v == C_sets[k].size() && BB_ReCol(g, v, C, C_sets, k)) break;
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
        if (v != C_sets[k].size()) C_sets[k].unset_bit(v);
        // if, after the above operation, C_sets[k] remains empty, we don't increase k
        if (C_sets[k]) ++k;
    }
}

inline void BBMCR(const custom_graph& g, custom_bitset& Ubb, std::vector<uint64_t>& Ul, const std::vector<uint64_t>& C, std::vector<custom_bitset>& C_sets, custom_bitset& S, custom_bitset& S_max) {
    while (!Ul.empty()) {
        const auto v = Ul.back();
        Ul.pop_back();

        Ubb.unset_bit(v);

        if (S.n_set_bits() + C[v] >= S_max.n_set_bits()) {
            S.set_bit(v);

            if (auto candidates = Ubb & g.get_neighbor_set(v)) {
                std::vector<uint64_t> C1(g.size());
                std::vector<uint64_t> Ul1;
                const int64_t k_min = S_max.n_set_bits() - S.n_set_bits();

                BB_ColorR(g, candidates, Ul1, C1, C_sets, k_min);

                BBMCR(g, candidates, Ul1, C1, C_sets, S, S_max);
            } else if (S.n_set_bits() > S_max.n_set_bits()) { // if there are no more candidates (leaf) check if we obtained a max clique
                S_max = S;
                std::cout << S_max.n_set_bits() << std::endl;
            }

            S.unset_bit(v);
        }
    }
}

inline std::vector<uint64_t> run_BBMCR(const custom_graph& g) {
    // initialize Ubb
    custom_bitset Ubb(g.size(), 1);

    // initialize Ul
    std::vector<uint64_t> Ul;
    // pre-allocate list size
    Ul.reserve(g.size());

    // max branching set
    custom_bitset S(g.size());
    custom_bitset S_max(g.size());

    // coloring
    std::vector<uint64_t> C(g.size());
    std::vector<custom_bitset> C_sets;
    C_sets.reserve(g.size());
    for (uint64_t i=0; i < g.size(); i++) C_sets.emplace_back(g.size());

    BB_ColorR(g, Ubb, Ul, C, C_sets);

    BBMCR(g, Ubb, Ul, C, C_sets, S, S_max);

    return g.convert_back_set(S_max);
}