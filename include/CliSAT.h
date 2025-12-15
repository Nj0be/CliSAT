//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>

#include "coloring.h"
#include "custom_bitset.h"
#include "custom_graph.h"
#include "fixed_vector.h"
#include "solution.h"
#include "threadsafe_vector.h"
#include "thread_pool.h"

inline constexpr int NONE = -1;

enum SORTING_METHOD {
    NO_SORT,
    NEW_SORT,
    DEG_SORT,
    COLOUR_SORT,
    RANDOM_SORT
};

class Solver {
public:
    explicit Solver(
        const size_t G_size
    ) : ISs_mapping(G_size),
          ISs_size(G_size),
          ISs_state(G_size, true),
          ISs_involved(G_size),
          ISs_used(G_size),
          ISs_tested(G_size),
          is_processed(G_size),
          ISs_new(G_size),
          is_processed_new(G_size),
          reason(G_size * 2),
          conflict_iset_stack(G_size),
          // Necessary G.size()*2 because there are also ADDED_NODES
          reduced_iset_stack(G_size * 2),
          passive_iset_stack(G_size),
          fixed_node_stack(G_size * 2),
          reason_stack(G_size),
          unit_stack(G_size),
          unit_stack2(G_size),
          unit_stack3(G_size),
          nodes(G_size),
          nodes2(G_size),
          nodes3(G_size),
          V_new(G_size),
          _color_class(G_size) {}

    void FindMaxClique(
        const custom_graph& G,  // graph
        fixed_vector<int>& K,       // current branch
        solution<int>& K_max,   // max branch
        custom_bitset& P_Bj,       // vertices set
        const custom_bitset& B,          // branching set
        std::vector<int>& u,        // incremental upper bounds
        std::chrono::time_point<std::chrono::steady_clock> max_time,
        thread_pool<Solver>& pool,
        size_t sequence,
        const fixed_vector<int>& alpha,
        bool is_k_partite = false,
        const std::vector<custom_bitset>& ISs = {},
        const std::vector<int>& color_class = {}
    );

private:
    std::vector<int> ISs_mapping;
    std::vector<int> ISs_size;
    std::vector<std::uint8_t> ISs_state;
    std::vector<std::uint8_t> ISs_involved;
    std::vector<std::uint8_t> ISs_used;
    std::vector<std::uint8_t> ISs_tested;
    custom_bitset is_processed;
    std::vector<std::vector<int>> ISs_new;
    std::vector<std::uint8_t> is_processed_new;
    std::vector<int> reason;
    std::vector<std::vector<int>> conflict_iset_stack;
    fixed_vector<int> reduced_iset_stack;
    fixed_vector<int> passive_iset_stack;
    fixed_vector<int> fixed_node_stack;
    fixed_vector<int> reason_stack;
    fixed_vector<int> unit_stack;
    fixed_vector<int> unit_stack2;
    fixed_vector<int> unit_stack3;
    custom_bitset nodes;
    custom_bitset nodes2;
    custom_bitset nodes3;
    custom_bitset V_new;
    std::vector<int> _color_class;
    std::vector<custom_bitset> _ISs;

    void identify_conflict_isets(
        int iset,
        const std::vector<custom_bitset>& ISs
    );

    int fix_newNode_for_iset(
        int fix_node,
        int fix_iset,
        const custom_graph& G,
        fixed_vector<int>& unit_stack
    );

    int fix_oldNode_for_iset(
        int fix_node,
        int fix_iset,
        const custom_graph& G,
        fixed_vector<int>& unit_stack,
        const std::vector<int>& color_class
    );

    [[nodiscard]] int get_node_of_unit_iset(
        int l_is,
        const custom_graph& G,
        const std::vector<custom_bitset>& ISs
    ) const;

    int fix_anyNode_for_iset(
        int fix_iset,
        const custom_graph& G,
        const std::vector<custom_bitset>& ISs,
        fixed_vector<int>& unit_stack,
        const std::vector<int>& color_class
    );

    int unit_iset_process(
        const custom_graph& G,
        const std::vector<custom_bitset>& ISs,
        fixed_vector<int>& new_unit_stack,
        const std::vector<int>& color_class
    );

    int unit_iset_process_used_first(
        const custom_graph& G,
        const std::vector<custom_bitset>& ISs,
        fixed_vector<int>& unit_stack,
        const std::vector<int>& color_class
    );

    void enlarge_conflict_sets(
        int ADDED_NODE,
        const custom_graph& G
    );

    void reset_context_for_maxsatz();

    void rollback_context_for_maxsatz(
        int reduced_iset_start,
        int passive_iset_start,
        int fixed_iset_start
    );

    int simple_further_test_node(
        int start,
        const custom_graph& G,
        const std::vector<custom_bitset>& ISs,
        const std::vector<int>& color_class
    );

    int test_node_for_failed_nodes(
        int node,
        int iset,
        const custom_graph& G,
        const std::vector<custom_bitset>& ISs,
        const std::vector<int>& color_class
    );

    bool test_by_eliminate_failed_nodes(
        const custom_graph& G,
        const std::vector<custom_bitset>& ISs,
        const std::vector<int>& color_class,
        int k
    );

    int further_test_reduced_iset(
        int start,
        const custom_graph& G,
        const std::vector<custom_bitset>& ISs,
        const std::vector<int>& color_class
    );

    int inc_maxsatz_lookahead_by_fl2(
        const custom_graph& G,
        const std::vector<custom_bitset>& ISs,
        const std::vector<int>& color_class
    );

    bool inc_maxsatz_on_last_iset(
        int ADDED_NODE,
        const custom_graph& G,
        custom_bitset& B,
        const std::vector<custom_bitset>& ISs,
        const std::vector<int>& color_class,
        int k
    );

    bool SATCOL(
        const custom_graph& G,
        custom_bitset& B,
        std::vector<custom_bitset>& ISs,
        std::vector<int>& color_class,
        int k
    );

    int FiltCOL(
        const custom_graph& G,  // graph
        custom_bitset& V,       // vertices set
        const std::vector<custom_bitset>& ISs,
        std::vector<custom_bitset>& ISs_t,
        const std::vector<int>& color_class,
        std::vector<int>& color_class_t,
        const fixed_vector<int>& alpha,
        int k_max
    );

    bool test_by_eliminate_failed_nodes2(
        custom_bitset& V,
        const custom_graph& G,
        std::vector<custom_bitset>& ISs,
        const std::vector<int>& color_class,
        int k
    );

    bool FiltSAT(
        const custom_graph& G,  // graph
        custom_bitset& V,       // vertices set
        std::vector<custom_bitset>& ISs,
        const std::vector<int>& color_class,
        int k_max
    );
};

static std::atomic_uint64_t steps = 0;
static std::atomic_uint64_t pruned = 0;

inline void Solver::identify_conflict_isets(
    const int iset,
    const std::vector<custom_bitset>& ISs
) {
    int starting_index = reason_stack.size();
    reason_stack.push_back(iset);
    ISs_involved[iset] = true;
    for (int i = starting_index; i < reason_stack.size(); i++) {
        const auto reason_iset = reason_stack[i];

        // removed_nodes
        custom_bitset::AND(nodes3, ISs[reason_iset], is_processed);
        for (auto r : nodes3) {
            const auto r_iset = reason[r];
            if (r_iset == NONE || ISs_involved[r_iset]) continue;

            ISs_involved[r_iset] = true;
            reason_stack.push_back(r_iset);
        }
        for (auto r : ISs_new[reason_iset]) {
            // not processed (removed)
            if (!is_processed_new[r-is_processed.size()] || reason[r] == NONE) continue;

            const auto r_iset = reason[r];
            if (ISs_involved[r_iset]) continue;

            ISs_involved[r_iset] = true;
            reason_stack.push_back(r_iset);
        }
    }

    for (int i = starting_index; i < reason_stack.size(); i++) {
        ISs_involved[reason_stack[i]] = false;
        ISs_used[reason_stack[i]] = true;
    }

    ISs_involved[iset] = false;
}

inline int Solver::fix_newNode_for_iset(
    const int fix_node,
    const int fix_iset,
    const custom_graph& G,
    fixed_vector<int>& new_unit_stack
) {
    ISs_state[fix_iset] = false;
    ISs_size[fix_iset]--;
    reduced_iset_stack.push_back(fix_iset);
    passive_iset_stack.push_back(fix_iset);
    fixed_node_stack.push_back(fix_node);
    is_processed_new[fix_node-G.size()] = true;
    reason[fix_node] = fix_iset;

    for (auto iset_idx : conflict_iset_stack[fix_node-G.size()]) {
        if (!ISs_state[iset_idx]) continue;

        ISs_size[iset_idx]--;
        reduced_iset_stack.push_back(iset_idx);

        if (ISs_size[iset_idx] == 0) return iset_idx;
        if (ISs_size[iset_idx] == 1) {
            new_unit_stack.push_back(iset_idx);
        }
    }
    return NONE;
}

inline int Solver::fix_oldNode_for_iset(
    const int fix_node,
    const int fix_iset,
    const custom_graph& G,
    fixed_vector<int>& new_unit_stack,
    const std::vector<int>& color_class
) {
    ISs_size[fix_iset]--;
    reduced_iset_stack.push_back(fix_iset);
    ISs_state[fix_iset] = false;
    passive_iset_stack.push_back(fix_iset);
    is_processed.set(fix_node);
    fixed_node_stack.push_back(fix_node);
    reason[fix_node] = fix_iset;

    //custom_bitset::DIFF(anti_neighbors, G.get_complement_neighbor_set(fix_node), is_processed);
    // equivalent to the above -> ~a & ~b == ~(a|b) (de morgan)
    custom_bitset::NOR(nodes3, G.get_neighbor_set(fix_node), is_processed);
    //is_processed |= anti_neighbors;

    for (auto r : nodes3) {
        const auto iset = color_class[r];

        if (!ISs_state[iset]) continue;

        ISs_size[iset]--;
        reduced_iset_stack.push_back(iset);
        is_processed.set(r);
        fixed_node_stack.push_back(r);
        reason[r] = fix_iset;

        // conflict found
        if (ISs_size[iset] == 0) return iset;

        if (ISs_size[iset] == 1) {
            new_unit_stack.push_back(iset);
        }
    }

    return NONE;
}

inline int Solver::get_node_of_unit_iset(
    const int l_is,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs
) const {
    for (auto new_node : ISs_new[l_is]) {
        if (is_processed_new[new_node-G.size()]) continue;
        return new_node;
    }

    const auto node  = ISs[l_is].front_difference(is_processed);
    if (node != custom_bitset::npos) return node;

    std::cout << "Error in get_node_of_unit_iset: l_is{" << l_is << "} node{" << node << "}" << std::endl;
    exit(1);
}

inline int Solver::fix_anyNode_for_iset(
    const int fix_iset,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    fixed_vector<int>& new_unit_stack,
    const std::vector<int>& color_class
) {
    const auto fix_node = get_node_of_unit_iset(fix_iset, G, ISs);

    if (fix_node >= G.size()) return fix_newNode_for_iset(fix_node, fix_iset, G, new_unit_stack);

    return fix_oldNode_for_iset(fix_node, fix_iset, G, new_unit_stack, color_class);
}

inline int Solver::unit_iset_process(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    fixed_vector<int>& new_unit_stack,
    const std::vector<int>& color_class
) {
    for (int l_is : unit_stack) {
        if (!ISs_state[l_is] || ISs_size[l_is] != 1) continue;

        new_unit_stack.clear();

        auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, new_unit_stack, color_class);
        if (empty_iset != NONE) return empty_iset;

        for (auto j = 0; j < new_unit_stack.size(); j++) {
            const auto l_is2 = new_unit_stack[j];
            if (!ISs_state[l_is2]) continue;

            // we are iterating on a unit_stack
            assert(ISs_size[l_is2] == 1);

            empty_iset = fix_anyNode_for_iset(l_is2, G, ISs, new_unit_stack, color_class);
            if (empty_iset != NONE) return empty_iset;
        }
    }

    return NONE;
}

inline int Solver::unit_iset_process_used_first(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    fixed_vector<int>& unit_stack,
    const std::vector<int>& color_class
) {
    int j = 0;
    int used_iset_start = 0;
    int iset_start = 0;

    do {
        for (; used_iset_start < unit_stack.size(); used_iset_start++) {
            const auto l_is = unit_stack[used_iset_start];

            // no need to check if iset is unit!
            if (!ISs_used[l_is] || !ISs_state[l_is]) continue;
            assert(ISs_size[l_is] == 1);

            //fix_node_iset
            const auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, unit_stack, color_class);
            if (empty_iset != NONE) return empty_iset;
        }

        for (j = iset_start; j < unit_stack.size(); j++) {
            const auto l_is = unit_stack[j];

            if (!ISs_state[l_is]) continue;
            assert(ISs_size[l_is] == 1);

            const auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, unit_stack, color_class);
            if (empty_iset != NONE) return empty_iset;

            iset_start = j+1;
            break;
        }
    } while (j < unit_stack.size());

    return NONE;
}

inline void Solver::enlarge_conflict_sets(
    const int ADDED_NODE,
    const custom_graph& G
) {
    is_processed_new[ADDED_NODE-G.size()] = false;
    reason[ADDED_NODE] = NONE;
    conflict_iset_stack[ADDED_NODE-G.size()].clear();

    for (int iset : reason_stack) {
        if (ISs_involved[iset]) continue;

        ISs_involved[iset] = true;
        ISs_new[iset].push_back(ADDED_NODE);
        ISs_size[iset]++;
        conflict_iset_stack[ADDED_NODE-G.size()].push_back(iset);
    }

    for (int reason_iset : reason_stack) {
        ISs_involved[reason_iset] = false;
        ISs_used[reason_iset] = false;
    }
}

inline void Solver::reset_context_for_maxsatz() {
    for (int node : fixed_node_stack) {
        // G.size() == is_processed.size()
        if (node >= is_processed.size()) is_processed_new[node-is_processed.size()] = false;
        else is_processed.reset(node);
    }
    fixed_node_stack.clear();

    for (int i : passive_iset_stack) {
        ISs_state[i] = true;
    }
    passive_iset_stack.clear();

    for (int i : reduced_iset_stack) ISs_size[i]++;
    reduced_iset_stack.clear();
}

inline void Solver::rollback_context_for_maxsatz(
    int reduced_iset_start,
    int passive_iset_start,
    int fixed_iset_start
) {
    for (int i = fixed_iset_start; i < fixed_node_stack.size(); i++) {
        auto node = fixed_node_stack[i];
        // G.size() == is_processed.size()
        if (node >= is_processed.size()) is_processed_new[node-is_processed.size()] = false;
        else is_processed.reset(node);
    }
    fixed_node_stack.resize(fixed_iset_start);

    for (int i = passive_iset_start; i < passive_iset_stack.size(); i++) {
        ISs_state[passive_iset_stack[i]] = true;
    }
    passive_iset_stack.resize(passive_iset_start);

    for (auto i = reduced_iset_start; i < reduced_iset_stack.size(); i++) ISs_size[reduced_iset_stack[i]]++;
    reduced_iset_stack.resize(reduced_iset_start);
}

inline int Solver::simple_further_test_node(
    const int start,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class
) {
    int saved_reduced_iset_stack_fill_pointer = reduced_iset_stack.size();
    int saved_passive_iset_stack_fill_pointer = passive_iset_stack.size();
    int saved_fixed_node_stack_fill_pointer = fixed_node_stack.size();
    int my_saved_reduced_iset_stack_fill_pointer = reduced_iset_stack.size();
    int my_saved_passive_iset_stack_fill_pointer = passive_iset_stack.size();
    int my_saved_fixed_node_stack_fill_pointer = fixed_node_stack.size();

    for (auto i = start; i < reduced_iset_stack.size(); i++) ISs_tested[reduced_iset_stack[i]] = false;

    bool conflict = false;
    int chosen_iset = NONE;
    for (int i = start; i < reduced_iset_stack.size(); i++) {
        chosen_iset = reduced_iset_stack[i];
        // we only consider reduced isets
        if (!ISs_state[chosen_iset] || ISs_tested[chosen_iset] || ISs_size[chosen_iset] != 2) continue;

        ISs_tested[chosen_iset] = true;
        custom_bitset::DIFF(nodes, ISs[chosen_iset], is_processed);
        for (auto node : nodes) {
            unit_stack2.clear();

            auto empty_iset = fix_oldNode_for_iset(node, chosen_iset, G, unit_stack2, color_class);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, unit_stack2, color_class);

            rollback_context_for_maxsatz(my_saved_reduced_iset_stack_fill_pointer, my_saved_passive_iset_stack_fill_pointer, my_saved_fixed_node_stack_fill_pointer);

            if (empty_iset == NONE) continue;

            is_processed.set(node);
            reason[node] = NONE;
            fixed_node_stack.push_back(node);
            ISs_size[chosen_iset]--;
            reduced_iset_stack.push_back(chosen_iset);

            assert(ISs_size[chosen_iset] == 1);

            // unit iset
            unit_stack2.clear();
            unit_stack2.push_back(chosen_iset);

            if (unit_iset_process_used_first(G, ISs, unit_stack2, color_class) != NONE) {
                conflict = true;
                break;
            }

            for (auto j = my_saved_reduced_iset_stack_fill_pointer; j < reduced_iset_stack.size(); j++) {
                ISs_tested[reduced_iset_stack[j]] = false;
            }

            my_saved_reduced_iset_stack_fill_pointer = reduced_iset_stack.size();
            my_saved_passive_iset_stack_fill_pointer = passive_iset_stack.size();
            my_saved_fixed_node_stack_fill_pointer = fixed_node_stack.size();
        }
        if (conflict == true) break;
    }

    rollback_context_for_maxsatz(saved_reduced_iset_stack_fill_pointer, saved_passive_iset_stack_fill_pointer, saved_fixed_node_stack_fill_pointer);

    if (conflict) return chosen_iset;
    return NONE;
}

inline int Solver::test_node_for_failed_nodes(
    const int node,
    const int iset,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class
) {
    int saved_reduced_iset_stack_fill_pointer = reduced_iset_stack.size();
    int saved_passive_iset_stack_fill_pointer = passive_iset_stack.size();
    int saved_fixed_node_stack_fill_pointer = fixed_node_stack.size();

    unit_stack2.clear();

    auto empty_iset = fix_oldNode_for_iset(node, iset, G, unit_stack2, color_class);
    if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, unit_stack2, color_class);
    if (empty_iset == NONE) empty_iset = simple_further_test_node(saved_reduced_iset_stack_fill_pointer, G, ISs, color_class);

    rollback_context_for_maxsatz(saved_reduced_iset_stack_fill_pointer, saved_passive_iset_stack_fill_pointer, saved_fixed_node_stack_fill_pointer);
    return empty_iset;
}

inline bool Solver::test_by_eliminate_failed_nodes(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class,
    const int k
) {
    int false_flag = 0;

    reduced_iset_stack.clear();
    passive_iset_stack.clear();
    fixed_node_stack.clear();

    do {
        false_flag = 0;
        for (auto my_iset = k; my_iset >= 0; my_iset--) {
            if (!ISs_state[my_iset]) continue;
            unit_stack.clear();
            custom_bitset::DIFF(nodes2, ISs[my_iset], is_processed);

            for (auto node : nodes2) {
                if (test_node_for_failed_nodes(node, my_iset, G, ISs, color_class) == NONE) continue;

                is_processed.set(node);
                reason[node] = NONE;
                fixed_node_stack.push_back(node);
                false_flag++;
                ISs_size[my_iset]--;
                reduced_iset_stack.push_back(my_iset);
                if (ISs_size[my_iset] == 1) {
                    unit_stack.push_back(my_iset);
                    break;
                } else if (ISs_size[my_iset] == 0) {
                    reset_context_for_maxsatz();
                    return true;
                }
            }

            if (unit_stack.size() > 0 &&
                unit_iset_process_used_first(G, ISs, unit_stack, color_class) != NONE) {
                reset_context_for_maxsatz();
                return true;
            }
        }
	} while (false_flag > 1);

    reset_context_for_maxsatz();

    return false;
}

inline int Solver::further_test_reduced_iset(
    const int start,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class
) {
    int saved_reduced_iset_stack_fill_pointer = reduced_iset_stack.size();
    int saved_passive_iset_stack_fill_pointer = passive_iset_stack.size();
    int saved_fixed_node_stack_fill_pointer = fixed_node_stack.size();

    for (auto i = start; i < reduced_iset_stack.size(); i++) ISs_tested[reduced_iset_stack[i]] = false;

    int chosen_iset = 0;
    for (int i = start; i < reduced_iset_stack.size(); i++) {
        chosen_iset = reduced_iset_stack[i];
        // we only consider reduced isets
        if (!ISs_state[chosen_iset] || ISs_tested[chosen_iset] || ISs_size[chosen_iset] != 2) continue;

        ISs_tested[chosen_iset] = true;

        bool has_new_node = false;
        for (auto node : ISs_new[chosen_iset]) {
            if (!is_processed_new[node-G.size()]) {
                has_new_node = true;
                break;
            }
        }
        if (has_new_node) continue;

        custom_bitset::DIFF(nodes2, ISs[chosen_iset], is_processed);
        std::size_t node = custom_bitset::npos;
        for (node = nodes2.front(); node != custom_bitset::npos; node = nodes2.next(node)) {
            unit_stack3.clear();

            auto empty_iset = fix_oldNode_for_iset(node, chosen_iset, G, unit_stack3, color_class);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, unit_stack3, color_class);

            if (empty_iset == NONE) {
                rollback_context_for_maxsatz(saved_reduced_iset_stack_fill_pointer, saved_passive_iset_stack_fill_pointer, saved_fixed_node_stack_fill_pointer);
                break;
            }

            ISs_involved[chosen_iset] = true;
            identify_conflict_isets(empty_iset, ISs);
            ISs_involved[chosen_iset] = false;

            rollback_context_for_maxsatz(saved_reduced_iset_stack_fill_pointer, saved_passive_iset_stack_fill_pointer, saved_fixed_node_stack_fill_pointer);
        }
        if (node == custom_bitset::npos) return chosen_iset;
    }
    return NONE;
}

inline int Solver::inc_maxsatz_lookahead_by_fl2(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class
) {
	int sn = fixed_node_stack.size();
	int sp = passive_iset_stack.size();
	int sr = reduced_iset_stack.size();
    int rs = reason_stack.size();

    for (auto i = 0; i < sr; i++) ISs_tested[reduced_iset_stack[i]] = false;

    for (auto i = 0; i < sr; i++) {
        int iset_idx = reduced_iset_stack[i];
        if (ISs_tested[iset_idx] || !ISs_state[iset_idx] || ISs_size[iset_idx] != 2) continue;

        reason_stack.resize(rs);
        ISs_tested[iset_idx] = true;
        custom_bitset::DIFF(nodes, ISs[iset_idx], is_processed);
        bool exit = false;
        for (auto node : nodes) {
            unit_stack2.clear();

            auto empty_iset = fix_oldNode_for_iset(node, iset_idx, G, unit_stack2, color_class);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, unit_stack2, color_class);
            if (empty_iset == NONE) {
                bool is_last_node = (ISs_new[iset_idx].empty() && nodes.next(node) == custom_bitset::npos);
                if (is_last_node) empty_iset = further_test_reduced_iset(sr, G, ISs, color_class);
            }

            if (empty_iset == NONE) {
                rollback_context_for_maxsatz(sr, sp, sn);
                exit = true;
                break;
            }
            identify_conflict_isets(empty_iset, ISs);
            rollback_context_for_maxsatz(sr, sp, sn);
        }
        bool node_is_last = !exit;
        if (!exit) {
            node_is_last = false;
            int j;
            for (j = 0; j < ISs_new[iset_idx].size(); j++) {
                //if (!is_node_active[node]) continue;
                auto node = ISs_new[iset_idx][j];
                if (is_processed_new[node-G.size()]) continue;

                unit_stack2.clear();

                auto empty_iset = fix_newNode_for_iset(node, iset_idx, G, unit_stack2);
                if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, unit_stack2, color_class);
                if (empty_iset == NONE) {
                    bool is_last_node = (j+1 == ISs_new[iset_idx].size());
                    if (is_last_node) empty_iset = further_test_reduced_iset(sr, G, ISs, color_class);
                }

                if (empty_iset == NONE) {
                    rollback_context_for_maxsatz(sr, sp, sn);
                    break;
                }
                identify_conflict_isets(empty_iset, ISs);
                rollback_context_for_maxsatz(sr, sp, sn);
            }
            node_is_last = j == ISs_new[iset_idx].size();
        }

        if (node_is_last) return true;

        for (auto j = rs; j < reason_stack.size(); j++) {
            ISs_involved[reason_stack[j]] = false;
            ISs_used[reason_stack[j]] = false;
        }
        reason_stack.resize(rs);
    }
    return false;
}

inline bool Solver::inc_maxsatz_on_last_iset(
    const int ADDED_NODE,
    const custom_graph& G,
    custom_bitset& B,
    const std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class,
    const int k
) {
    reason_stack.clear();
    reduced_iset_stack.clear();
    passive_iset_stack.clear();
    fixed_node_stack.clear();

    for (const auto bi : ISs[k]) {
        unit_stack2.clear();

        // fix old node
        auto empty_iset = fix_oldNode_for_iset(bi, k, G, unit_stack2, color_class);
        if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, unit_stack2, color_class);
        if (empty_iset == NONE) {
            empty_iset = unit_iset_process(G, ISs, unit_stack2, color_class);
        }

        // conflict found
        if (empty_iset != NONE) {
            identify_conflict_isets(empty_iset, ISs);
            reset_context_for_maxsatz();
            B.reset(bi);
            continue;
        }

        // conflict not found
        if (inc_maxsatz_lookahead_by_fl2(G, ISs, color_class)) {
            reset_context_for_maxsatz();
            B.reset(bi);
        } else {
            reset_context_for_maxsatz();
            return false;
        }
    }

    enlarge_conflict_sets(ADDED_NODE, G);

    return true;
}

inline bool Solver::SATCOL(
    const custom_graph& G,
    custom_bitset& B,
    std::vector<custom_bitset>& ISs,
    std::vector<int>& color_class,
    int k
) {
    unit_stack.clear();

    // is_processed = true for nodes not considered
    is_processed.set();

    for (int i = 0; i < k; i++) {
        // is_processed = true for nodes not considered
        is_processed -= ISs[i];
        ISs_size[i] = ISs[i].count();
        ISs_new[i].clear();
        if (ISs_size[i] == 1) {
            unit_stack.push_back(i);
        }
    }

    int ADDED_NODE = G.size();

    for (int i = 0; i < k; i++) {
        ISs_used[i] = false;
        assert(ISs_involved[i] == false);
    }

    do {
        ISEQ_one(G, B, ISs[k]);
        ISs_involved[k] = false;
        ISs_used[k] = false;
        ISs_state[k] = true;
        ISs_new[k].clear();
        ISs_size[k] = ISs[k].count();
        if (ISs_size[k] == 1) {
            unit_stack.push_back(k);
        }
        is_processed -= ISs[k];
        for (auto v : ISs[k]) {
            color_class[v] = k;
        }

        for (int i = 0; i <= k; i++) {
            assert(ISs_used[i] == false);
            assert(ISs_involved[i] == false);
        }

        // B is an IS
        if (B.count() == ISs_size[k] && test_by_eliminate_failed_nodes(G, ISs, color_class, k)) return true;
        if (!inc_maxsatz_on_last_iset(ADDED_NODE, G, B, ISs, color_class, k)) return false;

        ADDED_NODE++;
        k++;
        if (ISs.size() <= k) ISs.emplace_back(G.size());
    } while (B.any());

    return true;
}

inline int Solver::FiltCOL(
    const custom_graph& G,  // graph
    custom_bitset& V, // vertices set
    const std::vector<custom_bitset>& ISs,
    std::vector<custom_bitset>& ISs_t,
    const std::vector<int>& color_class,
    std::vector<int>& color_class_t,
    const fixed_vector<int>& alpha,
    const int k_max
) {
    nodes = V;

    for (auto i = 0; i < k_max; ++i) {
        auto v = nodes.front();
        // we can't build k+1 IS => we can't improve the solution
        if (v == custom_bitset::npos) return i;

        const int k = color_class[v];
        ISs_mapping[i] = k;

        custom_bitset::DIFF(ISs_t[i], nodes, G.get_neighbor_set(v));
        color_class_t[v] = i;
        nodes.reset(v);

        for (v = ISs_t[i].next(v); v != custom_bitset::npos; v = ISs_t[i].next(v)) {
            // if v isn't part of the original IS
            if (!ISs[k].test(v)) {
                // v will not be part of the current IS
                ISs_t[i].reset(v);
                // if v is greater than every other vertices in the original IS
                // TODO: doesn't work properly.
                if (v > alpha[k]) {
                    // removed from V
                    V.reset(v);
                    // removed from future iterations
                    nodes.reset(v);
                }
                // otherwise kept for next iterations
            } else {
                // at most, we can remove vertices, so we don't need to start a new scan
                ISs_t[i] -= G.get_neighbor_set(v);
                color_class_t[v] = i;
                nodes.reset(v);
            }
        }
        //alpha[k] = last_v;
    }
    return k_max;
}

inline bool Solver::test_by_eliminate_failed_nodes2(
    custom_bitset& V,
    const custom_graph& G,
    std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class,
    const int k
) {
    reduced_iset_stack.clear();
    passive_iset_stack.clear();
    fixed_node_stack.clear();

    for (auto my_iset = k; my_iset >= 0; my_iset--) {
        for (auto node : ISs[my_iset]) {
            if (test_node_for_failed_nodes(node, my_iset, G, ISs, color_class) == NONE) continue;

            V.reset(node);
            ISs[my_iset].reset(node);
            ISs_size[my_iset]--;
            is_processed.set(node);

            if (ISs_size[my_iset] == 0) return true;
        }
    }

    return false;
}

inline bool Solver::FiltSAT(
    const custom_graph& G,  // graph
    custom_bitset& V, // vertices set
    std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class,
    const int k_max
) {
    is_processed.set();
    for (int i = 0; i < k_max; i++) {
        is_processed -= ISs[i];
        ISs_state[i] = true;
        ISs_size[i] = ISs[i].count();
        ISs_used[i] = false;
        ISs_new[i].clear();
        assert(ISs_involved[i] == false);
    }

    // we apply FL (Failed Literal) to every vertex of each IS of Ct-alpha
    return test_by_eliminate_failed_nodes2(V, G, ISs, color_class, k_max-1);
}

inline void Solver::FindMaxClique(
    const custom_graph& G,  // graph
    fixed_vector<int>& K,       // current branch
    solution<int>& K_max,   // max branch
    custom_bitset& P_Bj, // vertices set
    const custom_bitset& B,       // branching set
    std::vector<int>& u, // incremental upper bounds,
    const std::chrono::time_point<std::chrono::steady_clock> max_time,
    thread_pool<Solver>& pool,
    size_t sequence,
    const fixed_vector<int>& alpha, // incremental upper bounds,
    const bool is_k_partite,
    const std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class
) {
    steps++;

    // |K|+1 because we are yet to add the vertex to the current solution
    const int depth = K.size()+1;

    while (_ISs.size() <= K_max.size()) _ISs.emplace_back(G.size());

    for (auto bi : B) {
        if (std::chrono::steady_clock::now() > max_time) {
            pool.stop_threads = true;
            return;
        }
        if (pool.stop_threads) {
            return;
        }

        const int lb = K_max.size();
        P_Bj.set(bi);

        if (u[bi] + K.size() <= lb) {
            // by resetting u[bi] (without calculating it) we allow to be
            // reconsidered in future iterations if necessary
            //u[bi] = static_cast<int>(K_max.size() - K.size());
            if (depth == 2) {
                u[bi] = 1;

                for (auto neighbor = P_Bj.prev(bi); neighbor != custom_bitset::npos; neighbor = P_Bj.prev(neighbor)) {
                    u[bi] = std::max(u[bi], 1+u[neighbor]);
                    // no point continue searching, we will overwrite this anyway with a potentially lower value
                    if (u[bi] + K.size() > lb) break;
                }
                u[bi] = std::min(u[bi], lb - (int)K.size());
            }

            pruned++;
            continue;
        }

        // if bi == 0, u[bi] always == 1!
        u[bi] = 1;

        for (auto neighbor = P_Bj.prev(bi); neighbor != custom_bitset::npos; neighbor = P_Bj.prev(neighbor)) {
            u[bi] = std::max(u[bi], 1+u[neighbor]);
            // no point continue searching, we will overwrite this anyway with a potentially lower value
            if (u[bi] + K.size() > lb) break;
        }

        //u[bi] = std::min(u[bi], lb-curr);
        // so if we enter in the following if, we don't overwrite it, otherwise we will replace u[bi] with lb-curr

        // if we can't improve, we prune the current branch
        // curr-1 because bi is not part of K yet
        // it goes into the pruned set

        if (u[bi] + K.size() <= lb) {
            pruned++;
            continue;
        }

        // calculate sub-problem
        custom_bitset::AND(V_new, P_Bj, G.get_neighbor_set(bi));
        int V_new_size = V_new.count();

        u[bi] = std::min(u[bi], V_new_size+1);
        if (u[bi] + K.size() <= lb) {
            pruned++;
            continue;
        }

        // if we are in a leaf
        if (V_new_size == 0) {
            if (K_max.update_solution(K, bi)) {
                //std::cout << "Last incumbent: " << K_max.size() << std::endl;
                // we can return because it's an incremental branching scheme, we can add only one vertex at a time
                pool.stop_threads = true;
                return;
            }
            continue;
        }

        const int k = lb-depth;
        u[bi] = k+1;

        int next_is_k_partite = is_k_partite;

        size_t new_alpha_idx = pool.borrow_alpha();
        fixed_vector<int>& new_alpha = pool.get_alpha(new_alpha_idx);

        size_t B_new_idx = pool.borrow_bitset();
        custom_bitset& B_new = pool.get_bitset(B_new_idx);
        assert(B_new.size() == G.size());

        // if is a k+1 partite graph
        if (is_k_partite) {
            const auto n_isets = FiltCOL(G, V_new, ISs, _ISs, color_class, _color_class, alpha, k+1);
            if (n_isets < k+1) {
                u[bi] = n_isets+1;
                pool.give_back_alpha(new_alpha_idx);
                pool.give_back_bitset(B_new_idx);
                continue;
            }

            if (FiltSAT(G, V_new, _ISs, _color_class, k+1)) {
                pool.give_back_alpha(new_alpha_idx);
                pool.give_back_bitset(B_new_idx);
                continue;
            }
            new_alpha.resize(k+1);
            for (int i = 0; i < k+1; i++) {
                new_alpha[i] = _ISs[ISs_mapping[i]].back();
            }
            B_new.copy_same_size(_ISs[k]);
            assert(B_new.any());
        } else {
            const auto n_isets = ISEQ_branching(G, V_new, _ISs, _color_class, k);
            if (n_isets < k+1) {
                u[bi] = n_isets+1;
                pool.give_back_alpha(new_alpha_idx);
                pool.give_back_bitset(B_new_idx);
                continue;
            }
            assert(_ISs[k].any());

            //if (B_new.none()) continue;
            if (is_IS(G, _ISs[k])) {
                next_is_k_partite = true;
                // if we could return here, huge gains... damn
                if (FiltSAT(G, V_new, _ISs, _color_class, k+1)) {
                    pool.give_back_alpha(new_alpha_idx);
                    pool.give_back_bitset(B_new_idx);
                    continue;
                }
                new_alpha.resize(k+1);
                for (int i = 0; i < k+1; i++) {
                    new_alpha[i] = _ISs[i].back();
                }
                B_new.copy_same_size(_ISs[k]);
            } else {
                B_new.copy_same_size(_ISs[k]);
                if (SATCOL(G, B_new, _ISs, _color_class, k)) {
                    pool.give_back_alpha(new_alpha_idx);
                    pool.give_back_bitset(B_new_idx);
                    continue;
                }
            }
        }

        // at this point B is not empty
        K.push_back(bi);
        size_t new_P_Bj_idx = pool.borrow_bitset();
        custom_bitset& new_P_Bj = pool.get_bitset(new_P_Bj_idx);
        assert(new_P_Bj.size() == G.size());
        custom_bitset::DIFF(new_P_Bj, V_new, B_new);

        size_t local_u_idx = pool.borrow_u();
        std::vector<int>& local_u = pool.get_u(local_u_idx);
        for (int i = 0; i < u.size(); i++) local_u[i] = u[i];

        /*
        if (pool.is_queue_full()) {
            size_t new_sequence = pool.get_new_sequence();
            if (next_is_k_partite && color_class.empty()) {
                size_t local_ISs_idx = pool.borrow_ISs();
                std::vector<custom_bitset>& local_ISs = pool.get_ISs(local_ISs_idx);
                while (local_ISs.size() <= K_max.size()) local_ISs.emplace_back(G.size());
                std::swap(local_ISs, _ISs);

                size_t local_color_class_idx = pool.borrow_color_class();
                std::vector<int>& local_color_class = pool.get_color_class(local_color_class_idx);
                std::swap(local_color_class, _color_class);

                FindMaxClique(G, K, K_max, new_P_Bj, B_new, local_u, max_time, pool, new_sequence, new_alpha, next_is_k_partite, local_ISs, local_color_class);
                pool.give_back_alpha(new_alpha_idx);
                pool.give_back_bitset(B_new_idx);
                pool.give_back_bitset(new_P_Bj_idx);
                pool.give_back_ISs(local_ISs_idx);
                pool.give_back_color_class(local_color_class_idx);
                pool.give_back_u(local_u_idx);
            } else {
                FindMaxClique(G, K, K_max, new_P_Bj, B_new, local_u, max_time, pool, new_sequence, new_alpha, next_is_k_partite, ISs, color_class);
                pool.give_back_alpha(new_alpha_idx);
                pool.give_back_bitset(B_new_idx);
                pool.give_back_bitset(new_P_Bj_idx);
                pool.give_back_u(local_u_idx);
            }
        } else {
        */
        size_t local_K_idx = pool.borrow_K();
        fixed_vector<int>& local_K = pool.get_K(local_K_idx);
        local_K.resize(K.size());
        for (int i = 0; i < K.size(); i++) local_K[i] = K[i];

        if (next_is_k_partite) {
            size_t local_ISs_idx = pool.borrow_ISs();
            std::vector<custom_bitset>& local_ISs = pool.get_ISs(local_ISs_idx);
            while (local_ISs.size() <= K_max.size()) local_ISs.emplace_back(G.size());
            std::swap(local_ISs, _ISs);

            size_t local_color_class_idx = pool.borrow_color_class();
            std::vector<int>& local_color_class = pool.get_color_class(local_color_class_idx);
            std::swap(local_color_class, _color_class);

            pool.submit(depth, [new_alpha_idx, B_new_idx, new_P_Bj_idx, local_ISs_idx, local_color_class_idx, local_K_idx, local_u_idx, &G, &K_max, &new_P_Bj, &B_new, max_time, &pool, &new_alpha, next_is_k_partite, &local_ISs, &local_color_class, &local_K, &local_u](Solver& solver, size_t sequence) {
                if (!pool.stop_threads) solver.FindMaxClique(G, local_K, K_max, new_P_Bj, B_new, local_u, max_time, pool, sequence, new_alpha, next_is_k_partite, local_ISs, local_color_class);
                pool.give_back_alpha(new_alpha_idx);
                pool.give_back_bitset(B_new_idx);
                pool.give_back_bitset(new_P_Bj_idx);
                pool.give_back_ISs(local_ISs_idx);
                pool.give_back_color_class(local_color_class_idx);
                pool.give_back_K(local_K_idx);
                pool.give_back_u(local_u_idx);
            });
        } else {
            pool.submit(depth, [new_alpha_idx, B_new_idx, new_P_Bj_idx, local_K_idx, local_u_idx, &G, &K_max, &new_P_Bj, &B_new, max_time, &pool, &new_alpha, next_is_k_partite, &local_K, &local_u](Solver& solver, size_t sequence) {
                if (!pool.stop_threads) solver.FindMaxClique(G, local_K, K_max, new_P_Bj, B_new, local_u, max_time, pool, sequence, new_alpha, next_is_k_partite);
                pool.give_back_alpha(new_alpha_idx);
                pool.give_back_bitset(B_new_idx);
                pool.give_back_bitset(new_P_Bj_idx);
                pool.give_back_K(local_K_idx);
                pool.give_back_u(local_u_idx);
            });
        }
        //}
        K.pop_back();

        // to avoid task starvation
        thread_pool<Solver>::Task task;
        while (pool.is_queue_full() && pool.get_higher_priority_task(task, depth-1, sequence)) task.func(*this, task.sequence);

        // if (new_solution_found) return true;

        // u[bi] cannot be lowered
        //u[bi] = std::min(u[bi], static_cast<int>(K_max.size() - K.size()));
    }
}

std::vector<int> CliSAT_no_sorting(const custom_graph& G, thread_pool<Solver>& pool, const custom_bitset& Ubb, std::chrono::milliseconds time_limit);

std::vector<int> CliSAT(
    const std::string& filename,
    std::chrono::milliseconds time_limit,
    std::chrono::milliseconds cs_time_limit,
    bool MISP,
    SORTING_METHOD sorting_method,
    bool AMTS_enabled,
    size_t threads,
    bool verbose);
