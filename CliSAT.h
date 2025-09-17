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

#define NONE (-1)
#define NO_REASON (-1)

static void identify_conflict_isets(
    const int iset,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<int>& ISs_new_size,
    std::vector<std::uint8_t>& ISs_involved,
    std::vector<std::uint8_t>& ISs_used,
    const custom_bitset& is_processed,
    const std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason_stack,
    int& reason_stack_size,
    const std::vector<int>& reason
) {
    int starting_index = reason_stack_size;
    reason_stack[reason_stack_size] = iset;
    reason_stack_size++;
    ISs_involved[iset] = true;
    for (int i = starting_index; i < reason_stack_size; i++) {
        const auto reason_iset = reason_stack[i];

        // removed_nodes
        static custom_bitset removed(is_processed.size());
        custom_bitset::AND(removed, ISs[reason_iset], is_processed);
        for (auto r : removed) {
            auto r_iset = reason[r];
            if (ISs_involved[r_iset]) continue;

            ISs_involved[r_iset] = true;
            reason_stack[reason_stack_size] = r_iset;
            reason_stack_size++;
        }
        for (int j = 0; j < ISs_new_size[reason_iset]; j++) {
            const auto r = ISs_new[reason_iset][j];
            // not processed (removed)
            if (!is_processed_new[r-is_processed.size()] || reason[r] == NO_REASON) continue;

            auto r_iset = reason[r];
            if (ISs_involved[r_iset]) continue;

            ISs_involved[r_iset] = true;
            reason_stack[reason_stack_size] = r_iset;
            reason_stack_size++;
        }
    }

    for (int i = starting_index; i < reason_stack_size; i++) {
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
    int& reduced_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int& fixed_node_size,
    custom_bitset& is_processed,
    std::vector<int>& reason,
    std::vector<int>& unit_stack,
    int& unit_stack_size
) {
    static custom_bitset removed(G.size());
    custom_bitset::DIFF(removed, ISs[iset], neighbors);

    // non processed vertices
    auto count_removed = 0;
    for (auto r : removed) {
        count_removed++;

        ISs_size[iset]--;
        REDUCED_iSET_STACK[reduced_iset_size] = iset;
        reduced_iset_size++;
        is_processed.set(r);
        FIXED_NODE_STACK[fixed_node_size] = r;
        fixed_node_size++;
        reason[r] = l_is;
    }

    // we did nothing
    if (count_removed == 0) return false;

    if (ISs_size[iset] == 1) {
        unit_stack[unit_stack_size] = iset;
        unit_stack_size++;
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
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int& reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int& passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int& fixed_node_size,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason,
    std::vector<int>& unit_stack,
    int& unit_stack_size
) {
    int idx;
    ISs_size[fix_iset]--;
    REDUCED_iSET_STACK[reduced_iset_size] = fix_iset;
    reduced_iset_size++;
    ISs_state[fix_iset] = false;
    PASSIVE_iSET_STACK[passive_iset_size] = fix_iset;
    passive_iset_size++;
    is_processed_new[fix_node-G.size()] = true;
    FIXED_NODE_STACK[fixed_node_size] = fix_node;
    fixed_node_size++;
    reason[fix_node] = fix_iset;

    for (auto iset_idx : CONFLICT_ISET_STACK[fix_node-G.size()]) {
        if (!ISs_state[iset_idx]) continue;

        ISs_size[iset_idx]--;
        REDUCED_iSET_STACK[reduced_iset_size] = iset_idx;
        reduced_iset_size++;

        if (ISs_size[iset_idx] == 0) return iset_idx;
        if (ISs_size[iset_idx] == 1) {
            unit_stack[unit_stack_size] = iset_idx;
            unit_stack_size++;
        }
    }
    return NONE;
}

static int fix_oldNode_for_iset(
    const int fix_node,
    const int fix_iset,
    const custom_bitset& neighbors,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int& reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int& passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int& fixed_node_size,
    custom_bitset& is_processed,
    std::vector<int>& reason,
    std::vector<int>& unit_stack,
    int& unit_stack_size,
    const int k
) {
    ISs_size[fix_iset]--;
    REDUCED_iSET_STACK[reduced_iset_size] = fix_iset;
    reduced_iset_size++;
    ISs_state[fix_iset] = false;
    PASSIVE_iSET_STACK[passive_iset_size] = fix_iset;
    passive_iset_size++;
    is_processed.set(fix_node);
    FIXED_NODE_STACK[fixed_node_size] = fix_node;
    fixed_node_size++;
    reason[fix_node] = fix_iset;

    for (int iset = 0; iset < k; iset++) {
        if (!ISs_state[iset]) continue;

        if (remove_neighbors_from_iset(fix_iset, iset, neighbors, G, ISs, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size)) {
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
    const std::vector<int>& ISs_new_size,
    const custom_bitset& is_processed,
    const std::vector<std::uint8_t>& is_processed_new
) {
    for (int i = 0; i < ISs_new_size[l_is]; i++) {
        const auto new_node = ISs_new[l_is][i];
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
    const std::vector<int>& ISs_new_size,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int& reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int& passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int& fixed_node_size,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason,
    std::vector<int>& unit_stack,
    int& unit_stack_size,
    const int k
) {
    static custom_bitset neighbors(G.size());

    const auto fix_node = get_node_of_unit_iset(fix_iset, G, ISs, ISs_new, ISs_new_size, is_processed, is_processed_new);

    if (fix_node >= G.size()) return fix_newNode_for_iset(fix_node, fix_iset, G, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed_new, reason, unit_stack, unit_stack_size);

    custom_bitset::OR(neighbors, G.get_neighbor_set(fix_node), is_processed);
    return fix_oldNode_for_iset(fix_node, fix_iset, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, k);
}

static int unit_iset_process(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<int>& ISs_new_size,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int& reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int& passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int& fixed_node_size,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason,
    const std::vector<int>& unit_stack,
    const int unit_stack_size,
    std::vector<int>& new_unit_stack,
    const int k
) {
    static custom_bitset neighbors(G.size());

    for (int i = 0; i < unit_stack_size; i++) {
        const auto l_is = unit_stack[i];
        if (!ISs_state[l_is] || ISs_size[l_is] != 1) continue;

        int new_unit_stack_size = 0;

        auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, new_unit_stack, new_unit_stack_size,  k);
        if (empty_iset != NONE) return empty_iset;

        for (auto j = 0; j < new_unit_stack_size; j++) {
            const auto l_is2 = new_unit_stack[j];
            if (!ISs_state[l_is2]) continue;

            empty_iset = fix_anyNode_for_iset(l_is2, G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, new_unit_stack, new_unit_stack_size, k);
            if (empty_iset != NONE) return empty_iset;
        }
    }

    return NONE;
}

static int unit_iset_process_used_first(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<int>& ISs_new_size,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int& reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int& passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int& fixed_node_size,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    const std::vector<std::uint8_t>& ISs_used,
    std::vector<int>& reason,
    std::vector<int>& unit_stack,
    int& unit_stack_size,
    const int k
) {
    static custom_bitset neighbors(G.size());

    int j = 0;
    int used_iset_start = 0;
    int iset_start = 0;

    do {
        for (; used_iset_start < unit_stack_size; used_iset_start++) {
            const auto l_is = unit_stack[used_iset_start];

            // no need to check if iset is unit!
            if (!ISs_used[l_is] || !ISs_state[l_is]) continue;
            assert(ISs_size[l_is] == 1);

            //fix_node_iset
            const auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, unit_stack, unit_stack_size, k);
            if (empty_iset != NONE) return empty_iset;
        }

        for (j = iset_start; j < unit_stack_size; j++) {
            const auto l_is = unit_stack[j];

            if (!ISs_state[l_is]) continue;
            assert(ISs_size[l_is] == 1);

            const auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, unit_stack, unit_stack_size, k);
            if (empty_iset != NONE) return empty_iset;

            iset_start = j+1;
            break;
        }
    } while (j < unit_stack_size);

    return NONE;
}

static void enlarge_conflict_sets(
    const int ADDED_NODE,
    const custom_graph& G,
    std::vector<std::vector<int>>& ISs_new,
    std::vector<int>& ISs_new_size,
    std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    std::vector<int>& ISs_size,
    std::vector<std::uint8_t>& ISs_used,
    std::vector<std::uint8_t>& ISs_involved,
    std::vector<int>& reason,
    const std::vector<int>& reason_stack,
    const int& reason_stack_size,
    std::vector<std::uint8_t>& is_processed_new
) {
    is_processed_new[ADDED_NODE-G.size()] = false;
    reason[ADDED_NODE] = NO_REASON;
    CONFLICT_ISET_STACK[ADDED_NODE-G.size()].clear();

    for (int i = 0; i < reason_stack_size; i++) {
        auto iset = reason_stack[i];
        if (ISs_involved[iset]) continue;

        ISs_involved[iset] = true;
        ISs_new[iset][ISs_new_size[iset]] = ADDED_NODE;
        ISs_new_size[iset]++;
        ISs_size[iset]++;
        CONFLICT_ISET_STACK[ADDED_NODE-G.size()].push_back(iset);
    }

    for (int i = 0; i < reason_stack_size; i++) {
        auto reason_iset = reason_stack[i];
        ISs_involved[reason_iset] = false;
        ISs_used[reason_iset] = false;
    }
}

static void reset_context_for_maxsatz(
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    const std::vector<int>& REDUCED_iSET_STACK,
    int& reduced_iset_size,
    const std::vector<int>& PASSIVE_iSET_STACK,
    int& passive_iset_size,
    const std::vector<int>& FIXED_NODE_STACK,
    int& fixed_node_size,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason
) {
    is_processed.reset();
    for (int i = 0; i < fixed_node_size; i++) {
        auto node = FIXED_NODE_STACK[i];
        reason[node] = -1;
        if (node >= is_processed.size()) is_processed_new[node-is_processed.size()] = false;
        //else is_processed.reset(node);
    }
    fixed_node_size = 0;

    for (int i = 0; i < passive_iset_size; i++) {
        ISs_state[PASSIVE_iSET_STACK[i]] = true;
    }
    passive_iset_size = 0;

    // G.size() == is_processed.size()
    for (auto i = 0; i < reduced_iset_size; i++) ISs_size[REDUCED_iSET_STACK[i]]++;
    reduced_iset_size = 0;
}

static void rollback_context_for_maxsatz(
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    const std::vector<int>& REDUCED_iSET_STACK,
    int& reduced_iset_size,
    int reduced_iset_start,
    const std::vector<int>& PASSIVE_iSET_STACK,
    int& passive_iset_size,
    int passive_iset_start,
    const std::vector<int>& FIXED_NODE_STACK,
    int& fixed_node_size,
    int fixed_iset_start,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason
) {
    for (int i = fixed_iset_start; i < fixed_node_size; i++) {
        auto node = FIXED_NODE_STACK[i];
        reason[node] = -1;
        if (node >= is_processed.size()) is_processed_new[node-is_processed.size()] = false;
        else is_processed.reset(node);
    }
    fixed_node_size = fixed_iset_start;

    for (int i = passive_iset_start; i < passive_iset_size; i++) {
        ISs_state[PASSIVE_iSET_STACK[i]] = true;
    }
    passive_iset_size = passive_iset_start;

    // G.size() == is_processed.size()
    for (auto i = reduced_iset_start; i < reduced_iset_size; i++) ISs_size[REDUCED_iSET_STACK[i]]++;
    reduced_iset_size = reduced_iset_start;
}

static int simple_further_test_node(
    const int start,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<int>& ISs_new_size,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int fixed_node_size,
    const std::vector<std::uint8_t>& ISs_used,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason,
    const int k
) {
    static std::vector<std::uint8_t> ISs_tested(G.size());

    int saved_reduced_iset_stack_fill_pointer = reduced_iset_size;
    int saved_passive_iset_stack_fill_pointer = passive_iset_size;
    int saved_fixed_node_stack_fill_pointer = fixed_node_size;
    int my_saved_reduced_iset_stack_fill_pointer = reduced_iset_size;
    int my_saved_passive_iset_stack_fill_pointer = passive_iset_size;
    int my_saved_fixed_node_stack_fill_pointer = fixed_node_size;

    for (auto i = start; i < reduced_iset_size; i++) ISs_tested[REDUCED_iSET_STACK[i]] = false;

    bool conflict = false;
    int chosen_iset = NONE;
    for (int i = start; i < reduced_iset_size; i++) {
        chosen_iset = REDUCED_iSET_STACK[i];
        // we only consider reduced isets
        if (!ISs_state[chosen_iset] || ISs_tested[chosen_iset] || ISs_size[chosen_iset] != 2) continue;

        ISs_tested[chosen_iset] = true;
        static custom_bitset nodes(G.size());
        custom_bitset::DIFF(nodes, ISs[chosen_iset], is_processed);
        for (auto node : nodes) {
            static std::vector<int> unit_stack(G.size());
            int unit_stack_size = 0;

            static custom_bitset neighbors(G.size());
            custom_bitset::OR(neighbors, G.get_neighbor_set(node), is_processed);
            auto empty_iset = fix_oldNode_for_iset(node, chosen_iset, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, k);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, k);

            rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, my_saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, my_saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, my_saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new, reason);

            if (empty_iset == NONE) continue;

            is_processed.set(node);
            reason[node] = NO_REASON;
            FIXED_NODE_STACK[fixed_node_size] = node;
            fixed_node_size++;
            ISs_size[chosen_iset]--;
            REDUCED_iSET_STACK[reduced_iset_size] = chosen_iset;
            reduced_iset_size++;

            assert(ISs_size[chosen_iset] == 1);

            // unit iset
            unit_stack[0] = chosen_iset;
            unit_stack_size = 1;

            if (unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, k) != NONE) {
                conflict = true;
                break;
            }

            for (auto j = my_saved_reduced_iset_stack_fill_pointer; j < reduced_iset_size; j++) {
                ISs_tested[REDUCED_iSET_STACK[j]] = false;
            }

            my_saved_reduced_iset_stack_fill_pointer = reduced_iset_size;
            my_saved_passive_iset_stack_fill_pointer = passive_iset_size;
            my_saved_fixed_node_stack_fill_pointer = fixed_node_size;
        }
        if (conflict == true) break;
    }

    rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new, reason);

    if (conflict) return chosen_iset;
    return NONE;
}

static int test_node_for_failed_nodes(
    const int node,
    const int iset,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<int>& ISs_new_size,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int fixed_node_size,
    const std::vector<std::uint8_t>& ISs_used,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason,
    const int k
) {
    int saved_reduced_iset_stack_fill_pointer = reduced_iset_size;
    int saved_passive_iset_stack_fill_pointer = passive_iset_size;
    int saved_fixed_node_stack_fill_pointer = fixed_node_size;

    static std::vector<int> unit_stack(G.size());
    int unit_stack_size = 0;

    static custom_bitset neighbors(G.size());
    custom_bitset::OR(neighbors, G.get_neighbor_set(node), is_processed);

    auto empty_iset = fix_oldNode_for_iset(node, iset, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, k);
    if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, k);
    if (empty_iset == NONE) empty_iset = simple_further_test_node(saved_reduced_iset_stack_fill_pointer, G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, k);

    rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new, reason);
    return empty_iset;
}

static bool test_by_eliminate_failed_nodes(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<int>& ISs_new_size,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    const std::vector<std::uint8_t>& ISs_used,
    std::vector<int>& REDUCED_iSET_STACK,
    std::vector<int>& PASSIVE_iSET_STACK,
    std::vector<int>& FIXED_NODE_STACK,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason,
    const int k
) {
    int false_flag = 0;

    int reduced_iset_size = 0;
    int passive_iset_size = 0;
    int fixed_node_size = 0;
    int unit_stack_size = 0;

    do {
        false_flag = 0;
        for (auto my_iset = k; my_iset >= 0; my_iset--) {
            if (!ISs_state[my_iset]) continue;

            static std::vector<int> unit_stack(G.size());

            unit_stack_size = 0;
            static custom_bitset nodes(G.size());
            custom_bitset::DIFF(nodes, ISs[my_iset], is_processed);
            for (auto node : nodes) {
                if (test_node_for_failed_nodes(node, my_iset, G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, k) == NONE) continue;

                is_processed.set(node);
                reason[node] = NO_REASON;
                FIXED_NODE_STACK[fixed_node_size] = node;
                fixed_node_size++;
                false_flag++;
                ISs_size[my_iset]--;
                REDUCED_iSET_STACK[reduced_iset_size] = my_iset;
                reduced_iset_size++;
                if (ISs_size[my_iset] == 1) {
                    unit_stack[unit_stack_size] = my_iset;
                    unit_stack_size++;
                    break;
                } else if (ISs_size[my_iset] == 0) {
                    reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason);
                    return true;
                }
            }

            if (unit_stack_size > 0 &&
                unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, k) != NONE) {
                reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason);
                return true;
            }
        }
	} while (false_flag > 1);

    reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason);

    return false;
}

static int further_test_reduced_iset(
    const int start,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<int>& ISs_new_size,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int fixed_node_size,
    std::vector<std::uint8_t>& ISs_used,
    std::vector<std::uint8_t>& ISs_involved,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason,
    std::vector<int>& REASON_STACK,
    int& reason_stack_size,
    const int k
) {
    static std::vector<std::uint8_t> ISs_tested(G.size());

    int saved_reduced_iset_stack_fill_pointer = reduced_iset_size;
    int saved_passive_iset_stack_fill_pointer = passive_iset_size;
    int saved_fixed_node_stack_fill_pointer = fixed_node_size;

    for (auto i = start; i < reduced_iset_size; i++) ISs_tested[REDUCED_iSET_STACK[i]] = false;

    int chosen_iset = 0;
    std::size_t node = custom_bitset::npos;
    for (int i = start; i < reason_stack_size; i++) {
        chosen_iset = REDUCED_iSET_STACK[i];
        // we only consider reduced isets
        if (!ISs_state[chosen_iset] || ISs_tested[chosen_iset] || ISs_size[chosen_iset] != 2) continue;

        ISs_tested[chosen_iset] = true;

        bool has_new_node = false;
        for (int i = 0; i < ISs_new_size[chosen_iset]; i++) {
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
            static std::vector<int> unit_stack(G.size());
            int unit_stack_size = 0;

            static custom_bitset neighbors(G.size());
            custom_bitset::OR(neighbors, G.get_neighbor_set(node), is_processed);
            auto empty_iset = fix_oldNode_for_iset(node, chosen_iset, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, k);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, k);

            if (empty_iset == NONE) {
                rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new, reason);
                break;
            }

            ISs_involved[chosen_iset] = true;
            identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_new_size, ISs_involved, ISs_used, is_processed, is_processed_new, REASON_STACK, reason_stack_size, reason);
            ISs_involved[chosen_iset] = false;

            rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new, reason);
        }
        if (node == custom_bitset::npos) return chosen_iset;
    }
    return NONE;
}

static int inc_maxsatz_lookahead_by_fl2(
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    const std::vector<std::vector<int>>& ISs_new,
    const std::vector<int>& ISs_new_size,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int fixed_node_size,
    std::vector<std::uint8_t>& ISs_used,
    std::vector<std::uint8_t>& ISs_involved,
    const std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason,
    std::vector<int>& REASON_STACK,
    int& reason_stack_size,
    const int k
) {
	int sn = fixed_node_size;
	int sp = passive_iset_size;
	int rs = reason_stack_size;
	int sr = reduced_iset_size;

    static std::vector<int> unit_stack(G.size());
    static std::vector<std::uint8_t> tested(G.size());

    for (auto i = 0; i < sr; i++) tested[REDUCED_iSET_STACK[i]] = false;

    for (auto i = 0; i < sr; i++) {
        int iset_idx = REDUCED_iSET_STACK[i];
        if (tested[iset_idx] || !ISs_state[iset_idx] || ISs_size[iset_idx] != 2) continue;

        reason_stack_size = rs;
        tested[iset_idx] = true;
        static custom_bitset nodes(G.size());
        custom_bitset::DIFF(nodes, ISs[iset_idx], is_processed);
        bool exit = false;
        for (auto node : nodes) {
            int unit_stack_size = 0;
            static custom_bitset neighbors(G.size());
            custom_bitset::OR(neighbors, G.get_neighbor_set(node), is_processed);
            auto empty_iset = fix_oldNode_for_iset(node, iset_idx, neighbors, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, k);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, k);
            if (empty_iset == NONE) {
                bool is_last_node = (ISs_new_size[iset_idx] == 0 && nodes.next(node) == custom_bitset::npos);
                if (is_last_node) empty_iset = further_test_reduced_iset(sr, G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, REASON_STACK, reason_stack_size, k);
            }

            if (empty_iset == NONE) {
                rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, sr, PASSIVE_iSET_STACK, passive_iset_size, sp, FIXED_NODE_STACK, fixed_node_size, sn, is_processed, is_processed_new, reason);
                exit = true;
                break;
            }
            identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_new_size, ISs_involved, ISs_used, is_processed, is_processed_new, REASON_STACK, reason_stack_size, reason);
            rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, sr, PASSIVE_iSET_STACK, passive_iset_size, sp, FIXED_NODE_STACK, fixed_node_size, sn, is_processed, is_processed_new, reason);
        }
        bool node_is_last = !exit;
        if (!exit) {
            node_is_last = false;
            int j;
            for (j = 0; j < ISs_new_size[iset_idx]; j++) {
                //if (!is_node_active[node]) continue;
                auto node = ISs_new[iset_idx][j];
                if (is_processed_new[node-G.size()]) continue;

                int unit_stack_size = 0;
                auto empty_iset = fix_newNode_for_iset(node, iset_idx, G, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed_new, reason, unit_stack, unit_stack_size);
                if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, k);
                if (empty_iset == NONE) {
                    bool is_last_node = (j+1 == ISs_new_size[iset_idx]);
                    if (is_last_node) empty_iset = further_test_reduced_iset(sr, G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, REASON_STACK, reason_stack_size, k);
                }

                if (empty_iset == NONE) {
                    rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, sr, PASSIVE_iSET_STACK, passive_iset_size, sp, FIXED_NODE_STACK, fixed_node_size, sn, is_processed, is_processed_new, reason);
                    break;
                }
                identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_new_size, ISs_involved, ISs_used, is_processed, is_processed_new, REASON_STACK, reason_stack_size, reason);
                rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, sr, PASSIVE_iSET_STACK, passive_iset_size, sp, FIXED_NODE_STACK, fixed_node_size, sn, is_processed, is_processed_new, reason);
            }
            node_is_last = j == ISs_new_size[iset_idx];
        }

        if (node_is_last) return true;

        for (auto j = rs; j < reason_stack_size; j++) {
            ISs_involved[REASON_STACK[j]] = false;
            ISs_used[REASON_STACK[j]] = false;
        }
        reason_stack_size = rs;
    }
    return false;
}

static bool inc_maxsatz_on_last_iset(
    const int ADDED_NODE,
    const custom_graph& G,
    custom_bitset& B,
    std::vector<custom_bitset>& ISs,
    std::vector<std::vector<int>>& ISs_new,
    std::vector<int>& ISs_new_size,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<std::uint8_t>& ISs_used,
    std::vector<std::uint8_t>& ISs_involved,
    std::vector<int>& REDUCED_iSET_STACK,
    std::vector<int>& PASSIVE_iSET_STACK,
    std::vector<int>& FIXED_NODE_STACK,
    std::vector<std::vector<int>>& CONFLICT_ISET_STACK,
    custom_bitset& is_processed,
    std::vector<std::uint8_t>& is_processed_new,
    std::vector<int>& reason,
    std::vector<int>& reason_stack,
    const std::vector<int>& unit_stack,
    const int unit_stack_size,
    const int k
) {
    static std::vector<int> new_unit_stack(G.size());

    int reason_stack_size = 0;
    int reduced_iset_size = 0;
    int passive_iset_size = 0;
    int fixed_node_size = 0;

    for (const auto bi : ISs[k]) {
        int new_unit_stack_size = 0;

        // fix old node
        auto empty_iset = fix_oldNode_for_iset(bi, k, G.get_neighbor_set(bi), G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, new_unit_stack, new_unit_stack_size, k);
        if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, new_unit_stack, new_unit_stack_size, k);
        if (empty_iset == NONE) {
            empty_iset = unit_iset_process(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, unit_stack, unit_stack_size, new_unit_stack, k);
        }

        // conflict found
        if (empty_iset != NONE) {
            identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_new_size, ISs_involved, ISs_used, is_processed, is_processed_new, reason_stack, reason_stack_size, reason);
            reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason);
            B.reset(bi);
            continue;
        }

        // conflict not found
        if (inc_maxsatz_lookahead_by_fl2(G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, reason_stack, reason_stack_size, k)) {
            reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason);
            B.reset(bi);
        } else {
            reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason);
            return false;
        }
    }

    enlarge_conflict_sets(ADDED_NODE, G, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_size, ISs_used, ISs_involved, reason, reason_stack, reason_stack_size, is_processed_new);

    return true;
}

static bool SATCOL(
    const custom_graph& G,
    custom_bitset& B,
    std::vector<custom_bitset>& ISs,
    int k
) {
    static std::vector<int> ISs_size(G.size());
    static std::vector<std::uint8_t> ISs_state(G.size(), true);
    static std::vector<std::uint8_t> ISs_involved(G.size());
    static std::vector<std::uint8_t> ISs_used(G.size());
    static custom_bitset is_processed(G.size());
    static std::vector ISs_new(G.size(), std::vector<int>(G.size()));
    static std::vector<int> ISs_new_size(G.size());
    static std::vector<std::uint8_t> is_processed_new(G.size());
    static std::vector<int> reason(G.size()*2);
    static std::vector<int> unit_stack(G.size());
    static std::vector<std::vector<int>> CONFLICT_ISET_STACK(G.size());
    // Necessary G.size() because there are also ADDED_NODES
    static std::vector<int> REDUCED_iSET_STACK(G.size()*2);
    static std::vector<int> PASSIVE_iSET_STACK(G.size());
    static std::vector<int> FIXED_NODE_STACK(G.size()*2);
    std::vector<int> reason_stack(G.size());

    int unit_stack_size = 0;

    for (int i = 0; i < k; i++) {
        ISs_size[i] = ISs[i].count();
        ISs_new_size[i] = 0;
        if (ISs_size[i] == 1) {
            unit_stack[unit_stack_size] = i;
            unit_stack_size++;
        }
    }

    int ADDED_NODE = G.size();

    do {
        ISEQ_one(G, B, ISs[k]);
        ISs_involved[k] = false;
        ISs_used[k] = false;
        ISs_state[k] = true;
        ISs_new_size[k] = 0;
        ISs_size[k] = ISs[k].count();
        if (ISs_size[k] == 1) {
            unit_stack[unit_stack_size] = k;
            unit_stack_size++;
        }

        for (int i = 0; i < k; i++) {
            assert(ISs_size[i] == ISs[i].count() + ISs_new_size[i]);
            ISs_used[i] = false;
        }

        // B is an IS
        if (B.count() == ISs_size[k] && test_by_eliminate_failed_nodes(G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, ISs_used, REDUCED_iSET_STACK, PASSIVE_iSET_STACK, FIXED_NODE_STACK, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, k)) return true;
        if (!inc_maxsatz_on_last_iset(ADDED_NODE, G, B, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, ISs_used, ISs_involved, REDUCED_iSET_STACK, PASSIVE_iSET_STACK, FIXED_NODE_STACK, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, reason_stack, unit_stack, unit_stack_size, k)) return false;

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
