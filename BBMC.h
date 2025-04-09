//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include "custom_bitset.h"
class custom_graph;

void BB_Color(const custom_graph& g, custom_bitset Ubb, std::vector<uint64_t>& Ul, std::vector<uint64_t>& C, int64_t k_min=0);
void BBMC(const custom_graph& g, custom_bitset& Ubb, std::vector<std::vector<uint64_t>>& Ul, std::vector<std::vector<uint64_t>>& C, custom_bitset& S, custom_bitset& S_max, uint64_t depth=0);
custom_bitset run_BBMC(const custom_graph &g, custom_bitset Ubb);
custom_bitset run_BBMC(const custom_graph &g);
custom_bitset run_BBMC_file(const std::string& filename);
