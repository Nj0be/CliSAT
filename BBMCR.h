//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <vector>
#include "custom_bitset.h"
#include "custom_graph.h"

// k_min can be negative! int and not uint. It causes bugs
inline void BB_ColorR(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, const int64_t k_min=0) {
    custom_bitset Qbb(g.size());
    for (auto k = 0; Ubb; ++k) {
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

inline void BBMCR(const custom_graph& g, custom_bitset& Ubb, std::vector<uint64_t>& Ul, const std::vector<uint64_t>& C, custom_bitset& S, custom_bitset& S_max) {
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

                BB_ColorR(g, candidates, Ul1, C1, k_min);

                BBMCR(g, candidates, Ul1, C1, S, S_max);
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

    BB_ColorR(g, Ubb, Ul, C);

    BBMCR(g, Ubb, Ul, C, S, S_max);

    return g.convert_back_set(S_max);
}