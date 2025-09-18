//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <cstdint>
#include <vector>

#include "custom_bitset.h"
#include "custom_graph.h"

inline void BB_Color(const custom_graph& g, custom_bitset Ubb, std::vector<std::uint64_t>& Ul, std::vector<std::uint64_t>& C, const std::int64_t k_min=0) {
    static custom_bitset Qbb(g.size());
    for (std::int64_t k = 0; Ubb.any(); ++k) {
        Qbb = Ubb;

        for (const auto v : Qbb) {
            // at most we can remove vertices, so we don't need to start a new scan
            Qbb -= g.get_neighbor_set(v);

            if (k >= k_min) {
                C[v] = k;
                Ul.push_back(v);
            }
        }
        Ubb -= Qbb;
    }
}

inline void BBMC(const custom_graph& g, custom_bitset& Ubb, std::vector<std::vector<std::uint64_t>>& Ul,
                std::vector<std::vector<std::uint64_t>>& C, custom_bitset& S, custom_bitset& S_max,
                const std::uint64_t depth=0) {
    while (!Ul[depth].empty()) {
        const auto v = Ul[depth].back();
        Ul[depth].pop_back();

        Ubb.reset(v);

        const auto S_bits = S.count() + 1;
        const auto S_max_bits = S_max.count();

        if (S_bits + C[depth][v] > S_max_bits) {
            S.set(v);

            auto candidates = Ubb & g.get_neighbor_set(v);
            if (candidates.any()) {
                Ul[depth+1].clear();
                const std::int64_t k_min = S_max_bits - S_bits;

                BB_Color(g, candidates, Ul[depth+1], C[depth+1], k_min);

                BBMC(g, candidates, Ul, C, S, S_max, depth+1);
            } else if (S_bits > S_max_bits) {
                S_max = S;
            }

            S.reset(v);
        }
    }
}

inline custom_bitset run_BBMC(const custom_graph &g, custom_bitset Ubb) {
    // initialize Ul
    std::vector<std::vector<std::uint64_t>> Ul(g.size());

    // max branching set
    custom_bitset S(g.size());
    custom_bitset S_max(g.size());

    // coloring
    std::vector C(g.size(), std::vector<std::uint64_t>(g.size()));

    BB_Color(g, Ubb, Ul[0], C[0]);

    BBMC(g, Ubb, Ul, C, S, S_max);

    return g.convert_back_set(S_max);
}

inline custom_bitset run_BBMC(const custom_graph &g) {
    return run_BBMC(g, custom_bitset(g.size(), true));
}
