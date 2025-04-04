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
inline std::vector<uint64_t> BBColor(const custom_graph& g, custom_bitset& Ubb, std::vector<uint64_t>& Ul, const uint64_t k_min) {
    std::vector<uint64_t> coloring(Ubb.size());
    custom_bitset Qbb(Ubb);

    for (auto k = 0; Ubb; ++k) {
        custom_bitset Ck(Ubb.size());

        auto vertex = Qbb.first_bit_destructive();
        while (vertex != Qbb.size()) {
            Ck.set_bit(vertex);
            // this or Ubb -= Ck ?
            //Ubb.unset_bit(vertex);
            Qbb &= g.get_anti_neighbor_set(vertex);

            //get next vertex
            vertex = Qbb.next_bit_destructive();

            if (k > k_min) {
                coloring[vertex] = k;
            }
        }

        // this or above? need to check
        Ubb -= Ck;
        Qbb = Ubb;
        if (k > k_min) {
            Ul = static_cast<std::vector<uint64_t>>(Ck);
        }
    }

    return coloring;
}

inline uint64_t CliSAT(custom_graph& graph) {
    return 0;
}
