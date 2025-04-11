//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <vector>
#include <iostream>
#include "custom_graph.h"

// k_min can be negative! int and not uint. It causes bugs
inline void BB_Color(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, const int64_t k_min=0) {
    static custom_bitset Qbb(g.size());
    for (int64_t k = 0; Ubb; ++k) {
        Qbb = Ubb;
        auto v = Qbb.first_bit();

        while (v != Qbb.size()) {
            // al piu' posso togliere, quindi non serve iniziare di nuovo un'altra scansione
            Qbb -= g.get_neighbor_set(v);

            if (k >= k_min) {
                C[v] = k;
                Ul.push_back(v);
            }

            //get next vertex
            v = Qbb.next_bit();
        }
        Ubb -= Qbb;
    }
}

inline void BBMC(const custom_graph& g, custom_bitset& Ubb, std::vector<std::vector<uint64_t>>& Ul, std::vector<std::vector<uint64_t>>& C, custom_bitset& S, custom_bitset& S_max, const uint64_t depth=0) {
    while (!Ul[depth].empty()) {
        const auto v = Ul[depth].back();
        Ul[depth].pop_back();

        Ubb.unset_bit(v);

        const auto S_bits = S.n_set_bits() + 1;
        const auto S_max_bits = S_max.n_set_bits();

        if (S_bits + C[depth][v] > S_max_bits) {
            S.set_bit(v);

            if (auto candidates = Ubb & g.get_neighbor_set(v)) {
                Ul[depth+1].clear();
                const int64_t k_min = S_max_bits - S_bits;

                BB_Color(g, candidates, Ul[depth+1], C[depth+1], k_min);

                BBMC(g, candidates, Ul, C, S, S_max, depth+1);
            } else if (S_bits > S_max_bits) { // if there are no more candidates (leaf) check if we obtained a max clique
                S_max = S;
                //std::cout << S_max.n_set_bits() << std::endl;
            }

            S.unset_bit(v);
        }
    }
}

inline custom_bitset run_BBMC(const custom_graph &g, custom_bitset Ubb) {
    // initialize Ul
    std::vector<std::vector<uint64_t>> Ul(g.size());

    // max branching set
    custom_bitset S(g.size());
    custom_bitset S_max(g.size());

    // coloring
    std::vector C(g.size(), std::vector<uint64_t>(g.size()));

    BB_Color(g, Ubb, Ul[0], C[0]);

    BBMC(g, Ubb, Ul, C, S, S_max);

    return g.convert_back_set(S_max);
}

inline custom_bitset run_BBMC(const custom_graph &g) {
    return run_BBMC(g, custom_bitset(g.size(), true));
}
