//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <vector>
#include "custom_bitset.h"
#include "custom_graph.h"

// TODO: slower??
inline bool BB_ReCol(const custom_graph& g, const uint64_t v, std::vector<uint64_t>& C, const std::vector<custom_bitset>& C_sets, const int64_t k_min) {
    for (int64_t k1 = 0; k1 < k_min-1; ++k1) {
        auto inters = C_sets[k1] & g.get_neighbor_set(v);
        const auto w = inters.first_bit();
        if (w == inters.size()) { // empty set - single swap
            C[v] = k1;
            //C_sets[k1].set_bit(v);
            return true;
        }
        if (inters.next_bit() == inters.size()) {  // double swap
            for (int64_t k2 = k1+1; k2 < k_min; ++k2) {
                if (!(C_sets[k2] & g.get_neighbor_set(w))) {
                    C[v] = k1;
                    //C_sets[k1].set_bit(v);
                    C[w] = k2;
                    //C_sets[k2].set_bit(w);
                    return true;
                }
            }
        }
    }
    return false;
}

inline void BB_ColorR(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, std::vector<custom_bitset>& C_sets, const int64_t k_min=0) {
    custom_bitset Qbb(g.size());
    for (int64_t k = 0; Ubb; ++k) {
        C_sets[k].unset_all();

        Qbb = Ubb;
        auto v = Qbb.first_bit_destructive();

        while (v != Qbb.size()) {
            C_sets[k].set_bit(v);
            if (k >= k_min) {
                if (!Qbb && BB_ReCol(g, v, C, C_sets, k_min)) break;
                C[v] = k;
                Ul.push_back(v);
            }
            Qbb -= g.get_neighbor_set(v);

            //get next vertex
            v = Qbb.next_bit_destructive();
        }
        Ubb -= C_sets[k];
    }

    /*for (auto u:C) std::cout << u << " ";
    std::cout << std::endl;
    for (auto set:C_sets) {
        std::cout << set << std::endl;
    }
    std::cout << std::endl;*/
}

inline void BBMCR(const custom_graph& g, custom_bitset& Ubb, std::vector<uint64_t>& Ul, const std::vector<uint64_t>& C, std::vector<custom_bitset>& C_sets, custom_bitset& S, custom_bitset& S_max) {
    while (!Ul.empty()) {
        const auto v = Ul.back();
        Ul.pop_back();

        Ubb.unset_bit(v);

        if (S.n_set_bits() + C[v] >= S_max.n_set_bits()) {
            S.set_bit(v);

            if (auto candidates = Ubb & g.get_neighbor_set(v)) {
                std::vector<uint64_t> C1(g.size()+1);
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
    std::vector<uint64_t> C(g.size()+1);
    std::vector<custom_bitset> C_sets;
    C_sets.reserve(g.size());
    for (uint64_t i=0; i < g.size(); i++) C_sets.emplace_back(g.size());

    BB_ColorR(g, Ubb, Ul, C, C_sets);

    BBMCR(g, Ubb, Ul, C, C_sets, S, S_max);

    return g.convert_back_set(S_max);
}