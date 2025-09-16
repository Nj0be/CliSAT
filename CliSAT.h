//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <cassert>
#include <stack>
#include <iostream>

#include "coloring.h"
#include "custom_bitset.h"
#include "custom_graph.h"

#define NONE -1
#define NO_REASON -1

static void identify_conflict_isets(
    const int iset,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    std::vector<bool>& ISs_involved,
    std::vector<bool>& ISs_used,
    const custom_bitset& is_processed,
    std::vector<bool>& is_processed_new,
    std::vector<int>& reason_stack,
    const std::vector<int>& reason
) {
    int starting_index = reason_stack.size();
    reason_stack.push_back(iset);
    ISs_involved[iset] = true;
    for (int i = starting_index; i < reason_stack.size(); i++) {
        const auto reason_iset = reason_stack[i];

        // removed_nodes
        static custom_bitset removed(is_processed.size());
        custom_bitset::AND(removed, ISs[reason_iset], is_processed);
        for (auto r : removed) {
            auto r_iset = reason[r];
            if (ISs_involved[r_iset]) continue;

            ISs_involved[r_iset] = true;
            reason_stack.push_back(r_iset);
        }
        for (auto r : ISs_new[reason_iset]) {
            // not processed (removed)
            if (!is_processed_new[r-is_processed.size()] || reason[r] == NO_REASON) continue;

            auto r_iset = reason[r];
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

static bool remove_neighbors_from_iset(
    const int l_is,
    const int iset,
    const custom_bitset& neighbors,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    custom_bitset& is_processed,
    std::vector<int>& reason,
    std::vector<int>& unit_stack
) {
    static custom_bitset removed(G.size());
    custom_bitset::DIFF(removed, ISs[iset], neighbors);

    // non processed vertices
    auto count_removed = 0;
    for (auto r : removed) {
        count_removed++;

        ISs_size[iset]--;
        REDUCED_iSET_STACK.push_back(iset);
        is_processed.set(r);
        reason[r] = l_is;
    }

    // we did nothing
    if (count_removed == 0) return false;

    if (ISs_size[iset] == 1) {
        unit_stack.emplace_back(iset);
        return false;
    }
    // conflict found
    if (ISs_size[iset] == 0) return true;

    return false;
}

static int fix_newNode_for_iset(
    const int fix_node,
    const int fix_iset,
    const custom_graph& G,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    std::vector<bool>& is_processed_new,
    std::vector<int>& reason,
    std::vector<int>& unit_stack
) {
    int idx;
    ISs_size[fix_iset]--;
    REDUCED_iSET_STACK.push_back(fix_iset);
    ISs_state[fix_iset] = false;
    is_processed_new[fix_node-G.size()] = true;
    reason[fix_node] = fix_iset;

    for (auto iset_idx : CONFLICT_ISET_STACK[fix_node-G.size()]) {
        if (!ISs_state[iset_idx]) continue;

        ISs_size[iset_idx]--;
        REDUCED_iSET_STACK.push_back(iset_idx);

        if (ISs_size[iset_idx] == 0) return iset_idx;
        if (ISs_size[iset_idx] == 1) unit_stack.push_back(iset_idx);
    }
    return NONE;
}

static int fix_oldNode_for_iset(
    const int fix_node,
    const int fix_iset,
    const custom_bitset& neighbors,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    custom_bitset& is_processed,
    std::vector<int>& reason,
    std::vector<int>& unit_stack,
    const int k
) {
    ISs_size[fix_iset]--;
    REDUCED_iSET_STACK.push_back(fix_iset);
    ISs_state[fix_iset] = false;
    is_processed.set(fix_node);
    reason[fix_node] = fix_iset;

    for (int iset = 0; iset < k; iset++) {
        if (!ISs_state[iset]) continue;

        if (remove_neighbors_from_iset(fix_iset, iset, neighbors, G, ISs, ISs_size, REDUCED_iSET_STACK, is_processed, reason, unit_stack)) {
            return iset;
        }
    }
    return NONE;
}

inline int get_node_of_unit_iset(
    const int l_is,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const custom_bitset& is_processed,
    const std::vector<bool>& is_processed_new
) {
    for (auto new_node : ISs_new[l_is]) {
        if (is_processed_new[new_node-G.size()]) continue;
        return new_node;
    }

    auto node  = ISs[l_is].front_difference(is_processed);
    if (node != custom_bitset::npos) return node;

    std::cout << "Error in get_node_of_unit_iset: l_is{" << l_is << "} node{" << node << "}" << std::endl;
    exit(1);
}

static int fix_anyNode_for_iset(
    const int fix_iset,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    custom_bitset& is_processed,
    std::vector<bool>& is_processed_new,
    std::vector<int>& reason,
    std::vector<int>& unit_stack,
    const int k
) {
    static custom_bitset neighbors(G.size());

    const auto fix_node = get_node_of_unit_iset(fix_iset, G, ISs, ISs_new, is_processed, is_processed_new);

    if (fix_node >= G.size()) return fix_newNode_for_iset(fix_node, fix_iset, G, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed_new, reason, unit_stack);

    custom_bitset::OR(neighbors, G.get_neighbor_set(fix_node), is_processed);
    return fix_oldNode_for_iset(fix_node, fix_iset, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, reason, unit_stack, k);
}

static int unit_iset_process(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    custom_bitset& is_processed,
    std::vector<bool>& is_processed_new,
    std::vector<int>& reason,
    const std::vector<int>& unit_stack,
    const int k
) {
    static std::vector<int> new_unit_stack;
    static custom_bitset neighbors(G.size());

    for (const auto l_is : unit_stack) {
        if (!ISs_state[l_is] || ISs_size[l_is] != 1) continue;

        new_unit_stack.clear();

        auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, reason, new_unit_stack, k);
        if (empty_iset != NONE) return empty_iset;

        for (auto j = 0; j < new_unit_stack.size(); j++) {
            const auto l_is2 = new_unit_stack[j];
            if (!ISs_state[l_is2]) continue;

            empty_iset = fix_anyNode_for_iset(l_is2, G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, reason, new_unit_stack, k);
            if (empty_iset != NONE) return empty_iset;
        }
    }

    return NONE;
}

static int unit_iset_process_used_first(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    custom_bitset& is_processed,
    std::vector<bool>& is_processed_new,
    std::vector<bool>& ISs_used,
    std::vector<int>& reason,
    std::vector<int>& unit_stack,
    const int k
) {
    static custom_bitset neighbors(G.size());

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
            const auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, reason, unit_stack, k);
            if (empty_iset != NONE) return empty_iset;
        }

        for (j = iset_start; j < unit_stack.size(); j++) {
            const auto l_is = unit_stack[j];

            if (!ISs_state[l_is]) continue;
            assert(ISs_size[l_is] == 1);

            const auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, reason, unit_stack, k);
            if (empty_iset != NONE) return empty_iset;

            iset_start = j+1;
            break;
        }
    } while (j < unit_stack.size());

    return NONE;
}

static void enlarge_conflict_sets(
    const int ADDED_NODE,
    const custom_graph& G,
    std::vector<std::vector<int>>& ISs_new,
    std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<int>& ISs_size,
    std::vector<bool>& ISs_used,
    std::vector<bool>& ISs_involved,
    std::vector<int>& reason,
    const std::vector<int>& reason_stack,
    std::vector<bool>& is_processed_new
) {
    is_processed_new[ADDED_NODE-G.size()] = false;
    reason[ADDED_NODE] = NO_REASON;
    CONFLICT_ISET_STACK[ADDED_NODE-G.size()].clear();

    for (const auto iset : reason_stack) {
        if (ISs_involved[iset]) continue;

        ISs_involved[iset] = true;
        ISs_new[iset].push_back(ADDED_NODE);
        ISs_size[iset]++;
        CONFLICT_ISET_STACK[ADDED_NODE-G.size()].push_back(iset);
    }

    for (const auto reason_iset : reason_stack) {
        ISs_involved[reason_iset] = false;
        ISs_used[reason_iset] = false;
    }
}

static void reset_context_for_maxsatz(
    const int ADDED_NODE,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    custom_bitset& is_processed,
    std::vector<bool>& is_processed_new,
    std::vector<int>& reason,
    const int k
) {
    for (int i = 0; i < k; i++) {
        //ISs_size[i] = ISs_size_back[i];
        ISs_state[i] = true;
        //ISs_removed[i].reset();
    }
    is_processed.reset();
    for (int i = 0; i < ADDED_NODE; i++) {
        reason[i] = -1;
    }
    // G.size() == is_processed.size()
    for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
        is_processed_new[i] = false;
    }
    for (auto i = 0; i < REDUCED_iSET_STACK.size(); i++) ISs_size[REDUCED_iSET_STACK[i]]++;
    REDUCED_iSET_STACK.resize(0);
}

static void rollback_context_for_maxsatz(
    const int ADDED_NODE,
    std::vector<bool>& ISs_state,
    const std::vector<bool>& ISs_state_back,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    const int reduced_start,
    //const std::vector<int>& ISs_size_back,
    std::vector<int>& reason,
    const std::vector<int>& reason_back,
    const int k
) {
    for (int i = 0; i < k; i++) {
        //ISs_size[i] = ISs_size_back[i];
        ISs_state[i] = ISs_state_back[i];
    }
    for (int i = 0; i < ADDED_NODE; i++) {
        reason[i] = reason_back[i];
    }
    for (auto i = reduced_start; i < REDUCED_iSET_STACK.size(); i++) ISs_size[REDUCED_iSET_STACK[i]]++;
    REDUCED_iSET_STACK.resize(reduced_start);
}

static int simple_further_test_node(
    const int start,
    const int ADDED_NODE,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    std::vector<bool>& ISs_used,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    const custom_bitset& is_processed_prev,
    const std::vector<bool>& is_processed_new_prev,
    std::vector<int>& reason,
    const int k
) {
    static std::vector<bool> ISs_state_back(G.size());
    static std::vector<int> reason_back(G.size()*2);
    static custom_bitset is_processed(G.size());
    static std::vector<bool> is_processed_new(G.size());
    static custom_bitset is_processed_back(G.size());
    static std::vector<bool> is_processed_new_back(G.size());
    static std::vector<bool> ISs_tested(G.size());

    for (int i = 0; i < k; i++) {
        ISs_state_back[i] = ISs_state[i];
    }
    for (int i = 0; i < ADDED_NODE; i++) {
        reason_back[i] = reason[i];
    }

    is_processed = is_processed_prev;
    is_processed_back = is_processed;
    for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
        is_processed_new[i] = is_processed_new_prev[i];
        is_processed_new_back[i] = is_processed_new[i];
    }

    int saved_reduced_iset_stack_fill_pointer = REDUCED_iSET_STACK.size();
    int my_saved_reduced_iset_stack_fill_pointer = REDUCED_iSET_STACK.size();

    for (auto i = start; i < REDUCED_iSET_STACK.size(); i++) ISs_tested[REDUCED_iSET_STACK[i]] = false;

    bool conflict = false;
    int chosen_iset = 0;
    for (int i = start; i < REDUCED_iSET_STACK.size(); i++) {
        chosen_iset = REDUCED_iSET_STACK[i];
        // we only consider reduced isets
        if (!ISs_state[chosen_iset] || ISs_tested[chosen_iset] || ISs_size[chosen_iset] != 2) continue;

        ISs_tested[chosen_iset] = true;
        static custom_bitset nodes(G.size());
        custom_bitset::DIFF(nodes, ISs[chosen_iset], is_processed);
        for (auto node : nodes) {
            static std::vector<int> unit_stack;
            unit_stack.clear();

            static custom_bitset neighbors(G.size());
            custom_bitset::OR(neighbors, G.get_neighbor_set(node), is_processed);
            auto empty_iset = fix_oldNode_for_iset(node, chosen_iset, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, reason, unit_stack, k);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, ISs_used, reason, unit_stack, k);

            is_processed = is_processed_back;
            for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
                is_processed_new[i] = is_processed_new_back[i];
            }
            rollback_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_state_back, ISs_size, REDUCED_iSET_STACK, my_saved_reduced_iset_stack_fill_pointer, reason, reason_back, k);

            if (empty_iset == NONE) continue;

            is_processed.set(node);
            reason[node] = NO_REASON;
            ISs_size[chosen_iset]--;
            REDUCED_iSET_STACK.push_back(chosen_iset);

            assert(ISs_size[chosen_iset] == 1);

            // unit iset
            unit_stack.clear();
            unit_stack.push_back(chosen_iset);

            if (unit_iset_process_used_first(G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, ISs_used, reason, unit_stack, k) != NONE) {
                conflict = true;
                break;
            }

            for (auto j = my_saved_reduced_iset_stack_fill_pointer; j < REDUCED_iSET_STACK.size(); j++) {
                ISs_tested[REDUCED_iSET_STACK[j]] = false;
            }

            is_processed_back = is_processed;
            is_processed_new_back = is_processed_new;
            for (int i = 0; i < k; i++) {
                ISs_state_back[i] = ISs_state[i];
            }
            for (int i = 0; i < ADDED_NODE; i++) {
                reason_back[i] = reason[i];
            }
            my_saved_reduced_iset_stack_fill_pointer = REDUCED_iSET_STACK.size();
        }
        if (conflict == true) break;
    }

    rollback_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_state_back, ISs_size, REDUCED_iSET_STACK, saved_reduced_iset_stack_fill_pointer, reason, reason_back, k);

    if (conflict) return chosen_iset;
    return NONE;
}

static int test_node_for_failed_nodes(
    const int node,
    const int iset,
    const int ADDED_NODE,
    const custom_graph& G,
    custom_bitset& B,
    const custom_bitset& B_new,
    std::vector<custom_bitset>& ISs,
    std::vector<std::vector<int>>& ISs_new,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    std::vector<bool>& ISs_used,
    std::vector<bool>& ISs_involved,
    std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    const custom_bitset& is_processed_prev,
    const std::vector<bool>& is_processed_new_prev,
    std::vector<int>& reason,
    const int k
) {
    static std::vector<int> ISs_size_back(G.size());
    static std::vector<bool> ISs_state_back(G.size());
    static std::vector<int> reason_back(G.size()*2);
    static custom_bitset is_processed(G.size());
    static std::vector<bool> is_processed_new(G.size());

    for (int i = 0; i < k; i++) {
        ISs_size_back[i] = ISs_size[i];
        ISs_state_back[i] = ISs_state[i];
    }
    for (int i = 0; i < ADDED_NODE; i++) {
        reason_back[i] = reason[i];
    }

    is_processed = is_processed_prev;
    for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
        is_processed_new[i] = is_processed_new_prev[i];
    }

    int saved_reduced_iset_stack_fill_pointer = REDUCED_iSET_STACK.size();

    static std::vector<int> unit_stack;
    unit_stack.clear();

    static custom_bitset neighbors(G.size());
    custom_bitset::OR(neighbors, G.get_neighbor_set(node), is_processed);

    auto empty_iset = fix_oldNode_for_iset(node, iset, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, reason, unit_stack, k);
    if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, ISs_used, reason, unit_stack, k);
    if (empty_iset == NONE) empty_iset = simple_further_test_node(saved_reduced_iset_stack_fill_pointer, ADDED_NODE, G, ISs, ISs_new, ISs_state, ISs_size, REDUCED_iSET_STACK, ISs_used, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, k);

    rollback_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_state_back, ISs_size, REDUCED_iSET_STACK, saved_reduced_iset_stack_fill_pointer, reason, reason_back, k);
    return empty_iset;
}

static bool test_by_eliminate_failed_nodes(
    const int ADDED_NODE,
    const custom_graph& G,
    custom_bitset& B,
    const custom_bitset& B_new,
    std::vector<custom_bitset>& ISs,
    std::vector<std::vector<int>>& ISs_new,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<bool>& ISs_used,
    std::vector<bool>& ISs_involved,
    std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<int>& reason,
    const int k
) {
    bool conflict = false;
    int false_flag = 0;
    static std::vector<int> unit_stack;
    static custom_bitset is_processed(G.size());
    static std::vector<bool> is_processed_new(G.size());
    static std::vector<int> REDUCED_iSET_STACK;

    do {
        false_flag = 0;
        for (auto my_iset = k-1; my_iset >= 0; my_iset--) {
            if (!ISs_state[my_iset]) continue;

            conflict = false;
            unit_stack.clear();
            static custom_bitset nodes(G.size());
            custom_bitset::DIFF(nodes, ISs[my_iset], is_processed);
            for (auto node : nodes) {
                if (test_node_for_failed_nodes(node, my_iset, ADDED_NODE, G, B, B_new, ISs, ISs_new, ISs_state, ISs_size, REDUCED_iSET_STACK, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, k) == NONE) continue;

                is_processed.set(node);
                reason[node] = NO_REASON;
                false_flag++;
                ISs_size[my_iset]--;
                REDUCED_iSET_STACK.push_back(my_iset);
                if (ISs_size[my_iset] == 1) {
                    unit_stack.push_back(my_iset);
                    break;
                } else if (ISs_size[my_iset] == 0) {
                    conflict = true;
                    break;
                }
            }

            if (conflict == true) break;

            if (unit_stack.size() > 0 &&
                unit_iset_process_used_first(G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, ISs_used, reason, unit_stack, k) != NONE) {
                conflict = true;
                break;
            }
        }
	} while (false_flag > 1 && conflict == false);

    reset_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, reason, k);

    return conflict;
}

static int further_test_reduced_iset(
    const int start,
    const int ADDED_NODE,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    std::vector<bool>& ISs_used,
    std::vector<bool>& ISs_involved,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    const custom_bitset& is_processed_prev,
    const std::vector<bool>& is_processed_new_prev,
    std::vector<int>& reason,
    std::vector<int>& REASON_STACK,
    const int k
) {
    static std::vector<bool> ISs_state_back(G.size());
    static std::vector<int> reason_back(G.size()*2);
    static custom_bitset is_processed(G.size());
    static std::vector<bool> is_processed_new(G.size());
    static custom_bitset is_processed_back(G.size());
    static std::vector<bool> is_processed_new_back(G.size());
    static std::vector<bool> ISs_tested(G.size());

    for (int i = 0; i < k; i++) {
        ISs_state_back[i] = ISs_state[i];
    }
    for (int i = 0; i < ADDED_NODE; i++) {
        reason_back[i] = reason[i];
    }

    is_processed = is_processed_prev;
    is_processed_back = is_processed;
    for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
        is_processed_new[i] = is_processed_new_prev[i];
        is_processed_new_back[i] = is_processed_new[i];
    }

    int saved_reduced_iset_stack_fill_pointer = REDUCED_iSET_STACK.size();
    int my_saved_reduced_iset_stack_fill_pointer = REDUCED_iSET_STACK.size();

    for (auto i = start; i < REDUCED_iSET_STACK.size(); i++) ISs_tested[REDUCED_iSET_STACK[i]] = false;

    int chosen_iset = 0;
    std::size_t node = custom_bitset::npos;
    for (int i = start; i < REDUCED_iSET_STACK.size(); i++) {
        chosen_iset = REDUCED_iSET_STACK[i];
        // we only consider reduced isets
        if (!ISs_state[chosen_iset] || ISs_tested[chosen_iset] || ISs_size[chosen_iset] != 2) continue;

        ISs_tested[chosen_iset] = true;

        bool has_new_node = false;
        for (int i = 0; i < ISs_new[chosen_iset].size(); i++) {
            node = ISs_new[chosen_iset][i];
            if (!is_processed_new[node-G.size()]) {
                has_new_node = true;
                break;
            }
        }
        if (has_new_node) continue;

        static custom_bitset nodes(G.size());
        custom_bitset::DIFF(nodes, ISs[chosen_iset], is_processed);
        for (node = nodes.front(); node != custom_bitset::npos; node = nodes.next(node)) {
            static std::vector<int> unit_stack;
            unit_stack.clear();

            static custom_bitset neighbors(G.size());
            custom_bitset::OR(neighbors, G.get_neighbor_set(node), is_processed);
            auto empty_iset = fix_oldNode_for_iset(node, chosen_iset, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, reason, unit_stack, k);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, ISs_used, reason, unit_stack, k);

            if (empty_iset == NONE) {
                is_processed = is_processed_back;
                for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
                    is_processed_new[i] = is_processed_new_back[i];
                }
                rollback_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_state_back, ISs_size, REDUCED_iSET_STACK, my_saved_reduced_iset_stack_fill_pointer, reason, reason_back, k);
                break;
            }

            ISs_involved[chosen_iset] = true;
            identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_involved, ISs_used, is_processed, is_processed_new, REASON_STACK, reason);
            ISs_involved[chosen_iset] = false;
            is_processed = is_processed_back;
            for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
                is_processed_new[i] = is_processed_new_back[i];
            }
            rollback_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_state_back, ISs_size, REDUCED_iSET_STACK, my_saved_reduced_iset_stack_fill_pointer, reason, reason_back, k);
        }
        if (node == custom_bitset::npos) return chosen_iset;
    }
    return NONE;
}

static int inc_maxsatz_lookahead_by_fl2(
    const int ADDED_NODE,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    std::vector<bool>& ISs_used,
    std::vector<bool>& ISs_involved,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    custom_bitset& is_processed,
    std::vector<bool>& is_processed_new,
    std::vector<int>& reason,
    std::vector<int>& REASON_STACK,
    const int k
) {
    static std::vector<bool> ISs_state_back(G.size());
    static std::vector<int> reason_back(G.size()*2);
    static custom_bitset is_processed_back(G.size());
    static std::vector<bool> is_processed_new_back(G.size());
    static std::vector<bool> ISs_tested(G.size());

    for (int i = 0; i < k; i++) {
        ISs_state_back[i] = ISs_state[i];
    }
    for (int i = 0; i < ADDED_NODE; i++) {
        reason_back[i] = reason[i];
    }

    is_processed_back = is_processed;
    for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
        is_processed_new_back[i] = is_processed_new[i];
    }

    int rs = REASON_STACK.size();
    int sr = REDUCED_iSET_STACK.size();

    static std::vector<int> unit_stack;
    static std::vector<bool> tested(G.size());

    for (auto i = 0; i < sr; i++) tested[REDUCED_iSET_STACK[i]] = false;

    for (auto i = 0; i < sr; i++) {
        int iset_idx = REDUCED_iSET_STACK[i];
        if (tested[iset_idx] || !ISs_state[iset_idx] || ISs_size[iset_idx] != 2) continue;

        REASON_STACK.resize(rs);
        tested[iset_idx] = true;
        static custom_bitset nodes(G.size());
        custom_bitset::DIFF(nodes, ISs[iset_idx], is_processed);
        bool exit = false;
        for (auto node : nodes) {
            //if (!is_node_active[node]) continue;
            if (is_processed.test(node)) continue;

            unit_stack.clear();
            static custom_bitset neighbors(G.size());
            custom_bitset::OR(neighbors, G.get_neighbor_set(node), is_processed);
            auto empty_iset = fix_oldNode_for_iset(node, iset_idx, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, reason, unit_stack, k);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, ISs_used, reason, unit_stack, k);
            if (empty_iset == NONE) {
                bool is_last_node = (ISs_new[iset_idx].size() == 0 && nodes.next(node) == custom_bitset::npos);
                if (is_last_node) empty_iset = further_test_reduced_iset(sr, ADDED_NODE, G, ISs, ISs_new, ISs_state, ISs_size, REDUCED_iSET_STACK, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, REASON_STACK, k);
            }

            if (empty_iset == NONE) {
                is_processed = is_processed_back;
                for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
                    is_processed_new[i] = is_processed_new_back[i];
                }
                rollback_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_state_back, ISs_size, REDUCED_iSET_STACK, sr, reason, reason_back, k);
                exit = true;
                break;
            }
            identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_involved, ISs_used, is_processed, is_processed_new, REASON_STACK, reason);
            is_processed = is_processed_back;
            for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
                is_processed_new[i] = is_processed_new_back[i];
            }
            rollback_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_state_back, ISs_size, REDUCED_iSET_STACK, sr, reason, reason_back, k);
        }
        bool node_is_last = !exit;
        if (!exit) {
            node_is_last = false;
            int j;
            for (j = 0; j < ISs_new[iset_idx].size(); j++) {
                //if (!is_node_active[node]) continue;
                auto node = ISs_new[iset_idx][j];
                if (is_processed_new[node-G.size()]) continue;

                unit_stack.clear();
                auto empty_iset = fix_newNode_for_iset(node, iset_idx, G, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed_new, reason, unit_stack);
                if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, ISs_used, reason, unit_stack, k);
                if (empty_iset == NONE) {
                    bool is_last_node = (j+1 == ISs_new[iset_idx].size());
                    if (is_last_node) empty_iset = further_test_reduced_iset(sr, ADDED_NODE, G, ISs, ISs_new, ISs_state, ISs_size, REDUCED_iSET_STACK, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, REASON_STACK, k);
                }

                if (empty_iset == NONE) {
                    is_processed = is_processed_back;
                    for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
                        is_processed_new[i] = is_processed_new_back[i];
                    }
                    rollback_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_state_back, ISs_size, REDUCED_iSET_STACK, sr, reason, reason_back, k);
                    break;
                }
                identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_involved, ISs_used, is_processed, is_processed_new, REASON_STACK, reason);
                is_processed = is_processed_back;
                for (int i = 0; i < ADDED_NODE - is_processed.size(); i++) {
                    is_processed_new[i] = is_processed_new_back[i];
                }
                rollback_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_state_back, ISs_size, REDUCED_iSET_STACK, sr, reason, reason_back, k);
            }
            node_is_last = j+1 == ISs_new[iset_idx].size();
        }

        if (node_is_last) return true;

        for (auto j = rs; j < REASON_STACK.size(); j++) {
            ISs_involved[REASON_STACK[j]] = false;
            ISs_used[REASON_STACK[j]] = false;
        }
        REASON_STACK.resize(rs);
    }
    return false;
}

static bool inc_maxsatz_on_last_iset(
    const int ADDED_NODE,
    const custom_graph& G,
    custom_bitset& B,
    const custom_bitset& B_new,
    std::vector<custom_bitset>& ISs,
    std::vector<std::vector<int>>& ISs_new,
    std::vector<bool>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<bool>& ISs_used,
    std::vector<bool>& ISs_involved,
    std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<int>& reason,
    const std::vector<int>& unit_stack,
    const int k
) {
    static custom_bitset is_processed(G.size());
    static std::vector<bool> is_processed_new(G.size());
    static std::vector<int> reason_stack;
    static std::vector<int> REDUCED_iSET_STACK;

    reason_stack.clear();

    for (const auto bi : B_new) {
        static std::vector<int> new_unit_stack;

        new_unit_stack.clear();

        ISs[k].reset();
        ISs[k].set(bi);
        //ISs_size[k] = 1;

        // fix old node
        auto empty_iset = fix_oldNode_for_iset(bi, k, G.get_neighbor_set(bi), G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, reason, new_unit_stack, k);

        if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, ISs_used, reason, new_unit_stack, k);
        if (empty_iset == NONE) {
            empty_iset = unit_iset_process(G, ISs, ISs_new, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, reason, unit_stack, k);
        }

        // conflict found
        if (empty_iset != NONE) {
            identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_involved, ISs_used, is_processed, is_processed_new, reason_stack, reason);
            reset_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, reason, k);
            B.reset(bi);
            continue;
        }

        // conflict not found
        if (inc_maxsatz_lookahead_by_fl2(ADDED_NODE, G, ISs, ISs_new, ISs_state, ISs_size, REDUCED_iSET_STACK, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, reason_stack, k)) {
            reset_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, reason, k);
            B.reset(bi);
        } else {
            reset_context_for_maxsatz(ADDED_NODE, ISs_state, ISs_size, REDUCED_iSET_STACK, is_processed, is_processed_new, reason, k);
            return false;
        }
    }

    enlarge_conflict_sets(ADDED_NODE, G, ISs_new, CONFLICT_ISET_STACK, ISs_size, ISs_used, ISs_involved, reason, reason_stack, is_processed_new);

    return true;
}

static bool SATCOL(
    const custom_graph& G,
    custom_bitset& B,
    std::vector<custom_bitset>& ISs,
    int k
) {
    static std::vector<int> ISs_size(G.size());
    static std::vector<bool> ISs_state(G.size());
    static std::vector<bool> ISs_involved(G.size());
    static std::vector<bool> ISs_used(G.size());
    static custom_bitset is_processed(G.size());
    static std::vector<std::vector<int>> ISs_new(G.size());
    static std::vector<int> is_processed_new(G.size());
    static std::vector<int> reason(G.size()*2);
    static std::vector<int> unit_stack;
    static std::vector<std::vector<int>> CONFLICT_ISET_STACK(G.size());

    unit_stack.clear();
    //unit_stack.reserve(G.size());

    for (int i = 0; i < k; i++) {
        ISs_new[i].clear();
        ISs_size[i] = ISs[i].count();
        if (ISs_size[i] == 1) {
            unit_stack.emplace_back(i);
        }
    }

    int ADDED_NODE = G.size();

    do {
        static custom_bitset B_new(G.size());
        ISEQ_one(G, B, B_new);
        ISs_involved[k] = false;
        ISs_used[k] = false;
        ISs_state[k] = true;
        ISs_new[k].clear();
        ISs_size[k] = 1;

        for (int i = 0; i < k; i++) {
            assert(ISs_size[i] == ISs[i].count() + ISs_new[i].size());
            ISs_used[i] = false;
        }

        // B is an IS
        if (B.count() == B_new.count() && test_by_eliminate_failed_nodes(ADDED_NODE, G, B, B_new, ISs, ISs_new, ISs_state, ISs_size, ISs_used, ISs_involved, CONFLICT_ISET_STACK, reason, k)) {
            return true;
        }

        if (!inc_maxsatz_on_last_iset(ADDED_NODE, G, B, B_new, ISs, ISs_new, ISs_state, ISs_size, ISs_used, ISs_involved, CONFLICT_ISET_STACK, reason, unit_stack, k)) return false;

        ISs[k] = B_new;
        ISs_size[k] = B_new.count() + ISs_new[k].size();
        ADDED_NODE++;
        k++;
    } while (B.any());

    return true;
}

static int FiltCOL(
    const custom_graph& G,  // graph
    custom_bitset& V, // vertices set
    const std::vector<custom_bitset>& ISs,
    std::vector<custom_bitset>& ISs_t,
    const std::vector<int>& color_class,
    std::vector<int>& color_class_t,
    std::vector<std::size_t>& alpha,
    const int k_max
) {
    static custom_bitset Ubb(G.size());
    Ubb = V;

    for (auto i = 0; i < k_max; ++i) {
        auto v = Ubb.front();
        // we can't build k+1 IS => we can't improve the solution
        if (v == custom_bitset::npos) return i;

        const int k = color_class[v];
        color_class_t[v] = i;

        custom_bitset::DIFF(ISs_t[i], Ubb, G.get_neighbor_set(v));
        Ubb.reset(v);

        auto last_v = v;
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
                    Ubb.reset(v);
                }
                // otherwise kept for next iterations
            } else {
                last_v = v;
                // at most, we can remove vertices, so we don't need to start a new scan
                ISs_t[i] -= G.get_neighbor_set(v);
                color_class_t[v] = i;
                Ubb.reset(v);
            }
        }
        alpha[k] = last_v;
    }
    return k_max;
}

static bool FiltSAT(
    const custom_graph& G,  // graph
    custom_bitset& V, // vertices set
    std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class,
    std::vector<std::size_t>& alpha,
    const int k_max
) {
    static std::vector<std::pair<custom_bitset::reference, int>> S;
    static custom_bitset already_added(G.size());
    static custom_bitset already_visited(G.size());
    static custom_bitset neighbors(G.size());
    static custom_bitset D(G.size());

    // we apply FL (Failed Literal) to every vertex of each IS of Ct-alpha
    for (int i = k_max-1; i >= 0; --i) {
        for (const auto ui : ISs[i]) {
            std::stack<custom_bitset::reference> s;
            bool conflict_found = false;

            s.emplace(ui); // we add the current vertex ui to the stack S

            already_added.reset();
            already_visited.reset();

            neighbors.set();

            // for each Unit IS
            while (!s.empty()) {
                const auto u = s.top();
                already_visited.set(color_class[u]);
                s.pop();

                neighbors &= G.get_neighbor_set(u);

                // useless to iterate over r+1 that contains only u;
                for (auto j = 0; j < k_max; ++j) {
                    // if D is the unit IS set that we are setting to true, we continue
                    if (j == i || j == color_class[u] || already_visited[j]) continue;

                    // remove vertices non adjacent to u
                    custom_bitset::AND(D, ISs[j], neighbors);
                    auto di = D.front();

                    if (di == custom_bitset::npos) { // empty IS, conflict detected
                        conflict_found = true;
                        V.reset(ui);
                        ISs[i].reset(ui);

                        // more than one conflict can be found, but it's redundant in this case
                        // we are not keeping track of the conflicting clauses
                        break;
                    }
                    if (!already_added[j] && D.next(di) == custom_bitset::npos) { // Unit IS
                        already_added.set(j);
                        s.emplace(di);
                    }
                }

                // empty IS has been derived, we break
                if (conflict_found) break;
            }

            if (!conflict_found) alpha[i] = ui;
        }
        if (ISs[i].none()) return false;
    }
    return true;
}

static std::uint64_t steps = 0;
static std::uint64_t pruned = 0;

//__attribute__((target_clones("default", "popcnt")))
static void FindMaxClique(
    const custom_graph& G,  // graph
    std::vector<int>& K,       // current branch
    std::vector<int>& K_max,   // max branch
    custom_bitset& P_Bj, // vertices set
    custom_bitset& B,       // branching set
    std::vector<int> u, // incremental upper bounds,
    const bool is_k_partite = false
) {
    static std::vector ISs(G.size(), custom_bitset(G.size()));
    static std::vector ISs_t(G.size(), custom_bitset(G.size()));
    static std::vector<int> color_class(G.size());
    static std::vector<int> color_class_t(G.size());
    static std::vector alphas(G.size(), std::vector<std::size_t>(G.size()));
    static custom_bitset V_new(G.size());
    static std::vector P_Bjs(G.size(), custom_bitset(G.size()));
    static std::vector B_news(G.size(), custom_bitset(G.size()));

    steps++;

    // bitset containing all elements of P and all elements of B up to j
    const int curr = K.size()+1;

    for (const auto bi : B) {
        const int lb = K_max.size();
        P_Bj.set(bi);

        if (u[bi] + curr-1 <= lb) {
            u[bi] = static_cast<int>(K_max.size() - K.size());
            pruned++;
            continue;
        }

        // calculate sub-problem
        custom_bitset::AND(V_new, P_Bj, G.get_neighbor_set(bi));
        int V_new_size = V_new.count();

        // if we are in a leaf
        if (V_new_size == 0) {
            if (curr > lb) {
                // K_max = K U {bi}
                K_max = K;
                K_max.push_back(bi);
                std::cout << "Last incumbent: " << K_max.size() << std::endl;
            }
            return;
        }

        // if bi == 0, u[bi] always == 1!
        u[bi] = 1;

        for (auto neighbor = V_new.front(); neighbor < bi; neighbor = V_new.next(neighbor)) {
            u[bi] = std::max(u[bi], 1+u[neighbor]);
            // no point continue searching, we will overwrite this anyway with a potentially lower value
            if (u[bi] + curr-1 > lb) break;
        }

        //u[bi] = std::min(u[bi], lb-curr);
        // so if we enter in the following if, we don't overwrite it, otherwise we will replace u[bi] with lb-curr

        // if we can't improve, we prune the current branch
        // curr-1 because bi is not part of K yet
        // it goes into the pruned set

        u[bi] = std::min(u[bi], V_new_size+1);
        if (u[bi] + curr-1 <= lb) {
            pruned++;
            continue;
        }

        const int k = lb-curr;
        u[bi] = 1 + k;

        int next_is_k_partite = is_k_partite;
        // TODO: sus, it doesn't seem to work properly. setting random values doesn't change the output
        alphas[curr] = alphas[curr-1];

        /*
        // if is a k+1 partite graph
        if (is_k_partite) {
            // It's necessary to continue
            auto n_isets = FiltCOL(G, V_new, ISs, ISs_t, color_class, color_class_t, alphas[curr], k+1);
            // TODO: it's necessary to update u[bi]?
            if (n_isets < k+1) {
                u[bi] = n_isets;
                continue;
            }

            if (!FiltSAT(G, V_new, ISs_t, color_class_t, alphas[curr], k+1)) continue;
            B_new = ISs_t[k];
        } else {
            auto n_isets = ISEQ_branching(G, V_new, ISs, color_class, alphas[curr], k);
            if (n_isets < k) {
                u[bi] = n_isets;
                continue;
            }
            B_new = ISs[k];

            //if (B_new.none()) continue;
            if (is_IS(G, B_new)) {
                next_is_k_partite = true;
                // if we could return here, huge gains... damn
                if (!FiltSAT(G, V_new, ISs, color_class, alphas[curr], k+1)) continue;
                B_new = ISs[k];
            } else {
                //if (!cut_by_inc_maxsat_eliminate_first(G, V_new, B_new, ISs, u, k)) {
                //    // TODO: correct?
                //    u[bi] = lb - curr - 1;
                //}

                //if (!SATCOL(G, B_new, ISs, k)) continue;
            }
        }
        */

        auto n_isets = ISEQ_branching(G, V_new, ISs, color_class, alphas[curr], k);
        if (n_isets < k) {
            // TODO: why +2 and not +1?
            u[bi] = 1 + n_isets+1;
            continue;
        }
        B_news[curr] = ISs[k];
        assert(B_news[curr].any());
        if (SATCOL(G, B_news[curr], ISs, k)) continue;

        // at this point B is not empty
        K.push_back(bi);
        custom_bitset::DIFF(P_Bjs[curr+1], V_new, B_news[curr]);
        FindMaxClique(G, K, K_max, P_Bjs[curr+1], B_news[curr], u, next_is_k_partite);
        K.pop_back();

        // u[bi] cannot be lowered
        //u[bi] = std::min(u[bi], static_cast<int>(K_max.size() - K.size()));
    }
}

std::vector<int> CliSAT_no_sorting(const custom_graph& g, const custom_bitset& Ubb);
std::vector<int> CliSAT(const std::string& filename);
