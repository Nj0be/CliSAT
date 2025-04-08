//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <vector>
#include "custom_bitset.h"
#include "custom_graph.h"

// k_min can be negative! int and not uint. It causes bugs
inline void BB_Color(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, const int64_t k_min=0) {
    custom_bitset Qbb(g.size());
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
                std::cout << S_max.n_set_bits() << std::endl;
            }

            S.unset_bit(v);
        }
    }
}

inline std::vector<uint64_t> run_BBMC(const custom_graph& g) {
    // initialize Ubb
    custom_bitset Ubb(g.size(), 1);

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

inline std::vector<uint64_t> BB_Max_Clique_iter(const custom_graph& g) {
    std::vector<custom_bitset> Ubb;
    Ubb.emplace_back(g.size(), 1);

    // initialize Ul
    std::vector<std::vector<uint64_t>> Ul(1);
    // pre-allocate list size
    Ul.back().reserve(g.size());

    // max branching set
    custom_bitset S(g.size());
    custom_bitset S_max(g.size());

    // coloring
    std::vector<std::vector<uint64_t>> C;
    C.emplace_back(g.size());

    std::vector<uint64_t> to_remove;

    BB_Color(g, Ubb.back(), Ul.back(), C.back());

    while (!Ul.empty()) {
        if (Ul.back().empty()) {
            Ul.pop_back();
            Ubb.pop_back();
            C.pop_back();
            S.unset_bit(to_remove.back());
            to_remove.pop_back();
            continue;
        }

        const auto v = Ul.back().back();
        Ul.back().pop_back();

        Ubb.back().unset_bit(v);

        if (S.n_set_bits() + C.back()[v] >= S_max.n_set_bits()) {
            S.set_bit(v);
            Ubb.emplace_back(Ubb.back() & g.get_neighbor_set(v));
            if (Ubb.back()) {
                C.emplace_back(g.size());
                Ul.emplace_back();
                to_remove.push_back(v);
                const int64_t k_min = S_max.n_set_bits() - S.n_set_bits();

                BB_Color(g, Ubb.back(), Ul.back(), C.back(), k_min);

                continue;
            } else if (S.n_set_bits() > S_max.n_set_bits()) { // if there are no more candidates (leaf) check if we obtained a max clique
                S_max = S;
                std::cout << S_max.n_set_bits() << std::endl;
            }

            Ubb.pop_back();
            S.unset_bit(v);
        }
    }

    return g.convert_back_set(S_max);
}


class BB_Max_Clique_cl {
    const custom_graph g;
    custom_bitset S;
    custom_bitset S_max;
    custom_bitset Qbb;
    std::vector<custom_bitset> Ubb;
    std::vector<std::vector<uint64_t>> C;


    void iter_BBColor(custom_bitset Ubb, std::vector<uint64_t>& Ul, const uint64_t depth=0, const int64_t k_min=1) {
        for (auto k = 1; Ubb; ++k) {
            Qbb = Ubb;

            auto v = Qbb.first_bit();
            while (v != Qbb.size()) {
                // this or Ubb -= Ck ?
                Ubb.unset_bit(v);

                Qbb -= g.get_neighbor_set(v);

                if (k >= k_min) {
                    C[depth][v] = k;
                    Ul.push_back(v);
                }

                //get next vertex
                v = Qbb.next_bit();
            }
            Ubb -= Qbb;
        }
    }

    void iter_BB_Max_Clique(std::vector<uint64_t> &Ul, const uint64_t depth=0) {
        while (!Ul.empty()) {
            const auto v = Ul.back();
            Ul.pop_back();
            Ubb[depth].unset_bit(v);

            if (S.n_set_bits() + C[depth][v] > S_max.n_set_bits()) {
                S.set_bit(v);
                // TODO: optimize candidates calculation?
                if (Ubb.size() <= depth+1) Ubb.push_back(custom_bitset(g.size()));
                Ubb[depth+1] = Ubb[depth] & g.get_neighbor_set(v);
                if (Ubb[depth+1]) {
                    // TODO: C or C1? Maybe C works because BB_Color touches only the candidates that are not used by this iteration
                    if (C.size() <= depth+1) C.push_back(std::vector<uint64_t>(g.size()));
                    std::vector<uint64_t> Ul1;
                    // Ul1.reserve(g.size()); // harms performance
                    const int64_t k_min = S_max.n_set_bits() - S.n_set_bits() + 1;
                    //BB_Color(g, candidates, Ul1, C1, k_min);
                    iter_BBColor(Ubb[depth+1], Ul1, depth+1, k_min);
                    // TODO: order candidates based on coloring
                    //BBMC(g, candidates, Ul1, C1, S, S_max);
                    iter_BB_Max_Clique(Ul1, depth+1);
                } else if (S.n_set_bits() > S_max.n_set_bits()) {
                    // to save resources we simply swap the two
                    // TODO: optimize assignment
                    S_max = S;
                    std::cout << S_max.n_set_bits() << std::endl;
                }

                S.unset_bit(v);
            }
        }
    }

public:
    explicit BB_Max_Clique_cl(const std::string &filename) : g(filename), S(g.size()), S_max(g.size()), Qbb(g.size()) {
        C.emplace_back(g.size());
        Ubb.emplace_back(g.size(), 1);
    }

    std::vector<uint64_t> run() {
        // initialize Ul
        std::vector<uint64_t> Ul;
        // pre-allocate list size
        Ul.reserve(g.size());

        iter_BBColor(Ubb[0], Ul);

        iter_BB_Max_Clique(Ul);

        return g.convert_back_set(S_max);
    }
};
