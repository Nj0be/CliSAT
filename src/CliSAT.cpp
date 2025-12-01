//
// Created by benia on 15/09/2025.
//

#include <vector>
#include <string>
#include <chrono>
#include <print>
#include <iostream>
#include <numeric>

#include "custom_graph.h"
#include "custom_bitset.h"
#include "CliSAT.h"
#include "sorting.h"
#include "AMTS.h"
#include "parsing.h"
#include "solution.h"

std::vector<int> CliSAT_no_sorting(const custom_graph& G, thread_pool<Solver>& pool, const custom_bitset& Ubb, const std::chrono::milliseconds time_limit) {
    //auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     AMTS Tabu search
    auto max_time = std::chrono::steady_clock::now() + time_limit;
    solution<int> K_max;
    fixed_vector<int> K(G.size());
    K_max.clear();
    K.clear();

    K_max.push_back(Ubb.front());
    int lb = static_cast<int>(K_max.size());

    // u with default value 1 (minimum)
    static std::vector u(G.size(), 1);

    // first |k_max| values bounded by |K_max| (==lb)
    for (auto i = 1; i < lb; i++) {
        for (const auto neighbor : G.get_prev_neighbor_set(i)) {
            u[i] = std::max(u[i], 1 + u[neighbor]);
        }
        u[i] = std::min(u[i], lb);
    }

    // remaining values bounded by k
    // TODO: why it's necessary??
    for (std::size_t i = lb; i < G.size(); i++) {
        u[i] = 1;
    }

    for (auto i : Ubb) {
        if (std::chrono::steady_clock::now() > max_time) {
            break;
        }
        lb = K_max.size();

        static custom_bitset B(G.size());
        static custom_bitset P(G.size());

        custom_bitset::AND(B, G.get_neighbor_set(i), Ubb, i);
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

        size_t local_u_idx = pool.borrow_u();
        std::vector<int>& local_u = pool.get_u(local_u_idx);
        for (int j = 0; j < u.size(); j++) local_u[j] = u[j];

        size_t alpha_idx = pool.borrow_alpha();
        fixed_vector<int>& alpha = pool.get_alpha(alpha_idx);

        pool.submit(0, [local_u_idx, alpha_idx, &G, &K_max, &pool, &K, &local_u, max_time, &alpha](Solver& solver, const size_t sequence) {
            solver.FindMaxClique(G, K, K_max, P, B, local_u, max_time, pool, sequence, alpha);
            pool.give_back_u(local_u_idx);
            pool.give_back_alpha(alpha_idx);
        });
        pool.wait_until_idle();
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
std::vector<int> CliSAT(const std::string& filename, const std::chrono::milliseconds time_limit, const bool MISP, const SORTING_METHOD sorting_method, const bool AMTS_enabled, const size_t threads) {
    auto begin = std::chrono::steady_clock::now();
    custom_graph G = parse_dimacs_extended(filename, MISP);
    thread_pool<Solver> pool(G.size(), threads);
    auto end = std::chrono::steady_clock::now();
    auto seconds_double = std::chrono::duration<double, std::chrono::seconds::period>(end - begin).count();
    std::cout << "Parsing = " << seconds_double << "[s]" << std::endl;
    std::cout << "N: " << G.size() << " M: " << G.get_n_edges() << " D: " << G.get_density() << std::endl;

    std::vector<std::size_t> ordering(G.size());
    begin = std::chrono::steady_clock::now();

    switch (sorting_method) {
        case NO_SORT:
            std::iota(ordering.begin(), ordering.end(), 0);
            break;
        case NEW_SORT:
            ordering = new_sort(G, pool);
            G.change_order(ordering);
            break;
        case DEG_SORT:
            ordering = deg_sort(G);
            G.change_order(ordering);
            break;
        case COLOUR_SORT:
            ordering = colour_sort(G, pool).first;
            G.change_order(ordering);
            break;
        default:
            return {};
            break;
    }

    auto begin_CliSAT = std::chrono::steady_clock::now();
    auto max_time = std::chrono::steady_clock::now() + time_limit;

    solution<int> K_max;
    fixed_vector<int> K(G.size());

    if (AMTS_enabled) {
        K_max = run_AMTS(G); // lb <- |K|    ->     AMTS Tabu search
        std::cout << "AMTS found clique of size " << K_max.size() << std::endl;
    } else {
        K_max.push_back(0);
    }

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
    int lb = static_cast<int>(K_max.size());

    // u with default value 1 (minimum)
    std::vector u(G.size(), 1);

    // first |k_max| values bounded by |K_max| (==lb)
    for (auto i = 1; i < lb; i++) {
        for (const auto neighbor : G.get_prev_neighbor_set(i)) {
            u[i] = std::max(u[i], 1 + u[neighbor]);
        }
        u[i] = std::min(u[i], lb);
    }

    end = std::chrono::steady_clock::now();
    seconds_double = std::chrono::duration<double, std::chrono::seconds::period>(end - begin).count();
    std::cout << "Preprocessing = " << seconds_double << "[s]" << std::endl;

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

    for (std::size_t i = lb; i < G.size(); ++i) {
        if (std::chrono::steady_clock::now() > max_time) {
            std::cout << "Exit on timeout" << std::endl;
            break;
        }

        begin = std::chrono::steady_clock::now();
        static custom_bitset B(G.size());
        static custom_bitset P(G.size());

        custom_bitset::BEFORE(B, G.get_neighbor_set(i), i);
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

        size_t local_u_idx = pool.borrow_u();
        std::vector<int>& local_u = pool.get_u(local_u_idx);
        for (int j = 0; j < u.size(); j++) local_u[j] = u[j];

        size_t alpha_idx = pool.borrow_alpha();
        fixed_vector<int>& alpha = pool.get_alpha(alpha_idx);

        pool.submit(0, [local_u_idx, alpha_idx, &G, &K_max, &pool, &K, &local_u, max_time, &alpha](Solver& solver, const size_t sequence) {
            solver.FindMaxClique(G, K, K_max, P, B, local_u, max_time, pool, sequence, alpha);
            pool.give_back_u(local_u_idx);
            pool.give_back_alpha(alpha_idx);
        });
        pool.wait_until_idle();
        K.pop_back();

        // u[i] = lb
        u[i] = K_max.size();

        end = std::chrono::steady_clock::now();
        // std::print("{}/{} (max {}) {}ms -> {} steps {} pruned\n", i+1, g.size(), K_max.size(), std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count(), steps-old_steps, pruned-old_pruned);
        std::print("{}/{} (max {}) {}ms -> {} steps {} pruned\n", i+1, G.size(), K_max.size(), std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count(), steps-old_steps, pruned-old_pruned);
    }
    auto end_CliSAT = std::chrono::steady_clock::now();
    std::cout << "Branching time: " << std::chrono::duration<double, std::chrono::seconds::period>(end_CliSAT - begin_CliSAT).count() << " [s]" << std::endl;

    std::cout << "Steps: " << steps << std::endl;
    std::cout << "Pruned: " << pruned << std::endl;

    if (!is_clique(G, custom_bitset(std::vector<int>(K_max), G.size()))) {
        std::cout << "Error: wrong solution (" << custom_bitset(std::vector<int>(G.convert_back_set(K_max, ordering))) << ")" << std::endl;
        exit(1);
    }

    return G.convert_back_set(K_max, ordering);
}
