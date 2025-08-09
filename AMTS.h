//
// Created by Beniamino Vagnarelli on 10/04/25.
//

#pragma once
#include <chrono>
#include <random>

#include "custom_bitset.h"
#include "custom_graph.h"

inline std::pair<custom_bitset, bool> TS(const custom_graph& g, std::vector<uint64_t>& swap_mem, custom_bitset S, const uint64_t k, const uint64_t L, uint64_t& Iter, const std::chrono::time_point<std::chrono::steady_clock> max_time) {
    uint64_t I = 0; // iterations
    custom_bitset S_max = S;
    std::vector<uint64_t> tabu_list(g.size());

    // TODO: move random generator out and pass it to everything
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<> real_dist(0, 1);
    std::uniform_int_distribution<> int_dist(0, INT32_MAX);

    // TODO: instead of generate everything every loop, we can update the values
    while (I < L) {
        if (std::chrono::steady_clock::now() > max_time) break;

        uint64_t old_S_edges = 0;
        custom_bitset S_neg = ~S;
        std::vector<uint64_t> A;
        std::vector<uint64_t> A_without_tabu;
        std::vector<uint64_t> B;
        std::vector<uint64_t> B_without_tabu;
        uint64_t MinInS = 0;
        uint64_t MaxOutS = 0;
        std::vector<uint64_t> d(g.size());
        for (uint64_t v = 0; v < g.size(); v++) {
            d[v] = (g.get_neighbor_set(v) & S).count();
            // removed tabu_list check, moved later on the code
            if (S[v]) {
                if (d[v] < MinInS) {
                    MinInS = d[v];
                    A.clear();
                    A_without_tabu.clear();
                }
                A.emplace_back(v);
                if (tabu_list[v] <= I) A_without_tabu.emplace_back(v);
            }
            else if (S_neg[v]) {
                if (d[v] > MaxOutS) {
                    MaxOutS = d[v];
                    B.clear();
                    B_without_tabu.emplace_back(v);
                }
                B.emplace_back(v);
                if (tabu_list[v] <= I) B_without_tabu.emplace_back(v);
            }
            old_S_edges += d[v]; //we sum every vertex degree
        }
        old_S_edges /= 2; // we divide by 2 (we count 2 times)

        std::vector<std::pair<uint64_t, uint64_t>> T;

        for (auto u : A) {
            for (auto v : B) {
                if (!g.get_neighbor_set(u)[v]) {
                    T.emplace_back(u, v);
                }
            }
        }

        int64_t u = 0;
        int64_t v = 0;
        int64_t delta = 0;

        if (!T.empty()) {
            std::tie(u, v) = T[int_dist(rng) % T.size()];
            delta = d[v] - d[u];
        } else {
            //TODO: in small graphs it can happen
            if (A_without_tabu.empty() || B_without_tabu.empty()) {
                Iter++;
                I++;
                continue;
            }
            // if no better solution is found, we need to blacklist nodes in the tabu_list
            u = A_without_tabu[int_dist(rng) % A_without_tabu.size()];
            v = B_without_tabu[int_dist(rng) % B_without_tabu.size()];
            delta = d[v] - d[u] - 1;
        }

        if (delta <= 0) {
            uint64_t l = k*(k-1)/2 - old_S_edges;
            const double p = std::min(static_cast<double>((l + 2)/g.size()), 0.1);
            const double random = real_dist(rng);
            if (random <= p) {
                auto rand_num = int_dist(rng) % S.count();

                // select u at random from S
                auto new_u = S.front();
                for (uint64_t i = 1; i < rand_num; i++) {
                    new_u = S.next(new_u);
                }

                // select v from V\S such that d[v] < integer part of k*p
                auto new_v = S_neg.front();
                while (new_v != S_neg.end()) {
                    if (d[*new_v] < static_cast<uint64_t>(k*g.get_density())) break;
                    new_v = S_neg.next(new_v);
                }
                if (new_v != S_neg.end()) {
                    u = *new_u;
                    v = *new_v;
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

        tabu_list[u] = I + l + (int_dist(rng)%C);
        tabu_list[v] = I + 0.6*l + (int_dist(rng)%static_cast<uint64_t>(0.6 * C));

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

inline std::pair<custom_bitset, bool> AMTS(const custom_graph& g, const uint64_t k, const uint64_t L, const uint64_t Iter_max, const std::chrono::time_point<std::chrono::steady_clock> max_time) {
    std::vector<uint64_t> swap_mem(g.size());
    custom_bitset S(g.size());

    // construct initial Solution
    for (uint64_t i = 0; i < k; i++) {
        auto S_neg = ~S;
        uint64_t OutMaxEdge = 0;

        auto v = S_neg.front();
        auto selected_v = v;
        while (v != S_neg.end()) {
            auto v_edges = (g.get_neighbor_set(*v) & S).count();
            if (v_edges > OutMaxEdge) {
                OutMaxEdge = v_edges;
                selected_v = v;
            }

            v = S_neg.next(v);
        }

        S.set_bit(selected_v);
    }

    custom_bitset S_max(S);
    uint64_t Iter = 0;
    while (Iter < Iter_max) {
        if (std::chrono::steady_clock::now() > max_time) break;

        bool is_legal_k_clique = false;
        std::tie(S_max, is_legal_k_clique) = TS(g, swap_mem, S, k, L, Iter, max_time);
        if (is_legal_k_clique) return {S_max, true};

        //else
        S.reset();
        auto least_frequent = std::distance(swap_mem.begin(),std::min(swap_mem.begin(), swap_mem.end()));
        S.set_bit(least_frequent);

        // TODO: can improve?
        for (uint64_t i = 1; i < k; i++) {
            auto S_neg = ~S;
            std::vector<uint64_t> candidates;
            uint64_t OutMaxEdge = 0;

            for (const auto v : S_neg) {
                auto v_edges = (g.get_neighbor_set(v) & S).count();
                if (v_edges > OutMaxEdge) {
                    OutMaxEdge = v_edges;
                    candidates.clear();
                }
                candidates.push_back(v);
            }

            auto v_min = std::ranges::min_element(candidates.begin(), candidates.end(),
                [&swap_mem](const uint64_t a, const uint64_t b) {
                    return swap_mem[a] < swap_mem[b];
                });

            S.set_bit(*v_min);
        }
        if (*std::min(swap_mem.begin(), swap_mem.end()) > k) {
            std::ranges::fill(swap_mem, 0);
        }
    }

    return {S_max, false};
}

inline custom_bitset run_AMTS(const custom_graph& g, int64_t run_time=50) {
    auto max_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(run_time);
    // TODO: get complement for p < 0.5
    uint64_t Iter_Max = 100000000;
    custom_bitset S_max(g.size());
    custom_bitset S(g.size());
    for (uint64_t k = 1; k < g.size(); k++) {
        if (std::chrono::steady_clock::now() > max_time) break;

        uint64_t L = g.size() * k;
        // if brock or san L = 4 * k;
        bool is_legal_k_clique = false;
        std::tie(S, is_legal_k_clique) = AMTS(g, k, L, Iter_Max, max_time);
        if (!is_legal_k_clique) return S_max;
        S_max = S;
    }
    return S_max;
}
