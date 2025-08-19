//
// Created by Beniamino Vagnarelli on 08/04/25.
//

module;

#include <set>
#include <vector>
#include <iostream>

export module CliSAT;

import AMTS;
import custom_bitset;
import custom_graph;
import sorting;
import coloring;

// TODO: implement
inline void SATCOL() {

}

// TODO: implement
inline void FiltCOL() {

}

// TODO: implement
inline void FiltSAL() {

}

/**
 *
 */
inline custom_bitset IncMaxSat(
    custom_graph& G,
    const custom_bitset& V,
    custom_bitset B,
    std::vector<custom_bitset>& ISs,
    int r,
    std::vector<int>& VertexUB
) {
    //G.resize(G.size()+ B.count()*r*2);

    static std::vector<custom_bitset> ISs_copy(G.size(), custom_bitset(G.size()*2));

    auto count = 0;

    for (const auto ui : B) {
        bool conflict_found = false;

        auto unit_is = custom_bitset(G.size());
        unit_is.set(ui); // we create a unit independent set with the current vertex ui

        std::vector<std::pair<custom_bitset::reference, int>> S;

        ISs[r] = unit_is; // we add the unit independent set to ISs
        S.emplace_back(ui, r); // we add the current vertex ui to the stack S
        std::set<int> culprit_ISs;

        custom_bitset already_visited(G.size());

        for (auto i = 0; i < r+1; ++i) {
            ISs_copy[i] = ISs[i];
        }

        custom_bitset removed_literals(G.size());

        //const std::vector<custom_bitset> ISs_copy(ISs.begin(), ISs.begin() + r+1);

        while (!S.empty()) {
            const auto [u, u_is] = S.back();
            S.pop_back();

            // insert current node to the culprit_ISs
            // TODO: necessary?
            culprit_ISs.emplace(u_is);
            //ISs[u_is].reset(u);

            auto neighbor_set = G.get_neighbor_set(u);
            //neighbor_set.set(u);

            // useless to iterate over r+1 that contains only u;
            for (auto i = 0; i < r; ++i) {
                custom_bitset& D = ISs[i];

                // if D is the unit IS set that we are setting to true, we continue
                if (i == u_is) continue;

                // TODO: necessary?
                removed_literals |= D - neighbor_set;
                // remove vertices non adjacent to u
                D &= neighbor_set;
                auto di = D.front();

                if (di == D.size()) { // empty IS, conflict detected
                    culprit_ISs.emplace(i); // we have derived an empty independent set
                    conflict_found = true;
                    B.reset(ui);
                    count++;
                    //return B;
                    break;
                }
                if (!already_visited[di] && D.next(di) == D.size()) { // Unit IS
                    already_visited.set(di);
                    S.emplace_back(di, i);
                }
            }

            // no empty IS has been derived, we can continue
            if (!conflict_found) continue;

            // if we have derived an empty IS, we restore all removed vertices
            for (auto i = 0; i < r+1; ++i) {
                ISs[i] = ISs_copy[i];
            }

            // we need to add culprit_ISs also based on hard clauses (neighbors)
            for (auto is = 0; is < r; ++is) {
                if ((ISs[is] & removed_literals).any()) {
                    culprit_ISs.emplace(is);
                }
            }

            auto first_node = G.size();
            G.resize(G.size() + culprit_ISs.size());

            for (auto is = 0; is < r+1; ++is) {
                ISs[is].resize(G.size());
            }

            auto last_node = first_node;

            for (const auto is : culprit_ISs) {
                for (auto curr_is = 0; curr_is < r+1; ++curr_is) {
                    if (curr_is == is) continue; // skip the current IS

                    for (auto v : ISs[curr_is]) {
                        if (v >= first_node) break; // skip new vertices
                        G.add_edge(last_node, v);
                    }
                }
                for (auto v : B) {
                    G.add_edge(last_node, v);
                }

                ISs[is].set(last_node); // add the new vertex to the is
                ++last_node;
            }

            break;
        }

        /*
        for (uint64_t i = 0; i < r; ++i) {
            ISs[i] = ISs_copy2[i];
        }
        */
        // no empty IS has been derived, we can break
        if (!conflict_found) break;
        // TODO: necessary?
        r++;
    }

    G.resize(VertexUB.size());

    //std::cout << count << std::endl;

    return B;
}

// TODO: P pass by reference or not? I don't think so
// we pass u by copy, not reference!
inline void FindMaxClique(
    const custom_graph& G,  // graph
    custom_bitset& K,       // current branch
    const int curr,           // lower bound
    custom_bitset& K_max,   // max branch
    int& lb,           // lower bound
    const custom_bitset& V, // vertices set
    const custom_bitset &B,       // branching set
    // should not be a reference?
    std::vector<int> u // incremental upper bounds
) {
    //static std::vector<custom_bitset> ISs(G.size(), custom_bitset(G.size()*2));

    auto V_copy = V;
    for (const auto bi : B) {
        K.set(bi);

        // calculate u[bi]
        // if bi == 0, u[bi] always == 1!
        u[bi] = 0;
        // TODO: G.get_prev_neighbor_set(bi, V_copy) wrong because we are removing elements from V_copy
        for (const auto neighbor : G.get_prev_neighbor_set(bi, V)) {
            u[bi] = std::max(u[bi], 1+u[neighbor]);
            if (u[bi] + curr > lb) break;
        }

        // if we can't improve, we prune the current branch
        if (u[bi] + curr > lb) {
            V_copy.reset(bi);
            auto V_new = V_copy & G.get_neighbor_set(bi);

            // if we are in a leaf
            if (V_new.none()) {
                // update best solution
                // TODO: inside or outside?
                if (curr > lb) {
                    lb = curr;
                    K_max = K;
                    //std::cout << "New solution: " << lb << std::endl;
                }

                K.reset(bi);
                return;
            }

            auto B_new = ISEQ_branching(G, V_new, lb - curr);
            /*
            if (B_new.none()) {
                K.reset(bi);
                continue;
            }
            */
            //B_new = IncMaxSat(G, V_new, B_new, ISs, lb - K.count(), u);

            // TODO
            // PMAX-SAT-P-based upper bounds (we improve UP and find new branching nodes)

            // FiltCOL
            // FiltSAT
            // SATCOL

            // if B is not none
            if (!B_new.none()) {
                FindMaxClique(G, K, curr+1, K_max, lb, V_new, B_new, u);
            }
        }

        u[bi] = std::min(u[bi], lb-curr);
        K.reset(bi);
    }
}

export inline custom_bitset CliSAT(const custom_graph& g) {
    auto [ordering, k] = NEW_SORT(g, 3);
    auto ordered_g = g.change_order(ordering);

    //auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     AMTS Tabu search
    custom_bitset K_max = {0};
    int lb = static_cast<int>(K_max.count());

    std::vector<int> u(ordered_g.size());
    // first |k_max| values bounded by |K_max| (==lb)
    u[0] = 1;
    for (auto i = 1; i < lb; i++) {
        auto max_u = 0;
        const custom_bitset prev_neighb_set = ordered_g.get_prev_neighbor_set(i);

        for (const auto neighbor : prev_neighb_set) {
            max_u = std::max(max_u, u[neighbor]);
        }
        u[i] = std::min(1 + max_u, lb);
    }

    // remaining values bounded by k
    for (std::size_t i = lb; i < ordered_g.size(); i++) {
        auto max_u = 0;
        const custom_bitset prev_neighb_set = ordered_g.get_prev_neighbor_set(i);

        for (const auto neighbor : prev_neighb_set) {
            max_u = std::max(max_u, u[neighbor]);
        }
        u[i] = std::min(1 + max_u, k);
    }

    for (auto i = K_max.count(); i < ordered_g.size(); ++i) {
        //std::cout << "Node " << i << std::endl;
        custom_bitset V = ordered_g.get_prev_neighbor_set(i);

        custom_bitset K(ordered_g.size());
        K.set(i);

        // first lb vertices of V
        // we start from 1 because we already have i in K
        custom_bitset B(V);
        auto count = 1;
        for (const auto v : V) {
            B.reset(v);
            count++;
            if (count >= lb) break;
        }

        FindMaxClique(ordered_g, K, 2, K_max, lb, V, B, u);
        u[i] = lb;
    }

    return ordered_g.convert_back_set(K_max);
}
