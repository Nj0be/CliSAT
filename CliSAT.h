//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <numeric>
#include <vector>
#include "custom_bitset.h"
#include "custom_graph.h"

// TODO: k_min should be calculated (in BBMaxClique)
// TODO: Ubb has different order than g... bitwise operation can't be done -> I don't think
// TODO: Ck is a list or a bitset??

// ok
inline void BBColor(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, const uint64_t k_min=0) {
    custom_bitset Qbb(Ubb);

    for (auto k = 0; Ubb; ++k) {
        /*auto d = g.convert_back_set(static_cast<std::vector<uint64_t>>(Ubb));
        for (auto vert : d) std::cout << vert+1 << " ";
        std::cout << std::endl;*/
        // Ck bitset or vector??
        //custom_bitset Ck(Ubb.size());
        std::vector<uint64_t> Ck;

        auto v = Qbb.first_bit_destructive();
        while (v != Qbb.size()) {

            // TODO: delete print
            /*auto a = g.convert_back_set(static_cast<std::vector<uint64_t>>(g.get_anti_neighbor_set(v)));
            std::cout << g.convert_back_set({v})[0]+1 << " -> ";
            for (auto vert : a) std::cout << vert+1 << " ";
            std::cout << std::endl;*/


            //std::cout << v << std::endl;
            Ck.push_back(v);
            // this or Ubb -= Ck ?
            //Ubb.unset_bit(vertex);
            /*auto b = g.convert_back_set(static_cast<std::vector<uint64_t>>(Qbb));
            std::cout << g.convert_back_set({v})[0]+1 << " -> ";
            for (auto vert : b) std::cout << vert+1 << " ";
            std::cout << std::endl;*/

            Qbb &= g.get_anti_neighbor_set(v);

            /*auto c = g.convert_back_set(static_cast<std::vector<uint64_t>>(Qbb));
            std::cout << g.convert_back_set({v})[0]+1 << " -> ";
            for (auto vert : c) std::cout << vert+1 << " ";
            std::cout << std::endl;*/

            if (k >= k_min) {
                C[v] = k;
            }

            //get next vertex
            v = Qbb.next_bit_destructive();
        }

        // this or above? need to check
        Ubb -= custom_bitset(Ck);
        Qbb = Ubb;
        if (k >= k_min) {
            // TODO: assignment or append?
            //Ul = Ck; // in the same order
            Ul.insert(Ul.end(), Ck.begin(), Ck.end());// in the same order
        }
        /*auto d = g.convert_back_set(static_cast<std::vector<uint64_t>>(Ck));
        for (auto vert : d) std::cout << vert+1 << " ";
        std::cout << std::endl;*/
    }
    /* std::cout << "Ul!" << std::endl;
    for (auto v : Ul) std::cout << v+1 << " ";
    std::cout << std::endl; */
}

inline void BB_Max_Clique(const custom_graph& g, custom_bitset& Ubb, std::vector<uint64_t> Ul, std::vector<uint64_t>& C, std::vector<uint64_t>& S, std::vector<uint64_t>& S_max) {
    // branching set

    /*for (auto v : g.convert_back_set(Ul)) std::cout << v+1 << " ";
    std::cout << std::endl;*/

    while (!Ul.empty()) {
        const auto v = Ul.back();
        Ul.pop_back();
        Ubb.unset_bit(v);

        if (S.size() + C[v] > S_max.size()) {
            S.push_back(v);
            // TODO: optimize candidates calculation?
            auto candidates = Ubb & g.get_neighbor_set(v, Ubb);
            if (candidates) {
                std::vector<uint64_t> C1(g.size());
                std::vector<uint64_t> Ul1;
                /*auto d = g.convert_back_set(static_cast<std::vector<uint64_t>>(candidates));
                for (auto vert : d) std::cout << vert+1 << " ";
                std::cout << std::endl;*/
                auto k_min = S_max.size() - S.size() + 1;
                BBColor(g, candidates, Ul1, C1, k_min);
                // TODO: order candidates based on coloring
                BB_Max_Clique(g, candidates, Ul1, C1, S, S_max);
            } else if (S.size() > S_max.size()) {
                // to save resources we simply swap the two
                // TODO: optimize assignment
                S_max = S;
                //std::cout << S_max.size() << std::endl;
            }

            S.pop_back();
        }
        //Ul.pop_back();
    }
}

inline std::vector<uint64_t> run_BB_Max_Clique(const custom_graph& g) {
    // initialize Ubb
    custom_bitset Ubb(g.size(), 1);

    // initialize Ul
    //std::vector<uint64_t> Ul(g.size());
    //std::iota(Ul.begin(), Ul.end(), 0);
    std::vector<uint64_t> Ul;

    // max branching set
    std::vector<uint64_t> S;
    std::vector<uint64_t> S_max;

    // coloring
    std::vector<uint64_t> C(g.size());
    // error?
    BBColor(g, Ubb, Ul, C);

    /*for (auto v : C) std::cout << v << " ";
    std::cout << std::endl;*/

    BB_Max_Clique(g, Ubb, Ul, C, S, S_max);

    return g.convert_back_set(S_max);
}
