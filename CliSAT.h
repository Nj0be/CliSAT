//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <vector>
#include "custom_bitset.h"
#include "custom_graph.h"

// TODO: k_min should be calculated (in BBMaxClique)
// TODO: Ubb has different order than g... bitwise operation can't be done -> I don't think
// TODO: Ck is a list or a bitset??

// ok

// TODO: k_min can be negative! int and not uint. It causes bugs
inline void BBColor(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, const int64_t k_min=0) {
    // TODO: K should start from 1 and not from 0!
    for (auto k = 1; Ubb; ++k) {
        custom_bitset Qbb(Ubb);
        // TODO: read in reverse?? It gives wrong results otherwise
        // no way, it's too slow in reverse
        auto v = Qbb.first_bit_destructive();
        while (v != Qbb.size()) {
            // this or Ubb -= Ck ?
            Ubb.unset_bit(v);

            // al piu' posso togliere, quindi non serve iniziare di nuovo un'altra scansione
            Qbb &= g.get_anti_neighbor_set(v);

            if (k >= k_min) {
                C[v] = k;
                Ul.push_back(v);
            }

            //get next vertex
            v = Qbb.next_bit_destructive();
        }
    }
}

inline void BB_Max_Clique(const custom_graph& g, custom_bitset& Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, custom_bitset& S, custom_bitset& S_max) {
    while (!Ul.empty()) {
        const auto v = Ul.back();
        Ul.pop_back();

        Ubb.unset_bit(v);

        if (S.n_set_bits() + C[v] > S_max.n_set_bits()) {
            S.set_bit(v);
            // TODO: optimize candidates calculation?
            auto candidates = Ubb & g.get_neighbor_set(v);
            if (candidates) {
                // TODO: C or C1? Maybe C works because BBColor touches only the candidates that are not used by this iteration
                // Nope
                // TODO: improve C1 creation performance
                std::vector<uint64_t> C1(g.size());
                std::vector<uint64_t> Ul1;
                Ul1.reserve(g.size());
                int64_t k_min = S_max.n_set_bits() - S.n_set_bits() + 1;
                BBColor(g, candidates, Ul1, C1, k_min);
                //BBColor(g, candidates, Ul1, C, k_min);
                // TODO: order candidates based on coloring
                BB_Max_Clique(g, candidates, Ul1, C1, S, S_max);
                //BB_Max_Clique(g, candidates, Ul1, C, S, S_max);
            } else if (S.n_set_bits() > S_max.n_set_bits()) { // if there are no more candidates (leaf) check if we obtained a max clique
                // to save resources we simply swap the two
                // TODO: optimize assignment
                S_max = S;
                std::cout << S_max.n_set_bits() << std::endl;
            }

            S.unset_bit(v);
        }
    }
}

inline std::vector<uint64_t> run_BB_Max_Clique(const custom_graph& g) {
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

    BBColor(g, Ubb, Ul, C);

    BB_Max_Clique(g, Ubb, Ul, C, S, S_max);

    return g.convert_back_set(S_max);
}


class BB_Max_Clique_cl {
    custom_graph g;
    custom_bitset S;
    custom_bitset S_max;
    custom_bitset Qbb;
    std::vector<uint64_t> C;
    custom_bitset Ubb;


    void iter_BBColor(custom_bitset Ubb, std::vector<uint64_t>& Ul, const int64_t k_min=0) {

        for (auto k = 0; Ubb; ++k) {
            Qbb = Ubb;

            auto v = Qbb.first_bit_destructive();
            while (v != Qbb.size()) {
                // this or Ubb -= Ck ?
                Ubb.unset_bit(v);

                Qbb &= g.get_anti_neighbor_set(v);

                if (k >= k_min) {
                    C[v] = k;
                    Ul.push_back(v);
                }

                //get next vertex
                v = Qbb.next_bit_destructive();
            }
        }
    }

    void iter_BB_Max_Clique(custom_bitset &Ubb, std::vector<uint64_t> &Ul) {
        // branching set

        while (!Ul.empty()) {
            const auto v = Ul.back();
            Ul.pop_back();
            Ubb.unset_bit(v);

            if (S.n_set_bits() + C[v] > S_max.n_set_bits()) {
                S.set_bit(v);
                // TODO: optimize candidates calculation?
                auto candidates = Ubb & g.get_neighbor_set(v);
                if (candidates) {
                    // TODO: C or C1? Maybe C works because BBColor touches only the candidates that are not used by this iteration
                    //std::vector<uint64_t> C1(g.size());
                    std::vector<uint64_t> Ul1;
                    Ul1.reserve(g.size());
                    int64_t k_min = S_max.n_set_bits() - S.n_set_bits() + 1;
                    //BBColor(g, candidates, Ul1, C1, k_min);
                    iter_BBColor(candidates, Ul1, k_min);
                    // TODO: order candidates based on coloring
                    //BB_Max_Clique(g, candidates, Ul1, C1, S, S_max);
                    iter_BB_Max_Clique(candidates, Ul1);
                } else if (S.n_set_bits() > S_max.n_set_bits()) {
                    // to save resources we simply swap the two
                    // TODO: optimize assignment
                    S_max = S;
                    //std::cout << S_max.n_set_bits() << std::endl;
                }

                S.unset_bit(v);
            }
        }
    }

public:
    explicit BB_Max_Clique_cl(std::string filename) : g(filename), S(g.size()), S_max(g.size()), Qbb(g.size()), C(g.size()), Ubb(g.size(), 1) {}

    std::vector<uint64_t> run() {
        // initialize Ul
        std::vector<uint64_t> Ul;
        // pre-allocate list size
        Ul.reserve(g.size());

        iter_BBColor(Ubb, Ul);

        iter_BB_Max_Clique(Ubb, Ul);

        return g.convert_back_set(S_max);
    }
};
