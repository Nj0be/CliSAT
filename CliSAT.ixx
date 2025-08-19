//
// Created by Beniamino Vagnarelli on 08/04/25.
//

module;

#include <chrono>
#include <set>
#include <vector>
#include <cstdint>
#include <iostream>
#include <ranges>

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

/*
/**
 * \brief Inserts the biggest vertices of P into r independent sets.
 *
 * Page 8 (143) Li et al. (2018a, 2017), section 5.2.1
 *
 * @param G Ordered graph
 * @param V Candidate set {p1, p2, ..., p|P|} of vertices to be inserted into the independent sets
 * @param r lower bound related to the largest clique found so far
 * @param VertexUB incremental upper bounds for each vertex
 * @return a set B of branching vertices that cannot be inserted into any of the r ISs and the r ISs themselves
 #1#
inline std::pair<custom_bitset, std::vector<custom_bitset>> FilterByColoring(
    const custom_graph& G,
    const custom_bitset& V,
    const uint64_t r,
    std::vector<uint64_t>& VertexUB
) {
    custom_bitset B(G.size());  // B, a set of branching vertices obtained from P
    std::vector<custom_bitset> ISs;  // set of independent sets

    // We insert the biggest vertices of P into r ISs (decreasing order)
    // TODO: doesn't work with reverse order
    for (const auto pi : V) {
        bool inserted = false;

        // If there is an IS in which pi is not adjacent to any vertex, we insert pi into that IS
        for (auto &is: ISs) {
            if (G.get_neighbor_set(pi, is).any()) continue;

            is.set(pi);
            inserted = true;

            if (B.none()) VertexUB[pi] = std::min(VertexUB[pi], ISs.size());

            break;
        }
        if (inserted) continue; // pi has been inserted into an IS, we can skip it

        if (ISs.size() < r) {
            // If there is no IS, we create a new one
            custom_bitset new_is(G.size());
            new_is.set(pi);
            ISs.push_back(new_is);

            // TODO: fix
            //std::cout << pi << " " << VertexUB[pi] << " " << ISs.size() << std::endl;
            //VertexUB[pi] = std::min(VertexUB[pi], ISs.size());

            continue;
        }


        // if there is an IS in which pi has only one adjacent vertex u, and u can be inserted into another IS
        // RECOLOR (RE-NUMBER of MCS)
        for (uint64_t i = 0; i < ISs.size(); ++i) {
            auto common_neighbors = G.get_neighbor_set(pi, ISs[i]);
            auto u = common_neighbors.front();

            if (u == common_neighbors.size()) continue; // no common neighbor, we can skip this IS

            // if there are more than one common neighbor, we can't insert pi into this IS
            if (common_neighbors.next(u) != common_neighbors.size()) continue;

            // u is the only neighbor of pi in is
            for (uint64_t j = 0; j < ISs.size(); ++j) {
            //for (uint64_t j = i+1; j < ISs.size(); ++j) {
                if (j == i) continue; // skip the current IS

                // if the intersection is not none, we can't insert u into this IS
                if (G.get_neighbor_set(u, ISs[j]).any()) continue; // u is adjacent to some vertex in ISs[j]

                // we can insert u in this IS, removing u from the previous IS and then insert pi into it
                ISs[i].reset(u);
                ISs[i].set(pi);
                ISs[j].set(u);
                inserted = true;

                if (B.none()) { VertexUB[pi] = std::min(VertexUB[pi], ISs.size()); }

                break;
            }
            if (inserted) break;
        }

        if (!inserted) B.set(pi);
    }

    return {B, ISs};
}


inline custom_bitset FilterByColoring2(
    const custom_graph& G,
    const custom_bitset& V,
    std::vector<custom_bitset>& ISs,
    const uint64_t r,
    std::vector<uint64_t>& VertexUB
) {
    custom_bitset B(G.size());  // B, a set of branching vertices obtained from P
    uint64_t ISs_size = 0;

    // We insert the biggest vertices of P into r ISs (decreasing order)
    // TODO: doesn't work with reverse order
    for (const auto pi : V) {
        bool inserted = false;

        // If there is an IS in which pi is not adjacent to any vertex, we insert pi into that IS
        for (uint64_t i = 0; i < ISs_size; ++i) {
            if (G.get_neighbor_set(pi, ISs[i]).any()) continue;

            ISs[i].set(pi);
            inserted = true;

            if (B.none()) VertexUB[pi] = std::min(VertexUB[pi], ISs_size);

            break;
        }
        if (inserted) continue; // pi has been inserted into an IS, we can skip it

        if (ISs_size < r) {
            // If there is no IS, we create a new one
            custom_bitset new_is(G.size());
            new_is.set(pi);
            ISs[ISs_size] = new_is;
            ISs_size++;

            // TODO: fix
            //std::cout << pi << " " << VertexUB[pi] << " " << ISs.size() << std::endl;
            //VertexUB[pi] = std::min(VertexUB[pi], ISs_size);

            continue;
        }


        // if there is an IS in which pi has only one adjacent vertex u, and u can be inserted into another IS
        // RECOLOR (RE-NUMBER of MCS)
        for (uint64_t i = 0; i < ISs_size; ++i) {
            auto common_neighbors = G.get_neighbor_set(pi, ISs[i]);
            auto u = common_neighbors.front();

            if (u == common_neighbors.size()) continue; // no common neighbor, we can skip this IS

            // if there are more than one common neighbor, we can't insert pi into this IS
            if (common_neighbors.next(u) != common_neighbors.size()) continue;

            // u is the only neighbor of pi in is
            for (uint64_t j = 0; j < ISs_size; ++j) {
            //for (uint64_t j = i+1; j < ISs.size(); ++j) {
                if (j == i) continue; // skip the current IS

                // if the intersection is not none, we can't insert u into this IS
                if (G.get_neighbor_set(u, ISs[j]).any()) continue; // u is adjacent to some vertex in ISs[j]

                // we can insert u in this IS, removing u from the previous IS and then insert pi into it
                ISs[i].reset(u);
                ISs[i].set(pi);
                ISs[j].set(u);
                inserted = true;

                if (B.none()) { VertexUB[pi] = std::min(VertexUB[pi], ISs_size); }

                break;
            }
            if (inserted) break;
        }

        if (!inserted) B.set(pi);
    }

    return B;
}
*/

/**
 *
 */
/*
inline custom_bitset IncMaxSat(
    custom_graph& G,
    const custom_bitset& V,
    const custom_bitset& B,
    std::vector<custom_bitset>& ISs,
    const uint64_t r,
    std::vector<uint64_t>& VertexUB
) {
    auto last_ui = custom_bitset::reference(0); // last vertex for which we derived an empty independent set
    custom_bitset already_visited(G.size());

    for (const auto ui : B) {
        auto unit_is = custom_bitset(G.size());
        unit_is.set(ui); // we create a unit independent set with the current vertex ui

        std::vector<std::pair<custom_bitset::reference, uint64_t>> S;

        ISs.emplace_back(unit_is);
        S.emplace_back(ui, ISs.size() - 1); // we add the current vertex ui to the stack S
        std::set<uint64_t> culprit_ISs;

        while (!S.empty()) {
            const auto [u, u_is] = S.back();
            S.pop_back();
            already_visited.set(u);
            // insert current node to the culprit_ISs
            culprit_ISs.insert(u_is);

            const auto ISs_copy = ISs;

            for (uint64_t i = 0; i < ISs.size(); ++i) {
                custom_bitset& D = ISs[i];
                // remove vertices non adjacent to u
                D &= G.get_neighbor_set(u);
                if (D.none()) {
                    culprit_ISs.emplace(i); // we have derived an empty independent set
                    last_ui = ui; // latest vertex for which we derived an empty IS
                    break;
                }
                if (!already_visited[D.front()] && D.count() == 1) {
                    S.emplace_back(D.front(), i);
                }
            }
            // no empty IS has been derived, we can continue
            if (last_ui != ui) continue;

            // if we have derived an empty IS, we restore all removed vertices
            ISs = ISs_copy;

            const uint64_t old_size = G.size();
            G.resize(G.size() + culprit_ISs.size());
            already_visited.resize(G.size());
            for (auto D : ISs) {
                D.resize(G.size());
            }

            uint64_t offset = 0;
            for (const auto is : culprit_ISs) {
                for (uint64_t curr_is = 0; curr_is < ISs.size(); ++curr_is) {
                    if (curr_is == is) continue; // skip the current IS

                    for (const auto v : ISs[curr_is]) {
                        G.add_edge(old_size + offset, v);
                    }
                }
                ISs[is].set(old_size + offset); // add the new vertex to the is
                ++offset;
            }

            break;
        }
        // no empty IS has been derived, we can break
        if (last_ui != ui) break;
    }

    // restore G size
    G.resize(VertexUB.size());

    // a conflict has been derived for every vertex in B, we return an empty set
    if (last_ui == B.back()) return custom_bitset(G.size());

    // ui is the biggest vertex in B for which IncMaxSat fails to derive a conflict
    for (const auto a : V) {
        if (B.front() <= a && a < last_ui) {
            //VertexUB[a] = std::min(VertexUB[a], r);
        }
    }

    // { p | p in P and p <= ui }
    custom_bitset B_ret = V;
    B_ret.clear_before(last_ui);

    return B_ret;
}
*/


/**
 *
 */
inline custom_bitset IncMaxSat(
    custom_graph& G,
    const custom_bitset& V,
    custom_bitset B,
    std::vector<custom_bitset>& ISs,
    uint64_t r,
    std::vector<uint64_t>& VertexUB
) {
    //G.resize(G.size()+ B.count()*r*2);

    static std::vector<custom_bitset> ISs_copy(G.size(), custom_bitset(G.size()*2));

    auto count = 0;

    for (const auto ui : B) {
        bool conflict_found = false;

        auto unit_is = custom_bitset(G.size());
        unit_is.set(ui); // we create a unit independent set with the current vertex ui

        std::vector<std::pair<custom_bitset::reference, uint64_t>> S;

        ISs[r] = unit_is; // we add the unit independent set to ISs
        S.emplace_back(ui, r); // we add the current vertex ui to the stack S
        std::set<uint64_t> culprit_ISs;

        custom_bitset already_visited(G.size());

        for (uint64_t i = 0; i < r+1; ++i) {
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
            for (uint64_t i = 0; i < r; ++i) {
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
            for (uint64_t i = 0; i < r+1; ++i) {
                ISs[i] = ISs_copy[i];
            }

            // we need to add culprit_ISs also based on hard clauses (neighbors)
            for (uint64_t is = 0; is < r; ++is) {
                if ((ISs[is] & removed_literals).any()) {
                    culprit_ISs.emplace(is);
                }
            }

            uint64_t first_node = G.size();
            G.resize(G.size() + culprit_ISs.size());

            for (uint64_t is = 0; is < r+1; ++is) {
                ISs[is].resize(G.size());
            }

            auto last_node = first_node;

            for (const auto is : culprit_ISs) {
                for (uint64_t curr_is = 0; curr_is < r+1; ++curr_is) {
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
    const uint64_t curr,           // lower bound
    custom_bitset& K_max,   // max branch
    uint64_t& lb,           // lower bound
    const custom_bitset& V, // vertices set
    const custom_bitset &B,       // branching set
    // should not be a reference?
    std::vector<uint64_t> u // incremental upper bounds
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
            //auto V_new = (V - custom_bitset(B, bi)) & G.get_neighbor_set(bi);
            V_copy.reset(bi);
            auto V_new = V_copy & G.get_neighbor_set(bi);
            /*
            auto V_new = V - custom_bitset::from(B, bi);
            V_new &= G.get_neighbor_set(bi);
            */
            //auto V_new = ((V - B) | custom_bitset::until(B, bi) ) & G.get_neighbor_set(bi);
            /*
            auto V_new = ((V - B) );
            V_new |= custom_bitset::until(B, bi);
            V_new &= G.get_neighbor_set(bi);
            */

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

        /*
        custom_bitset inv = custom_bitset::after(G.get_neighbor_set(bi), bi);
        if (u[bi] < lb - curr) {
            u[bi] = lb - curr;
            for (auto v : inv) {
                inv |= custom_bitset::after(G.get_neighbor_set(v), v);
                u[v] = 0;
                for (const auto neighbor : G.get_prev_neighbor_set(v)) {
                    u[v] = std::max(u[v], 1 + u[neighbor]);
                }
            }
        }
        */
        /*
        if (u[bi] < lb - curr) {
            u[bi] = lb - curr;
            for (auto v = bi+1; v < G.size(); ++v) {
                u[v] = 0;
                for (const auto neighbor : G.get_prev_neighbor_set(v)) {
                    u[v] = std::max(u[v], 1 + u[neighbor]);
                }
            }
        }
        */
        /*
        if (u[bi] < lb - curr) {
            u[bi] = lb - curr;
            for (auto v = G.get_neighbor_set(bi).next(bi); v != G.size(); v = G.get_neighbor_set(bi).next(v)) {
                u[v] = 0;
                for (const auto neighbor : G.get_prev_neighbor_set(v)) {
                    u[v] = std::max(u[v], 1 + u[neighbor]);
                }
            }
        }
        */
        K.reset(bi);
    }
}

export inline custom_bitset CliSAT(const custom_graph& g) {
    auto begin = std::chrono::steady_clock::now();
    auto [ordering, k] = NEW_SORT(g, 3);
    auto end = std::chrono::steady_clock::now();
    uint64_t tot = 0;
    tot += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    //std::cout << "Prep = " << tot << "[µs]" << std::endl;
    auto ordered_g = g.change_order(ordering);

    //auto K_max = run_AMTS(ordered_g); // lb <- |K|    ->     AMTS Tabu search
    custom_bitset K_max = {0};
    uint64_t lb = K_max.count();

    std::vector<uint64_t> u(ordered_g.size());
    // first |k_max| values bounded by |K_max| (==lb)
    u[0] = 1;
    for (uint64_t i = 1; i < lb; i++) {
        uint64_t max_u = 0;
        const custom_bitset prev_neighb_set = ordered_g.get_prev_neighbor_set(i);

        for (const auto neighbor : prev_neighb_set) {
            max_u = std::max(max_u, u[neighbor]);
        }
        u[i] = std::min(1 + max_u, lb);
    }

    // remaining values bounded by k
    for (uint64_t i = lb; i < ordered_g.size(); i++) {
        uint64_t max_u = 0;
        const custom_bitset prev_neighb_set = ordered_g.get_prev_neighbor_set(i);

        for (const auto neighbor : prev_neighb_set) {
            max_u = std::max(max_u, u[neighbor]);
        }
        u[i] = std::min(1 + max_u, k);
    }

    tot = 0;
    for (uint64_t i = K_max.count(); i < ordered_g.size(); ++i) {
        //std::cout << "Node " << i << std::endl;
        custom_bitset V = ordered_g.get_prev_neighbor_set(i);

        custom_bitset K(ordered_g.size());
        K.set(i);

        // first lb vertices of V
        // we start from 1 because we already have i in K
        custom_bitset B(V);
        uint64_t count = 1;
        for (const auto v : V) {
            B.reset(v);
            count++;
            if (count >= lb) break;
        }


        FindMaxClique(ordered_g, K, 2, K_max, lb, V, B, u);
        //auto begin = std::chrono::steady_clock::now();
        u[i] = lb;
        /*
        for (auto v = i+1; v < ordered_g.size(); ++v) {
            if (u[v] <= lb+1) continue;
            u[v] = 0;
            for (const auto neighbor : ordered_g.get_prev_neighbor_set(v)) {
                u[v] = std::max(u[v], 1 + u[neighbor]);
            }
        }
        auto end = std::chrono::steady_clock::now();
    */
    }
    //std::cout << "Prep = " << tot << "[µs]" << std::endl;

    return ordered_g.convert_back_set(K_max);
}
