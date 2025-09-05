module;

#include <vector>
#include <iostream>
#include <print>
#include <chrono>
#include <queue>
#include <stack>

export module CliSAT;

import AMTS;
import custom_bitset;
import custom_graph;
import sorting;
import coloring;

bool SATCOL(
    custom_graph& G,
    custom_bitset& B,
    std::vector<custom_bitset>& ISs,
    int k
) {
    static custom_bitset already_added(G.size());
    static custom_bitset already_visited(G.size());
    static custom_bitset conflicting_clauses(G.size());
    static custom_bitset IS_B(G.size());
    static custom_bitset neighbors(G.size());
    static custom_bitset D(G.size());

    const auto orig_size = G.size();
    conflicting_clauses.reset();

    // SATCOL iterates until:
    //  - it fails to find a conflict
    //  - B is empty
    while (true) {
        ISEQ_one(G, B, IS_B);
        auto conflict_found = false;

        for (const auto ui : IS_B) {
            std::stack<std::pair<custom_bitset::reference, int>> s;
            conflict_found = false;

            s.emplace(ui, k); // we add the current vertex ui to the stack S

            already_added.reset();
            already_visited.reset();

            neighbors.set();

            // for each Unit IS
            while (!s.empty()) {
                const auto [u, u_is] = s.top();
                s.pop();
                already_visited.set(u_is);

                // insert current node to the culprit_ISs
                conflicting_clauses.set(u_is);

                neighbors &= G.get_neighbor_set(u);

                // useless to iterate over r+1 that contains only u;
                for (auto is = 0; is < k; ++is) {
                //for (auto i = k-1; i >= 0; --i) {
                    // if D is the unit IS set that we are setting to true, we continue
                    if (is == u_is || already_visited[is]) continue;

                    // remove vertices non adjacent to u
                    custom_bitset::AND(ISs[is], neighbors, D);
                    auto di = D.front();

                    if (di == custom_bitset::npos) { // empty IS, conflict detected
                        conflicting_clauses.set(is); // we have derived an empty independent set
                        conflict_found = true;

                        // more than one conflict can be found, but it's redundant
                        /* TODO: CliSAT paper, page2, says:
                        * In the latter case, the soft clauses (2) in which a positive literal is set to
                        * true, together with the soft clause that becomes empty, determine
                        * a conflict.
                        * So it must continue to find every conflict (empty clause)
                        */
                        //break;
                    }
                    if (!already_added.test(is) && D.next(di) == custom_bitset::npos) { // Unit IS
                        already_added.set(is);
                        s.emplace(di, is);
                    }
                }

                // empty IS has been derived, we break
                if (conflict_found) break;
            }

            if (!conflict_found) break;
        }

        // TODO: break or continue?
        if (!conflict_found) break;

        ISs[k] = IS_B; // we add the unit independent set to ISs

        // conflict found!
        B -= IS_B;
        if (B.none()) break;

        k++;

        const auto first_node = G.size();

        G.resize(G.size() + conflicting_clauses.count());
        B.resize(G.size());
        neighbors.resize(G.size());
        D.resize(G.size());
        IS_B.resize(G.size());

        // k+1 for next iteration
        for (auto is = 0; is < k+1; ++is)
            ISs[is].resize(G.size());

        auto last_node = first_node;

        // we reset conflicting_clauses while scanning it
        for (auto is = conflicting_clauses.pop_front(); is != custom_bitset::npos; is = conflicting_clauses.pop_next(is)) {
            for (auto curr_is = 0; curr_is <= k; ++curr_is) {
                if (static_cast<std::size_t>(curr_is) == is) continue; // skip the current IS

                for (const auto v : ISs[curr_is]) {
                    if (v >= first_node) break; // skip new vertices
                    G.add_edge(last_node, v);
                }
            }
            // add edge also to future nodes
            for (const auto v : B)
                G.add_edge(last_node, v);

            ISs[is].set(last_node); // add the new vertex to the is
            ++last_node;
        }
    }

    G.resize(orig_size);
    B.resize(orig_size);
    neighbors.resize(orig_size);
    D.resize(orig_size);
    IS_B.resize(orig_size);

    for (auto is = 0; is < k+1; ++is)
        ISs[is].resize(orig_size);

    return B.any();
}

bool FiltCOL(
    const custom_graph& G,  // graph
    custom_bitset& V, // vertices set
    const std::vector<custom_bitset>& ISs,
    std::vector<custom_bitset>& ISs_t,
    const std::vector<int>& color_class,
    std::vector<std::size_t>& alpha,
    const int k_max
) {
    static custom_bitset Ubb(G.size());
    Ubb = V;
    //auto numbers = std::ranges::iota_view{0, k_max} | std::ranges::to<std::vector>();

    for (auto i = 0; i < k_max; ++i) {
        auto v = Ubb.front();
        if (v == custom_bitset::npos) return false;

        const int k = color_class[v];

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

bool FiltSAT(
    const custom_graph& G,  // graph
    custom_bitset& V, // vertices set
    std::vector<custom_bitset>& ISs,
    std::vector<std::size_t>& alpha,
    const int k_max
) {
    static std::vector ISs_copy(G.size(), custom_bitset(G.size()));
    static std::vector<std::pair<custom_bitset::reference, int>> S;
    static custom_bitset already_added(G.size());
    static custom_bitset already_visited(G.size());

    for (auto i = 0; i < k_max; ++i) {
        ISs_copy[i] = ISs[i];
    }

    // we apply FL (Failed Literal) to every vertex of each IS of Ct-alpha
    //for (int i = 0; i < k_max; ++i) {
    for (int i = k_max-1; i >= 0; --i) {
        for (const auto ui : ISs[i]) {
            S.clear();
            bool conflict_found = false;

            S.emplace_back(ui, k_max); // we add the current vertex ui to the stack S

            already_added.reset();
            already_visited.reset();

            // for each Unit IS
            while (!S.empty()) {
                const auto [u, u_is] = S.back();
                already_visited.set(u_is);
                S.pop_back();

                // use only with already_visited
                ISs[u_is].reset(u);

                // useless to iterate over r+1 that contains only u;
                for (auto j = 0; j < k_max; ++j) {
                    // if D is the unit IS set that we are setting to true, we continue
                    //if (i == u_is) continue;
                    if (j == i || j == u_is || already_visited[j]) continue;

                    auto& D = ISs[j];

                    // remove vertices non adjacent to u
                    D &= G.get_neighbor_set(u);
                    auto di = D.front();

                    if (di == custom_bitset::npos) { // empty IS, conflict detected
                        conflict_found = true;
                        V.reset(ui);
                        ISs[i].reset(ui);
                        ISs_copy[i].reset(ui);

                        // more than one conflict can be found, but it's redundant
                        break;
                    }
                    if (!already_added[j] && D.next(di) == custom_bitset::npos) { // Unit IS
                        already_added.set(j);
                        S.emplace_back(di, j);
                    }
                }

                // empty IS has been derived, we break
                if (conflict_found) break;
            }

            if (!conflict_found) alpha[i] = ui;

            for (auto is = 0; is < k_max; ++is) {
                //if (is == i) continue;
                ISs[is] = ISs_copy[is];
            }
        }
        if (ISs[i].none()) return false;
    }
    return true;
}

uint64_t steps = 0;
uint64_t pruned = 0;

void FindMaxClique(
    custom_graph& G,  // graph
    custom_bitset& K,       // current branch
    const int curr,           // lower bound
    custom_bitset& K_max,   // max branch
    int& lb,           // lower bound
    const custom_bitset& V, // vertices set
    custom_bitset& B,       // branching set
    std::vector<int>& u, // incremental upper bounds,
    const std::vector<std::size_t> &alpha = {},
    const int is_k_partite = 0
) {
    static std::vector ISs(G.size(), std::vector(G.size(), custom_bitset(G.size())));
    static std::vector<int> color_class(G.size());
    static custom_bitset prev_neighb_set(G.size());
    custom_bitset V_new(G.size());
    custom_bitset B_new(G.size());
    steps++;

    // bitset containing all elements of P and all elements of B up to j
    auto P_Bj = V - B;

    for (const auto bi : B) {
        P_Bj.set(bi);
        // if bi == 0, u[bi] always == 1!
        u[bi] = 1;

        // calculate sub-problem
        //custom_bitset::AND(P_Bj, G.get_neighbor_set(bi), V_new);
        custom_bitset::AND(G.get_neighbor_set(bi), V, prev_neighb_set, bi);
        for (const auto neighbor : prev_neighb_set) {
            u[bi] = std::max(u[bi], 1+u[neighbor]);
            // no point continue searching, we will overwrite this anyway with a potentially lower value
            if (u[bi] + curr-1 > lb) break;
        }

        // if we can't improve, we prune the current branch
        // curr-1 because bi is not part of K yet
        if (u[bi] + curr-1 <= lb) {
            pruned++;
            B.reset(bi);
            // lb-curr+1 because we have not added bi to K yet
            u[bi] = std::min(u[bi], lb-curr+1);
            continue;
        }

        u[bi] = std::min(u[bi], lb-curr);

        custom_bitset::AND(P_Bj, G.get_neighbor_set(bi), V_new);
        // if we are in a leaf
        if (V_new.none()) {
            // update best solution
            if (curr > lb) {
                lb = curr;
                K.set(bi);
                K_max = K;
                K.reset(bi);
                std::cout << "Last incumbent: " << lb << std::endl;
            }

            return;
        }

        const int k = lb-curr;
        int next_is_k_partite = is_k_partite;
        std::vector<std::size_t> new_alpha = alpha;
        const auto depth = std::max(is_k_partite - k, 0);

        // if is a k+1 partite graph
        if (is_k_partite) {
            // It's necessary to continue
            if (!FiltCOL(G, V_new, ISs[0], ISs[depth], color_class, new_alpha, k+1)) continue;
            if (!FiltSAT(G, V_new, ISs[depth], new_alpha, k+1)) continue;
            B_new = ISs[depth][k];
            //B_new = ISEQ_branching(G, V_new, k);
        } else {
            ISEQ_branching(G, V_new, ISs[depth], color_class, new_alpha, k, B_new);
            if (B_new.none()) continue;
            if (is_IS(G, B_new)) {
                ISs[depth][k] = B_new;
                new_alpha[k] = B_new.back();
                next_is_k_partite = k+1;
                // if we could return here, huge gains... damn
                if (!FiltSAT(G, V_new, ISs[depth], new_alpha, k+1)) continue;
                // TODO: which one?
                B_new = ISs[depth][k];
                //B_new = ISEQ_branching(G, V_new, k);
            } else {
                if (!SATCOL(G, B_new, ISs[depth], k)) continue;
            }
        }

        auto u_new = u;
        /*
        for (int i = 0; i < k; i++) {
            for (auto v : ISs[depth][i]) {
                //u_new[v] = std::min(u_new[v], dist);
                color_class[v] = i;
            }
        }

        custom_bitset seen(G.size());

        int distinct = 0;
        for (auto v : V_new) {
            const int c = color_class[v];
            if (c < k) {
                if (!seen.test(c)) {
                    seen.set(c);
                    distinct++;
                }
            } else {
                ++distinct;
            }
            int ub = std::min(distinct, k);
            // keep monotone across depths
            ub = std::min(ub, u[v]);
            u_new[v] = ub;
        }
        */

        // if B is not empty
        if (B_new.any()) {
            K.set(bi);

            FindMaxClique(G, K, curr+1, K_max, lb, V_new, B_new, u_new, new_alpha, next_is_k_partite);
            K.reset(bi);
        }

        // lb updated!
        u[bi] = std::min(u[bi], lb-curr);
    }
}

export custom_bitset CliSAT(const custom_graph& g) {
    auto [ordering, k] = NEW_SORT(g, 3);
    auto ordered_g = g.change_order(ordering);

    //auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     AMTS Tabu search
    custom_bitset K_max(g.size());
    custom_bitset K(g.size());
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
    // TODO: why it's necessary??
    for (std::size_t i = lb; i < ordered_g.size(); i++) {
        for (const auto neighbor : ordered_g.get_prev_neighbor_set(i)) {
            u[i] = std::max(u[i], 1 + u[neighbor]);
        }
        u[i] = std::min(u[i], k);
    }

    for (std::size_t i = lb; i < ordered_g.size(); ++i) {
        auto begin = std::chrono::steady_clock::now();
        auto V = custom_bitset::before(ordered_g.get_neighbor_set(i), i);
        B = V;

        // we pruned first lb vertices of V (they can't improve the solution on they own)
        // if we set count to zero, we can't possibly improve the solution because the B set can become empty even tough
        // it should be possible to improve, lb vertices + 1 from k, but we remove every one from the 23
        auto count = 1;
        for (const auto v : B) {
            if (count == lb) break;
            B.reset(v);
            count++;
        }

        K.set(i);
        auto u_new = u;
        FindMaxClique(ordered_g, K, 2, K_max, lb, V, B, u_new);
        K.reset(i);

        u[i] = lb;

        auto end = std::chrono::steady_clock::now();
        std::print("{}/{} (max {}) {}ms\n", i+1, ordered_g.size(), K_max.count(), std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
    }

    std::cout << "Steps: " << steps << std::endl;
    std::cout << "Pruned: " << pruned << std::endl;

    return ordered_g.convert_back_set(K_max);
}
