module;

#include <set>
#include <vector>
#include <iostream>
#include <print>
#include <chrono>

export module CliSAT;

import AMTS;
import custom_bitset;
import custom_graph;
import sorting;
import coloring;

// TODO: implement
inline void SATCOL() {

}

inline bool FiltCOL(
    const custom_graph& G,  // graph
    custom_bitset& V, // vertices set
    const std::vector<custom_bitset>& ISs,
    std::vector<custom_bitset>& ISs_t,
    std::vector<std::size_t>& alpha,
    const int k_max,
    const int orig_size
) {
    static custom_bitset Ubb(G.size());
    Ubb= V;
    //auto numbers = std::ranges::iota_view{0, k_max} | std::ranges::to<std::vector>();

    for (auto i = 0; i < k_max; ++i) {
        auto v = Ubb.front();
        if (v == custom_bitset::npos) return false;

        int k = -1;
        for (int j = 0; j < orig_size; ++j) {
            if (ISs[j].test(v)) {
                k = j;
                break;
            }
        }

        //ISs_t[i] = Ubb - G.get_neighbor_set(v);
        custom_bitset::SUB(Ubb, G.get_neighbor_set(v), ISs_t[i]);
        Ubb.reset(v);

        auto last_v = v;
        for (v = ISs_t[i].next(v); v != custom_bitset::npos; v = ISs_t[i].next(v)) {
            // if v isn't part of the original IS
            if (!ISs[k].test(v)) {
                // if v is greater than every other vertices in the original IS
                if (v > alpha[k]) {
                    // removed from V
                    V.reset(v);
                    // removed from future iterations
                    Ubb.reset(v);
                }
                // otherwise kept for next iterations
            } else {
                last_v = v;
                // at most, we can remove vertices, so we don't need to start a new scan
                ISs_t[i] -= G.get_neighbor_set(v);
                Ubb.reset(v);
            }
        }
        alpha[k] = last_v;
    }
    return true;
}

inline bool FiltSAT(
    const custom_graph& G,  // graph
    custom_bitset& V, // vertices set
    std::vector<custom_bitset>& ISs_t,
    std::vector<std::size_t>& alpha,
    const int k_max
) {
    // we apply FL (Failed Literal) to every vertex of each IS of Ct-alpha
    //for (int i = 0; i < k_max; ++i) {
    for (int i = k_max-1; i >= 0; --i) {
        for (auto u : ISs_t[i]) {
            auto is_connected = true;

            // check if u is not connected to another is
            //for (int j = 0; j < k_max; ++j) {
            for (int j = k_max-1; j >= 0; --j) {
                if (i == j) continue;
                is_connected = G.get_neighbor_set(u).intersects(ISs_t[j]);
                if (!is_connected) break;
            }
            if (!is_connected) {
                ISs_t[i].reset(u);
                if (ISs_t[i].none()) {
                    return false;
                }
                V.reset(u);
            } else {
                // update alpha with the biggest node not removed so far
                alpha[i] = u;
            }
        }
    }
    return true;
}

uint64_t steps = 0;
uint64_t pruned = 0;

// TODO: P pass by reference or not? I don't think so
// we pass u by copy, not reference!
inline void FindMaxClique(
    const custom_graph& G,  // graph
    custom_bitset& K,       // current branch
    const int curr,           // lower bound
    custom_bitset& K_max,   // max branch
    int& lb,           // lower bound
    const custom_bitset& V, // vertices set
    custom_bitset& B,       // branching set
    // should not be a reference?
    std::vector<int> u, // incremental upper bounds,
    // TODO: pass by reference or value?
    //std::vector<std::size_t>& alpha,
    const std::vector<std::size_t> &alpha = {},
    const int is_k_partite = 0
) {
    static std::vector<std::vector<custom_bitset>> ISs(G.size(), std::vector<custom_bitset>(G.size(), custom_bitset(G.size())));
    static custom_bitset prev_neighb_set(G.size());
    custom_bitset V_new(G.size());
    custom_bitset B_new(G.size());
    steps++;

    //auto V_copy = V;
    //auto P = V - B;
    for (const auto bi : B) {
        // if bi == 0, u[bi] always == 1!
        u[bi] = 1;
        // TODO: G.get_prev_neighbor_set(bi, V_copy) wrong because we are removing elements from V_copy
        custom_bitset::get_prev_neighbor_set(G.get_neighbor_set(bi), V, bi, prev_neighb_set);
        //for (const auto neighbor : G.get_prev_neighbor_set(bi, V)) {
        for (const auto neighbor : prev_neighb_set) {
            u[bi] = std::max(u[bi], 1+u[neighbor]);
            if (u[bi] + curr > lb) break;
        }

        // if we can't improve, we prune the current branch
        if (u[bi] + curr <= lb) {
            pruned++;
            //P.set(bi);
            B.reset(bi);
        } else {
            // equivalent to (P & N(bi)) | prev_neighbor(bi, V)
            //auto V_new = ((P & G.get_neighbor_set(bi)) | G.get_prev_neighbor_set(bi, V));

            // if we are in a leaf
            //if (V_new.none()) {
            if (!custom_bitset::calculate_subproblem(V, B, G.get_neighbor_set(bi), bi, V_new)) {
                // update best solution
                // TODO: inside or outside?
                if (curr > lb) {
                    lb = curr;
                    K.set(bi);
                    K_max = K;
                    K.reset(bi);
                    //std::cout << "New solution: " << lb << std::endl;
                }

                return;
            }

            const int k = lb-curr;
            /*
            if (k < 0) {
                std::cout << "cipolle" << std::endl;
                K.set(bi);
                FindMaxClique(G, K, curr+1, K_max, lb, V_new, V_new, u);
                K.reset(bi);
                continue;
            }
            */

            int next_is_k_partite = is_k_partite;
            std::vector<std::size_t> new_alpha = alpha;

            // if is a k+1 partite graph
            if (is_k_partite) {
                const auto depth = is_k_partite - k;
                // TODO: return or continue?
                if (!FiltCOL(G, V_new, ISs[0], ISs[depth], new_alpha, k+1, is_k_partite)) return;
                if (!FiltSAT(G, V_new, ISs[depth], new_alpha, k+1)) return;
                B_new = ISEQ_branching(G, V_new, k);
                //B_new = ISs[depth][k];
            } else {
                B_new = ISEQ_branching(G, V_new, ISs[0], new_alpha, k);
                //if (B_new.none()) continue;
                if (is_IS(G, B_new)) {
                    ISs[0][k] = B_new;
                    new_alpha[k] = B_new.back();
                    next_is_k_partite = k+1;
                    // if we could return here, huge gains... damn
                    if (!FiltSAT(G, V_new, ISs[0], new_alpha, k+1)) continue;
                    B_new = ISEQ_branching(G, V_new, k);
                } else {
                    SATCOL();
                }
            }

            //B_new = ISEQ_branching(G, V_new, k);

            // if B is not none
            if (!B_new.none()) {
                K.set(bi);
                FindMaxClique(G, K, curr+1, K_max, lb, V_new, B_new, u, new_alpha, next_is_k_partite);
                K.reset(bi);
            }
        }
        u[bi] = std::min(u[bi], lb-curr);
    }
}

export inline custom_bitset CliSAT(const custom_graph& g) {
    auto [ordering, k] = NEW_SORT(g, 3);
    const auto ordered_g = g.change_order(ordering);

    //auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     AMTS Tabu search
    custom_bitset K_max(g.size());
    custom_bitset K(g.size());
    custom_bitset V(g.size());
    custom_bitset B(g.size());

    K_max.set(0);
    int lb = static_cast<int>(K_max.count());

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
    for (std::size_t i = lb; i < ordered_g.size(); i++) {
        for (const auto neighbor : ordered_g.get_prev_neighbor_set(i)) {
            u[i] = std::max(u[i], 1 + u[neighbor]);
        }
        u[i] = std::min(u[i], k);
    }

    for (std::size_t i = lb; i < ordered_g.size(); ++i) {
        auto begin = std::chrono::steady_clock::now();
        custom_bitset::get_prev_neighbor_set(ordered_g.get_neighbor_set(i), custom_bitset::reference(i), V);
        B = V;

        K.set(i);

        // we pruned first lb vertices of V (they can't improve the solution on they own)
        B.clear_before(lb-1);

        FindMaxClique(ordered_g, K, 2, K_max, lb, V, B, u);
        K.reset(i);
        u[i] = lb;

        auto end = std::chrono::steady_clock::now();
        std::print("{}/{} (max {}) {}ms\n", i+1, ordered_g.size(), K_max.count(), std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
    }

    std::cout << "Steps: " << steps << std::endl;
    std::cout << "Pruned: " << pruned << std::endl;

    return ordered_g.convert_back_set(K_max);
}
