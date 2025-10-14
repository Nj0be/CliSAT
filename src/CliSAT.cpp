//
// Created by benia on 15/09/2025.
//

#include <vector>
#include <string>
#include <chrono>
#include <print>
#include <numeric>
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

// MISP indicates if the program needs to resolve the maximum independent set problem (1)
// sorting can be:
//  - 0: no sorting
//  - 1: NEW_SORT
//  - 2: DEG_SORT
//  - 3: COLOUR_SORT
std::vector<int> CliSAT(const std::string& filename, const bool MISP, const int sorting_method) {
    auto begin = std::chrono::steady_clock::now();
    custom_graph g(filename);
    if (MISP) g.complement();
    auto end = std::chrono::steady_clock::now();
    auto seconds_double = std::chrono::duration<double, std::chrono::seconds::period>(end - begin).count();
    std::cout << "Parsing = " << seconds_double << "[s]" << std::endl;
    std::cout << "N: " << g.size() << " M: " << g.get_n_edges() << " D: " << g.get_density() << std::endl;

    std::vector<std::size_t> ordering(g.size());
    begin = std::chrono::steady_clock::now();

    switch (sorting_method) {
        case 0:
            std::iota(ordering.begin(), ordering.end(), 0);
            break;
        case 1:
            ordering = NEW_SORT(g);
            g.change_order(ordering);
            break;
        case 2:
            ordering = DEG_SORT(g);
            g.change_order(ordering);
            break;
        case 3:
            ordering = COLOUR_SORT(g).first;
            g.change_order(ordering);
            break;
        default:
            return {};
            break;
    }

    end = std::chrono::steady_clock::now();
    seconds_double = std::chrono::duration<double, std::chrono::seconds::period>(end - begin).count();
    std::cout << "Preprocessing = " << seconds_double << "[s]" << std::endl;

    auto begin_CliSAT = std::chrono::steady_clock::now();

    //auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     AMTS Tabu search
    std::vector<int> K_max;
    std::vector<int> K;

    /*
    for (int i = 0; i < g.size(); i++) {
        const auto neighb = g.get_neighbor_set(i).front();
        if (neighb != custom_bitset::npos) {
            K_max.push_back(i);
            K_max.push_back(neighb);
            break;
        }
    }
    */
    K_max.push_back(0);
    int lb = static_cast<int>(K_max.size());

    // u with default value 1 (minimum)
    std::vector u(g.size(), 1);

    // first |k_max| values bounded by |K_max| (==lb)
    for (auto i = 1; i < lb; i++) {
        for (const auto neighbor : g.get_prev_neighbor_set(i)) {
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

    for (std::size_t i = lb; i < g.size(); ++i) {
        begin = std::chrono::steady_clock::now();
        static custom_bitset B(g.size());
        static custom_bitset P(g.size());

        custom_bitset::BEFORE(B, g.get_neighbor_set(i), i);
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
        auto old_pruned = pruned;

        K.push_back(i);
        FindMaxClique(g, K, K_max, P, B, u);
        K.pop_back();

        // u[i] = lb
        u[i] = K_max.size();

        end = std::chrono::steady_clock::now();
        std::print("{}/{} (max {}) {}ms -> {} steps {} pruned\n", i+1, g.size(), K_max.size(), std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count(), steps-old_steps, pruned-old_pruned);
    }
    auto end_CliSAT = std::chrono::steady_clock::now();
    std::cout << "Branching time: " << std::chrono::duration<double, std::chrono::seconds::period>(end_CliSAT - begin_CliSAT).count() << " [s]" << std::endl;

    std::cout << "Steps: " << steps << std::endl;
    std::cout << "Pruned: " << pruned << std::endl;
    //std::cout << custom_bitset(K_max) << std::endl;

    return g.convert_back_set(K_max, ordering);
}
