//
// Created by Beniamino Vagnarelli on 03/04/25.
//

module;

#include <vector>
#include <cstdint>
#include <iostream>

export module BBMCR;

import custom_graph;
import custom_bitset;

inline bool BB_ReCol(const custom_graph& g, const uint64_t v, std::vector<custom_bitset>& C_sets, const int64_t k_min) {
    static custom_bitset inters(g.size());

    // we use k < k_min because in the last iteration we could do a single swap anyway
    // it clearly doesn't enter the second for (with k2) because k1+1 == k_min -> exit
    for (int64_t k1 = 0; k1 < k_min; ++k1) {
        // c = a; c &= b; is faster than assign c = (a & b) !!!
        // less memory copy
        inters = C_sets[k1];
        inters &= g.get_neighbor_set(v);
        auto w = inters.front();
        // if the intersection between Ck1 and N(v) = 0 then we can put v in Ck1
        if (w == custom_bitset::npos) { // none set - single swap
            // Ck1 = Ck1 U v
            //C[v] = k1;
            C_sets[k1].set(v);
            return true;
        }
        // if the intersection between |Ck1 and N(v)| = 1 then we could search another set where to put w (the only vertex adjacent to v)
        if (inters.next(w) == custom_bitset::npos) {  // |set| = 1 -> double swap
            for (int64_t k2 = k1+1; k2 < k_min; ++k2) {
                inters = C_sets[k2];
                inters &= g.get_neighbor_set(w);
                // if the intersection between Ck2 and N(w) = 0 then we can put w in Ck2 and v in Ck1
                if (inters.none()) {
                    // Ck1 = (Ck1 \ w) U v
                    //C[v] = k1;
                    C_sets[k1].reset(w);
                    C_sets[k1].set(v);
                    // Ck2 = Ck2 U w
                    //C[w] = k2;
                    C_sets[k2].set(w);
                    return true;
                }
            }
        }
    }
    return false;
}

inline void BB_ColorR(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, const int64_t k_min=0) {
    static std::vector C_sets(g.size(), custom_bitset(g.size()));

    for (int64_t k = 0; Ubb.any();) {
        C_sets[k] = Ubb;
        auto cursor = C_sets[k].front();

        while (cursor != custom_bitset::npos) {
            C_sets[k] -= g.get_neighbor_set(cursor);

            //prefetch next vertex to check if the current one is the last
            const auto next_cursor = C_sets[k].next(cursor);

            if (k >= k_min) {
                // if v is the last element remaining to color
                if (next_cursor == custom_bitset::npos && BB_ReCol(g, cursor, C_sets, k_min)) break;
                C[cursor] = k;
                Ul.push_back(cursor);
            }

            cursor = next_cursor;
        }
        // C_sets[k] contains every colored vertex
        Ubb -= C_sets[k];

        // if we recolored v (we didn't run v = next that equals C_sets[k].size()), we remove v from current set
        // indeed v isn't part of current color (has been recolored)
        // Cnew = Cnew \ v
        if (cursor != custom_bitset::npos) {
            C_sets[k].reset(cursor);
            // if, after the above operation, C_sets[k] remains none, we don't increase k
            if (C_sets[k].none()) continue;
        }
        // otherwise we increase as usual
        ++k;
    }
}

inline void BBMCR(const custom_graph& g, custom_bitset& Ubb, std::vector<std::vector<uint64_t>>& Ul, std::vector<std::vector<uint64_t>>& C, custom_bitset& S, custom_bitset& S_max, const uint64_t depth=0) {
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
                const int64_t k_min = S_max_bits - S_bits;

                BB_ColorR(g, candidates, Ul[depth+1], C[depth+1], k_min);

                BBMCR(g, candidates, Ul, C, S, S_max, depth+1);
            } else if (S_bits > S_max_bits) { // if there are no more candidates (leaf) check if we obtained a max clique
                S_max = S;
                std::cout << S_max.count() << std::endl;
            }

            S.reset(v);
        }
    }
}

export inline custom_bitset run_BBMCR(const custom_graph& g, custom_bitset Ubb) {
    // initialize Ul
    std::vector<std::vector<uint64_t>> Ul(g.size());

    // max branching set
    custom_bitset S(g.size());
    custom_bitset S_max(g.size());

    // coloring
    std::vector C(g.size(), std::vector<uint64_t>(g.size()));

    BB_ColorR(g, Ubb, Ul[0], C[0]);

    BBMCR(g, Ubb, Ul, C, S, S_max);

    return g.convert_back_set(S_max);
}

export inline custom_bitset run_BBMCR(const custom_graph &g) {
    return run_BBMCR(g, custom_bitset(g.size(), true));
}
