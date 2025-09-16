//
// Created by benia on 15/09/2025.
//

#include <vector>
#include <string>
#include <chrono>
#include <print>
#include "custom_graph.h"
#include "CliSAT.h"

#include "sorting.h"

std::vector<int> CliSAT_no_sorting(const custom_graph& g, const custom_bitset& Ubb) {
    //auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     AMTS Tabu search
    static std::vector<int> K_max;
    static std::vector<int> K;
    K_max.clear();
    K.clear();

    K_max.push_back(Ubb.front());
    int lb = static_cast<int>(K_max.size());

    // u with default value 1 (minimum)
    static std::vector u(g.size(), 1);

    // first |k_max| values bounded by |K_max| (==lb)
    for (auto i = 1; i < lb; i++) {
        for (const auto neighbor : g.get_prev_neighbor_set(i)) {
            u[i] = std::max(u[i], 1 + u[neighbor]);
        }
        u[i] = std::min(u[i], lb);
    }

    // remaining values bounded by k
    // TODO: why it's necessary??
    for (std::size_t i = lb; i < g.size(); i++) {
        u[i] = 1;
    }

    for (auto i : Ubb) {
        lb = K_max.size();

        static custom_bitset B(g.size());
        static custom_bitset P(g.size());

        custom_bitset::AND(B, g.get_neighbor_set(i), Ubb, i);
        //if (B.count() <= lb) continue;
        P.reset();

        // we pruned first lb vertices of V (they can't improve the solution on they own)
        // if we set count to zero, we can't possibly improve the solution because the B set can become empty even tough
        // it should be possible to improve, lb vertices + 1 from k, but we remove every one from the lb ones
        auto count = 1;
        for (const auto v : B) {
            if (count == lb) break;
            B.reset(v);
            P.set(v);
            count++;
        }

        K.push_back(i);
        FindMaxClique(g, K, K_max, P, B, u);
        K.pop_back();

        // u[i] = lb
        u[i] = K_max.size();
    }

    return K_max;
}

std::vector<int> CliSAT(const std::string& filename) {
    const custom_graph g(filename);
    auto [ordering, k] = NEW_SORT(g, 3);
    auto ordered_g = g.change_order(ordering);

    //auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     AMTS Tabu search
    std::vector<int> K_max;
    std::vector<int> K;

    K_max.push_back(0);
    int lb = static_cast<int>(K_max.size());

    // u with default value 1 (minimum)
    std::vector u(g.size(), 1);

    // first |k_max| values bounded by |K_max| (==lb)
    for (auto i = 1; i < lb; i++) {
        for (const auto neighbor : ordered_g.get_prev_neighbor_set(i)) {
            u[i] = std::max(u[i], 1 + u[neighbor]);
        }
        u[i] = std::min(u[i], lb);
    }

    // remaining values bounded by k
    // TODO: why it's necessary??
    /*
    for (std::size_t i = lb; i < ordered_g.size(); i++) {
        for (const auto neighbor : ordered_g.get_prev_neighbor_set(i)) {
            u[i] = std::max(u[i], 1 + u[neighbor]);
        }
        u[i] = std::min(u[i], k);
    }
    */

    steps = 0;
    pruned = 0;

    auto begin_branch = std::chrono::steady_clock::now();
    for (std::size_t i = lb; i < ordered_g.size(); ++i) {
        auto begin = std::chrono::steady_clock::now();
        static custom_bitset B(g.size());
        static custom_bitset P(g.size());

        custom_bitset::BEFORE(B, ordered_g.get_neighbor_set(i), i);
        P.reset();

        lb = K_max.size();

        // we pruned first lb vertices of V (they can't improve the solution on they own)
        // if we set count to zero, we can't possibly improve the solution because the B set can become empty even tough
        // it should be possible to improve, lb vertices + 1 from k, but we remove every one from the lb ones
        auto count = 1;
        for (const auto v : B) {
            if (count == lb) break;
            B.reset(v);
            P.set(v);
            count++;
        }

        auto old_steps = steps;

        K.push_back(i);
        FindMaxClique(ordered_g, K, K_max, P, B, u);
        K.pop_back();

        // u[i] = lb
        u[i] = K_max.size();

        auto end = std::chrono::steady_clock::now();
        std::print("{}/{} (max {}) {}ms -> {} steps\n", i+1, ordered_g.size(), K_max.size(), std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count(), steps-old_steps);
    }

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin_branch).count() << "ms" << std::endl;

    std::cout << "Steps: " << steps << std::endl;
    std::cout << "Pruned: " << pruned << std::endl;
    //std::cout << custom_bitset(K_max) << std::endl;

    return ordered_g.convert_back_set(K_max);
}
