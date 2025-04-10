//
// Created by benia on 10/04/25.
//

#pragma once
#include <chrono>
#include <random>

#include "custom_bitset.h"
#include "custom_graph.h"

inline std::pair<custom_bitset, bool> TS(const custom_graph& g, std::vector<uint64_t>& swap_mem, custom_bitset S, const uint64_t k, const uint64_t L, uint64_t& Iter) {
    // TODO: implement tabu list
    uint64_t I = 0; // iterations
    custom_bitset S_max = S;
    std::vector<uint64_t> tabu_list(g.size());

    // TODO: move random generator out and pass it to everything
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<> real_dist(0, 1);
    std::uniform_int_distribution<> int_dist(0, UINT32_MAX);

    // TODO: instead of generate everything every loop, we can update the values
    while (I < L) {
        uint64_t old_S_edges = 0;
        custom_bitset S_neg = ~S;
        std::vector<uint64_t> A;
        std::vector<uint64_t> B;
        uint64_t MinInS = 0;
        uint64_t MaxOutS = 0;
        std::vector<uint64_t> d(g.size());
        for (uint64_t v = 0; v < g.size(); v++) {
            d[v] = (g.get_neighbor_set(v) & S).n_set_bits();
            // removed tabu_list check, moved later on the code
            if (S.get_bit(v)) {
                if (d[v] < MinInS) {
                    MinInS = d[v];
                    A.clear();
                }
                A.emplace_back(v);
            }
            else if (S_neg.get_bit(v)) {
                if (d[v] > MaxOutS) {
                    MaxOutS = d[v];
                    B.clear();
                }
                B.emplace_back(v);
            }
            old_S_edges += d[v]; //we sum every vertex degree
        }
        old_S_edges /= 2; // we divide by 2 (we count 2 times)

        std::vector<std::pair<uint64_t, uint64_t>> T;

        for (auto u : A) {
            for (auto v : B) {
                if (!g.get_neighbor_set(u).get_bit(v)) {
                    T.emplace_back(u, v);
                }
            }
        }

        uint64_t u = 0;
        uint64_t v = 0;
        uint64_t delta = 0;

        if (!T.empty()) {
            std::tie(u, v) = T[int_dist(rng) & T.size()];
            delta = d[v] - d[u];
        } else {
            // if no better solution is found, we need to blacklist nodes in the tabu_list
            do {
                u = A[int_dist(rng) & A.size()];
            } while (tabu_list[u] <= I);
            do {
                v = B[int_dist(rng) & B.size()];
            } while (tabu_list[v] <= I);
            delta = d[v] - d[u] - 1;
        }

        if (delta <= 0) {
            // probabilistic diversifying move selection rule
            // with
            uint64_t l = k*(k-1)/2 - old_S_edges;
            const double p = std::min(static_cast<double>((l + 2)/g.size()), 0.1);
            const double random = real_dist(rng);
            if (random <= p) {
                auto rand_num = int_dist(rng) % S.n_set_bits();

                // select u at random from S
                u = S.first_bit();
                for (uint64_t i = 1; i < rand_num; i++) {
                    u = S.next_bit();
                }

                // select v from V\S such that d[v] < integer part of k*p
                v = S_neg.first_bit();
                while (v != S_neg.size()) {
                    if (d[v] < static_cast<uint64_t>(k*g.get_density())) break;
                    v = S_neg.next_bit();
                }
            }
        }

        S.unset_bit(u);
        S.set_bit(v);
        swap_mem[u]++;
        swap_mem[v]++;

        auto S_edges = g.get_subgraph_edges(S);

        // TODO: we consider old or updated S for l1 calculation?
        auto l1 = k*(k-1)/2 - S_edges;
        auto l = std::min(l1, 10UL);
        uint64_t C = std::max(k/40, 6UL);

        tabu_list[u] = l + (int_dist(rng)%C);
        tabu_list[v] = 0.6*l + (int_dist(rng)%static_cast<uint64_t>(0.6 * C));

        // update tabu list


        // If S is a legal k clique
        if (S_edges == k*(k-1)/2) { // legal k clique
            return {S, true};
        }
        Iter++;

        if (S_edges > g.get_subgraph_edges(S_max)) {
            S_max = S;
            I = 0;
        } else {
            I++;
        }
    }

    return {S_max, false};
}

inline custom_bitset AMTS(const custom_graph& g, const uint64_t k, const uint64_t L, const uint64_t Iter_max) {
    std::vector<uint64_t> swap_mem(g.size());
    custom_bitset S(g.size());
    custom_bitset S_max(g.size());
    uint64_t Iter = 0;
    while (Iter < Iter_max) {
        auto bool legal_k_clique = false;
        std::tie(S_max, legal_k_clique) = TS(g, swap_mem, S, k, L, Iter);
        if (legal_k_clique) return S_max;

        //else
        S.unset_all();
        auto least_frequent = std::distance(swap_mem.begin(),std::min(swap_mem.begin(), swap_mem.end()));
        S.set_bit(least_frequent);

        // TODO: can improve?
        for (int i = 1; i < k; i++) {
            auto S_neg = ~S;
            std::vector<uint64_t> candidates(g.size());
            uint64_t OutMaxEdge = 0;

            auto v = S_neg.first_bit();
            while (v != S_neg.size()) {
                auto v_edges = (g.get_neighbor_set(v) & S).n_set_bits();
                if (v_edges > OutMaxEdge) {
                    OutMaxEdge = v_edges;
                    candidates.clear();
                }
                candidates.push_back(v);

                v = S_neg.next_bit();
            }

            auto v_min = std::ranges::min_element(candidates.begin(), candidates.end(), [&swap_mem](const uint64_t a, const uint64_t b) {
                return swap_mem[a] < swap_mem[b];
            });

            S.set_bit(*v_min);
        }
    }

    if (*std::min(swap_mem.begin(), swap_mem.end()) > k) {
        std::ranges::fill(swap_mem, 0);
    }
}

inline custom_bitset run_AMTS(const custom_graph& g) {

}
