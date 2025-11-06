//
// Created by Beniamino Vagnarelli on 03/04/25.
//

#pragma once

#include <cassert>
#include <chrono>
#include <stack>
#include <iostream>

#include "coloring.h"
#include "custom_bitset.h"
#include "custom_graph.h"

#define NONE (-1)
#define NO_REASON (-1)

enum SORTING_METHOD {
    NO_SORT,
    NEW_SORT,
    DEG_SORT,
    COLOUR_SORT
};

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
    static custom_bitset removed(is_processed.size());

    int starting_index = reason_stack_size;
    reason_stack[reason_stack_size] = iset;
    reason_stack_size++;
    ISs_involved[iset] = true;
    for (int i = starting_index; i < reason_stack_size; i++) {
        const auto reason_iset = reason_stack[i];

        // removed_nodes
        custom_bitset::AND(removed, ISs[reason_iset], is_processed);
        for (auto r : removed) {
            const auto r_iset = reason[r];
            if (r_iset == NO_REASON || ISs_involved[r_iset]) continue;

            ISs_involved[r_iset] = true;
            reason_stack[reason_stack_size] = r_iset;
            reason_stack_size++;
        }
        for (int j = 0; j < ISs_new_size[reason_iset]; j++) {
            const auto r = ISs_new[reason_iset][j];
            // not processed (removed)
            if (!is_processed_new[r-is_processed.size()] || reason[r] == NO_REASON) continue;

            const auto r_iset = reason[r];
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
    const custom_graph& G,
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
    const std::vector<int>& color_class
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

    static custom_bitset anti_neighbors(G.size());
    //custom_bitset::DIFF(anti_neighbors, G.get_complement_neighbor_set(fix_node), is_processed);
    // equivalent to the above -> ~a & ~b == ~(a|b) (de morgan)
    custom_bitset::NOR(anti_neighbors, G.get_neighbor_set(fix_node), is_processed);
    //is_processed |= anti_neighbors;

    for (auto r : anti_neighbors) {
        const auto iset = color_class[r];

        if (!ISs_state[iset]) continue;

        ISs_size[iset]--;
        REDUCED_iSET_STACK[reduced_iset_size] = iset;
        reduced_iset_size++;
        FIXED_NODE_STACK[fixed_node_size] = r;
        fixed_node_size++;
        reason[r] = fix_iset;
        is_processed.set(r);

        // conflict found
        if (ISs_size[iset] == 0) return iset;

        if (ISs_size[iset] == 1) {
            unit_stack[unit_stack_size] = iset;
            unit_stack_size++;
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

    const auto node  = ISs[l_is].front_difference(is_processed);
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
    const std::vector<int>& color_class
) {
    const auto fix_node = get_node_of_unit_iset(fix_iset, G, ISs, ISs_new, ISs_new_size, is_processed, is_processed_new);

    if (fix_node >= G.size()) return fix_newNode_for_iset(fix_node, fix_iset, G, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed_new, reason, unit_stack, unit_stack_size);

    return fix_oldNode_for_iset(fix_node, fix_iset, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);
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
    const std::vector<int>& color_class
) {
    for (int i = 0; i < unit_stack_size; i++) {
        const auto l_is = unit_stack[i];
        if (!ISs_state[l_is] || ISs_size[l_is] != 1) continue;

        int new_unit_stack_size = 0;

        auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, new_unit_stack, new_unit_stack_size, color_class);
        if (empty_iset != NONE) return empty_iset;

        for (auto j = 0; j < new_unit_stack_size; j++) {
            const auto l_is2 = new_unit_stack[j];
            if (!ISs_state[l_is2]) continue;

            empty_iset = fix_anyNode_for_iset(l_is2, G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, new_unit_stack, new_unit_stack_size, color_class);
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
    const std::vector<int>& color_class
) {
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
            const auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, unit_stack, unit_stack_size, color_class);
            if (empty_iset != NONE) return empty_iset;
        }

        for (j = iset_start; j < unit_stack_size; j++) {
            const auto l_is = unit_stack[j];

            if (!ISs_state[l_is]) continue;
            assert(ISs_size[l_is] == 1);

            const auto empty_iset = fix_anyNode_for_iset(l_is, G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, unit_stack, unit_stack_size, color_class);
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
        const auto iset = reason_stack[i];
        if (ISs_involved[iset]) continue;

        ISs_involved[iset] = true;
        if (ISs_new[iset].size() <= ISs_new_size[iset]) ISs_new[iset].push_back(ADDED_NODE);
        else ISs_new[iset][ISs_new_size[iset]] = ADDED_NODE;
        ISs_new_size[iset]++;
        ISs_size[iset]++;
        CONFLICT_ISET_STACK[ADDED_NODE-G.size()].push_back(iset);
    }

    for (int i = 0; i < reason_stack_size; i++) {
        const auto reason_iset = reason_stack[i];
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
    std::vector<std::uint8_t>& is_processed_new
) {
    for (int i = 0; i < fixed_node_size; i++) {
        auto node = FIXED_NODE_STACK[i];
        if (node >= is_processed.size()) is_processed_new[node-is_processed.size()] = false;
        else is_processed.reset(node);
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
    std::vector<std::uint8_t>& is_processed_new
) {
    for (int i = fixed_iset_start; i < fixed_node_size; i++) {
        auto node = FIXED_NODE_STACK[i];
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
    const std::vector<int>& color_class
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

            auto empty_iset = fix_oldNode_for_iset(node, chosen_iset, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, color_class);

            rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, my_saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, my_saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, my_saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new);

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

            if (unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, color_class) != NONE) {
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

    rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new);

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
    const std::vector<int>& color_class
) {
    int saved_reduced_iset_stack_fill_pointer = reduced_iset_size;
    int saved_passive_iset_stack_fill_pointer = passive_iset_size;
    int saved_fixed_node_stack_fill_pointer = fixed_node_size;

    static std::vector<int> unit_stack(G.size());
    int unit_stack_size = 0;

    auto empty_iset = fix_oldNode_for_iset(node, iset, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);
    if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, color_class);
    if (empty_iset == NONE) empty_iset = simple_further_test_node(saved_reduced_iset_stack_fill_pointer, G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, color_class);

    rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new);
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
    const std::vector<int>& color_class,
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
                if (test_node_for_failed_nodes(node, my_iset, G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, color_class) == NONE) continue;

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
                    reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new);
                    return true;
                }
            }

            if (unit_stack_size > 0 &&
                unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, color_class) != NONE) {
                reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new);
                return true;
            }
        }
	} while (false_flag > 1);

    reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new);

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
    const std::vector<int>& color_class
) {
    static std::vector<std::uint8_t> ISs_tested(G.size());

    int saved_reduced_iset_stack_fill_pointer = reduced_iset_size;
    int saved_passive_iset_stack_fill_pointer = passive_iset_size;
    int saved_fixed_node_stack_fill_pointer = fixed_node_size;

    for (auto i = start; i < reduced_iset_size; i++) ISs_tested[REDUCED_iSET_STACK[i]] = false;

    int chosen_iset = 0;
    std::size_t node = custom_bitset::npos;
    for (int i = start; i < reduced_iset_size; i++) {
        chosen_iset = REDUCED_iSET_STACK[i];
        // we only consider reduced isets
        if (!ISs_state[chosen_iset] || ISs_tested[chosen_iset] || ISs_size[chosen_iset] != 2) continue;

        ISs_tested[chosen_iset] = true;

        bool has_new_node = false;
        for (int j = 0; j < ISs_new_size[chosen_iset]; j++) {
            node = ISs_new[chosen_iset][j];
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

            auto empty_iset = fix_oldNode_for_iset(node, chosen_iset, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, color_class);

            if (empty_iset == NONE) {
                rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new);
                break;
            }

            ISs_involved[chosen_iset] = true;
            identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_new_size, ISs_involved, ISs_used, is_processed, is_processed_new, REASON_STACK, reason_stack_size, reason);
            ISs_involved[chosen_iset] = false;

            rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, is_processed_new);
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
    const std::vector<int>& color_class
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
            auto empty_iset = fix_oldNode_for_iset(node, iset_idx, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);
            if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, color_class);
            if (empty_iset == NONE) {
                bool is_last_node = (ISs_new_size[iset_idx] == 0 && nodes.next(node) == custom_bitset::npos);
                if (is_last_node) empty_iset = further_test_reduced_iset(sr, G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, REASON_STACK, reason_stack_size, color_class);
            }

            if (empty_iset == NONE) {
                rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, sr, PASSIVE_iSET_STACK, passive_iset_size, sp, FIXED_NODE_STACK, fixed_node_size, sn, is_processed, is_processed_new);
                exit = true;
                break;
            }
            identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_new_size, ISs_involved, ISs_used, is_processed, is_processed_new, REASON_STACK, reason_stack_size, reason);
            rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, sr, PASSIVE_iSET_STACK, passive_iset_size, sp, FIXED_NODE_STACK, fixed_node_size, sn, is_processed, is_processed_new);
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
                if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, unit_stack, unit_stack_size, color_class);
                if (empty_iset == NONE) {
                    bool is_last_node = (j+1 == ISs_new_size[iset_idx]);
                    if (is_last_node) empty_iset = further_test_reduced_iset(sr, G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, REASON_STACK, reason_stack_size, color_class);
                }

                if (empty_iset == NONE) {
                    rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, sr, PASSIVE_iSET_STACK, passive_iset_size, sp, FIXED_NODE_STACK, fixed_node_size, sn, is_processed, is_processed_new);
                    break;
                }
                identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_new_size, ISs_involved, ISs_used, is_processed, is_processed_new, REASON_STACK, reason_stack_size, reason);
                rollback_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, sr, PASSIVE_iSET_STACK, passive_iset_size, sp, FIXED_NODE_STACK, fixed_node_size, sn, is_processed, is_processed_new);
            }
            node_is_last = (j == ISs_new_size[iset_idx]);
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
    const std::vector<int>& color_class,
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
        auto empty_iset = fix_oldNode_for_iset(bi, k, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, new_unit_stack, new_unit_stack_size, color_class);
        if (empty_iset == NONE) empty_iset = unit_iset_process_used_first(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, ISs_used, reason, new_unit_stack, new_unit_stack_size, color_class);
        if (empty_iset == NONE) {
            empty_iset = unit_iset_process(G, ISs, ISs_new, ISs_new_size, CONFLICT_ISET_STACK, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new, reason, unit_stack, unit_stack_size, new_unit_stack, color_class);
        }

        // conflict found
        if (empty_iset != NONE) {
            identify_conflict_isets(empty_iset, ISs, ISs_new, ISs_new_size, ISs_involved, ISs_used, is_processed, is_processed_new, reason_stack, reason_stack_size, reason);
            reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new);
            B.reset(bi);
            continue;
        }

        // conflict not found
        if (inc_maxsatz_lookahead_by_fl2(G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, ISs_used, ISs_involved, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, reason_stack, reason_stack_size, color_class)) {
            reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new);
            B.reset(bi);
        } else {
            reset_context_for_maxsatz(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, is_processed_new);
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
    std::vector<int>& color_class,
    int k
) {
    static std::vector<int> ISs_size(G.size());
    static std::vector<std::uint8_t> ISs_state(G.size(), true);
    static std::vector<std::uint8_t> ISs_involved(G.size());
    static std::vector<std::uint8_t> ISs_used(G.size());
    static custom_bitset is_processed(G.size());
    static std::vector<std::vector<int>> ISs_new;
    static std::vector<int> ISs_new_size(G.size());
    static std::vector<std::uint8_t> is_processed_new(G.size());
    static std::vector<int> reason(G.size()*2);
    static std::vector<int> unit_stack(G.size());
    static std::vector<std::vector<int>> CONFLICT_ISET_STACK(G.size());
    // Necessary G.size()*2 because there are also ADDED_NODES
    static std::vector<int> REDUCED_iSET_STACK(G.size()*2);
    static std::vector<int> PASSIVE_iSET_STACK(G.size());
    static std::vector<int> FIXED_NODE_STACK(G.size()*2);
    std::vector<int> reason_stack(G.size());

    int unit_stack_size = 0;

    // is_processed = true for nodes not considered
    is_processed.set();

    if (ISs_new.size() <= k) {
        ISs_new.resize(k+1);
    }

    for (int i = 0; i < k; i++) {
        // is_processed = true for nodes not considered
        is_processed -= ISs[i];
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
        is_processed -= ISs[k];
        for (auto v : ISs[k]) {
            color_class[v] = k;
        }

        for (int i = 0; i < k; i++) {
            assert(ISs_size[i] == ISs[i].count() + ISs_new_size[i]);
            ISs_used[i] = false;
        }

        // B is an IS
        if (B.count() == ISs_size[k] && test_by_eliminate_failed_nodes(G, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, ISs_used, REDUCED_iSET_STACK, PASSIVE_iSET_STACK, FIXED_NODE_STACK, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, color_class, k)) return true;
        if (!inc_maxsatz_on_last_iset(ADDED_NODE, G, B, ISs, ISs_new, ISs_new_size, ISs_state, ISs_size, ISs_used, ISs_involved, REDUCED_iSET_STACK, PASSIVE_iSET_STACK, FIXED_NODE_STACK, CONFLICT_ISET_STACK, is_processed, is_processed_new, reason, reason_stack, unit_stack, unit_stack_size, color_class, k)) return false;

        ADDED_NODE++;
        k++;
        if (ISs_new.size() <= k) {
            ISs_new.resize(k+1);
        }
        if (ISs.size() <= k) ISs.emplace_back(G.size());
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
    const std::vector<int>& alpha,
    std::vector<int>& ISs_mapping,
    const int k_max
) {
    static custom_bitset Ubb(G.size());
    static custom_bitset already_added(G.size());
    already_added.reset();
    Ubb = V;

    for (auto i = 0; i < k_max; ++i) {
        auto v = Ubb.front();
        // we can't build k+1 IS => we can't improve the solution
        if (v == custom_bitset::npos) return i;

        const int k = color_class[v];
        ISs_mapping[i] = k;

        custom_bitset::DIFF(ISs_t[i], Ubb, G.get_neighbor_set(v));
        color_class_t[v] = i;
        Ubb.reset(v);

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
                // at most, we can remove vertices, so we don't need to start a new scan
                ISs_t[i] -= G.get_neighbor_set(v);
                color_class_t[v] = i;
                Ubb.reset(v);
            }
        }
        //alpha[k] = last_v;
    }
    return k_max;
}

static void rollback_context_for_maxsatz2(
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
    std::vector<int>& reason
) {
    for (int i = fixed_iset_start; i < fixed_node_size; i++) {
        auto node = FIXED_NODE_STACK[i];
        reason[node] = -1;
        is_processed.reset(node);
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

static int unit_iset_process2(
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
    const std::vector<int>& color_class
) {
    static std::vector<int> new_unit_stack(G.size());

    for (int i = 0; i < unit_stack_size; i++) {
        const auto l_is = unit_stack[i];
        if (!ISs_state[l_is] || ISs_size[l_is] != 1) continue;

        int new_unit_stack_size = 0;

        auto node  = ISs[l_is].front_difference(is_processed);
        auto empty_iset = fix_oldNode_for_iset(node, l_is, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, new_unit_stack, new_unit_stack_size, color_class);
        if (empty_iset != NONE) return empty_iset;

        for (auto j = 0; j < new_unit_stack_size; j++) {
            const auto l_is2 = new_unit_stack[j];
            if (!ISs_state[l_is2]) continue;

            auto node2  = ISs[l_is2].front_difference(is_processed);
            empty_iset = fix_oldNode_for_iset(node2, l_is, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);
            if (empty_iset != NONE) return empty_iset;
        }
    }

    return NONE;
}

static int simple_further_test_node2(
    const int start,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int fixed_node_size,
    custom_bitset& is_processed,
    std::vector<int>& reason,
    const std::vector<int>& color_class
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

            auto empty_iset = fix_oldNode_for_iset(node, chosen_iset, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);
            if (empty_iset == NONE) empty_iset = unit_iset_process2(G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);

            rollback_context_for_maxsatz2(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, my_saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, my_saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, my_saved_fixed_node_stack_fill_pointer, is_processed, reason);

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

            if (unit_iset_process2(G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class) != NONE) {
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

    rollback_context_for_maxsatz2(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, reason);

    if (conflict) return chosen_iset;
    return NONE;
}

static int test_node_for_failed_nodes2(
    const int node,
    const int iset,
    const custom_graph& G,
    const std::vector<custom_bitset>& ISs,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    int reduced_iset_size,
    std::vector<int>& PASSIVE_iSET_STACK,
    int passive_iset_size,
    std::vector<int>& FIXED_NODE_STACK,
    int fixed_node_size,
    custom_bitset& is_processed,
    std::vector<int>& reason,
    const std::vector<int>& color_class
) {
    int saved_reduced_iset_stack_fill_pointer = reduced_iset_size;
    int saved_passive_iset_stack_fill_pointer = passive_iset_size;
    int saved_fixed_node_stack_fill_pointer = fixed_node_size;

    static std::vector<int> unit_stack(G.size());
    int unit_stack_size = 0;

    auto empty_iset = fix_oldNode_for_iset(node, iset, G, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);
    if (empty_iset == NONE) empty_iset = unit_iset_process2(G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, unit_stack, unit_stack_size, color_class);
    if (empty_iset == NONE) empty_iset = simple_further_test_node2(saved_reduced_iset_stack_fill_pointer, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, color_class);

    rollback_context_for_maxsatz2(ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, saved_reduced_iset_stack_fill_pointer, PASSIVE_iSET_STACK, passive_iset_size, saved_passive_iset_stack_fill_pointer, FIXED_NODE_STACK, fixed_node_size, saved_fixed_node_stack_fill_pointer, is_processed, reason);
    return empty_iset;
}

static bool test_by_eliminate_failed_nodes2(
    custom_bitset& V,
    const custom_graph& G,
    std::vector<custom_bitset>& ISs,
    std::vector<std::uint8_t>& ISs_state,
    std::vector<int>& ISs_size,
    std::vector<int>& REDUCED_iSET_STACK,
    std::vector<int>& PASSIVE_iSET_STACK,
    std::vector<int>& FIXED_NODE_STACK,
    custom_bitset& is_processed,
    std::vector<int>& reason,
    const std::vector<int>& color_class,
    const int k
) {
    int reduced_iset_size = 0;
    int passive_iset_size = 0;
    int fixed_node_size = 0;

    for (auto my_iset = k; my_iset >= 0; my_iset--) {
        for (auto node : ISs[my_iset]) {
            if (test_node_for_failed_nodes2(node, my_iset, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, reduced_iset_size, PASSIVE_iSET_STACK, passive_iset_size, FIXED_NODE_STACK, fixed_node_size, is_processed, reason, color_class) == NONE) continue;

            V.reset(node);
            ISs[my_iset].reset(node);
            ISs_size[my_iset]--;
            is_processed.set(node);

            if (ISs_size[my_iset] == 0) return true;
        }
    }

    return false;
}

static bool FiltSAT(
    const custom_graph& G,  // graph
    custom_bitset& V, // vertices set
    std::vector<custom_bitset>& ISs,
    const std::vector<int>& color_class,
    const int k_max
) {
    static std::vector<int> ISs_size(G.size());
    static std::vector<std::uint8_t> ISs_state(G.size(), true);
    static std::vector<std::uint8_t> ISs_involved(G.size());
    static custom_bitset is_processed(G.size());
    static std::vector<int> reason(G.size()*2);
    // Necessary G.size()*2 because there are also ADDED_NODES
    static std::vector<int> REDUCED_iSET_STACK(G.size()*2);
    static std::vector<int> PASSIVE_iSET_STACK(G.size());
    static std::vector<int> FIXED_NODE_STACK(G.size()*2);
    std::vector<int> reason_stack(G.size());

    is_processed.set();
    for (int i = 0; i < k_max; i++) {
        is_processed -= ISs[i];
        ISs_state[i] = true;
        ISs_size[i] = ISs[i].count();
    }

    // we apply FL (Failed Literal) to every vertex of each IS of Ct-alpha
    return test_by_eliminate_failed_nodes2(V, G, ISs, ISs_state, ISs_size, REDUCED_iSET_STACK, PASSIVE_iSET_STACK, FIXED_NODE_STACK, is_processed, reason, color_class, k_max-1);
}

static std::uint64_t steps = 0;
static std::uint64_t pruned = 0;

//__attribute__((target_clones("default", "popcnt")))
static bool FindMaxClique(
    const custom_graph& G,  // graph
    std::vector<int>& K,       // current branch
    std::vector<int>& K_max,   // max branch
    custom_bitset& P_Bj, // vertices set
    custom_bitset& B,       // branching set
    std::vector<int> u, // incremental upper bounds,
    const std::chrono::time_point<std::chrono::steady_clock> max_time,
    std::vector<int> alpha = {}, // incremental upper bounds,
    const bool is_k_partite = false
) {
    static std::vector ISs(K_max.size()+1, custom_bitset(G.size()));
    static std::vector ISs_t(K_max.size()+1, custom_bitset(G.size()));
    static std::vector<int> color_class(G.size());
    static std::vector<int> color_class_t(G.size());
    static custom_bitset V_new(G.size());
    // bitset containing all elements of P and all elements of B up to j
    static std::vector P_Bjs(K_max.size()+2, custom_bitset(G.size()));
    static std::vector B_news(K_max.size()+1, custom_bitset(G.size()));
    static std::vector<int> ISs_mapping(G.size());

    steps++;

    // |K|+1 because we are yet to add the vertex to the current solution
    const int depth = K.size()+1;

    while (ISs.size() <= K_max.size()) ISs.emplace_back(G.size());

    for (const auto bi : B) {
        if (std::chrono::steady_clock::now() > max_time) {
            return false;
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

        // if we are in a leaf
        if (V_new_size == 0) {
            if (K.size()+1 > lb) {
                // K_max = K U {bi}
                K_max = K;
                K_max.push_back(bi);
                //std::cout << "Last incumbent: " << K_max.size() << std::endl;
                if (ISs.size() <= K_max.size()) ISs.emplace_back(G.size());
                ISs_t.emplace_back(G.size());
                P_Bjs.emplace_back(G.size());
                B_news.emplace_back(G.size());
                // we can return because it's an incremental branching scheme, we can add only one vertex at a time
                return true;
            }
            continue;
        }

        u[bi] = std::min(u[bi], V_new_size+1);
        if (u[bi] + K.size() <= lb) {
            pruned++;
            continue;
        }

        const int k = lb-depth;
        u[bi] = k+1;

        int next_is_k_partite = is_k_partite;
        std::vector<int> new_alpha;

        // if is a k+1 partite graph
        if (is_k_partite) {
            const auto n_isets = FiltCOL(G, V_new, ISs, ISs_t, color_class, color_class_t, alpha, ISs_mapping, k+1);
            if (n_isets < k+1) {
                u[bi] = n_isets+1;
                continue;
            }

            if (FiltSAT(G, V_new, ISs_t, color_class_t, k+1)) continue;
            new_alpha.resize(k+1);
            for (int i = 0; i < k+1; i++) {
                new_alpha[i] = ISs_t[ISs_mapping[i]].back();
            }
            B_news[depth] = ISs_t[k];
            assert(B_news[depth].any());
        } else {
            const auto n_isets = ISEQ_branching(G, V_new, ISs, color_class, k);
            if (n_isets < k+1) {
                u[bi] = n_isets+1;
                continue;
            }
            assert(ISs[k].any());

            //if (B_new.none()) continue;
            if (is_IS(G, ISs[k])) {
                next_is_k_partite = true;
                // if we could return here, huge gains... damn
                if (FiltSAT(G, V_new, ISs, color_class, k+1)) continue;
                new_alpha.resize(k+1);
                for (int i = 0; i < k+1; i++) {
                    new_alpha[i] = ISs[i].back();
                }
                B_news[depth] = ISs[k];
            } else {
                B_news[depth] = ISs[k];
                if (SATCOL(G, B_news[depth], ISs, color_class, k)) continue;
            }
        }

        // at this point B is not empty
        K.push_back(bi);
        custom_bitset::DIFF(P_Bjs[depth+1], V_new, B_news[depth]);
        auto new_solution_found = FindMaxClique(G, K, K_max, P_Bjs[depth+1], B_news[depth], u, max_time, new_alpha, next_is_k_partite);
        K.pop_back();
        if (new_solution_found) return true;

        // u[bi] cannot be lowered
        //u[bi] = std::min(u[bi], static_cast<int>(K_max.size() - K.size()));
    }

    return false;
}

std::vector<int> CliSAT_no_sorting(const custom_graph& g, const custom_bitset& Ubb, std::chrono::milliseconds time_limit);

std::vector<int> CliSAT(const std::string& filename, std::chrono::milliseconds time_limit, bool MISP, SORTING_METHOD sorting_method, bool AMTS_enabled = false);
