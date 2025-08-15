//
// Created by Beniamino Vagnarelli on 02/04/25.
//

module;

#include <fstream>
#include <numeric>
#include <cassert>
#include <sstream>
#include <ranges>

export module custom_graph;

import custom_bitset;

export class custom_graph {
    typedef custom_bitset::size_type size_type;

    std::vector<custom_bitset> graph;
    std::vector<size_type> order_conversion;

public:
    explicit custom_graph(const std::string& filename);
    explicit custom_graph(size_type size);
    custom_graph(const custom_graph& g, size_type size);

    using iterator = std::vector<custom_bitset>::iterator;
    using const_iterator = std::vector<custom_bitset>::const_iterator;

    iterator begin() noexcept { return graph.begin(); }
    iterator end() noexcept { return graph.end(); }
    [[nodiscard]] const_iterator begin() const noexcept { return graph.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return graph.end(); }

    custom_bitset& operator[](const size_type pos) { return graph[pos]; };

    void add_edge(size_type u, size_type v);
    void remove_edge(size_type u, size_type v);

    [[nodiscard]] size_type size() const { return graph.size(); }
    [[nodiscard]] size_type get_n_edges() const;

    [[nodiscard]] const custom_bitset& get_neighbor_set(const size_type v) const { return graph[v]; }
    [[nodiscard]] custom_bitset get_neighbor_set(const size_type v, const custom_bitset& set) const { return get_neighbor_set(v) & set; }
    [[nodiscard]] custom_bitset get_neighbor_set(const size_type v, const size_type threshold) const {
        return custom_bitset::before(get_neighbor_set(v), threshold);
    }
    [[nodiscard]] custom_bitset get_neighbor_set(const size_type v, const custom_bitset& set, const size_type threshold) const {
        return custom_bitset::before(get_neighbor_set(v, set), threshold);
    }
    [[nodiscard]] custom_bitset get_prev_neighbor_set(const size_type v) const { return get_neighbor_set(v, v); }
    [[nodiscard]] custom_bitset get_prev_neighbor_set(const size_type v, const custom_bitset& set) const { return get_prev_neighbor_set(v) & set; }
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const size_type v) const { return ~graph[v]; }
    [[nodiscard]] custom_bitset get_anti_neighbor_set(const size_type v, const custom_bitset& set) const { return get_anti_neighbor_set(v) & set; }

    [[nodiscard]] size_type degree() const;
    [[nodiscard]] size_type vertex_degree(size_type v) const;
    [[nodiscard]] float get_density() const;

    [[nodiscard]] std::vector<size_type> convert_back_set(const std::vector<size_type> &v) const;
    [[nodiscard]] custom_bitset convert_back_set(const custom_bitset &bb) const;
    [[nodiscard]] custom_graph get_complement() const;
    [[nodiscard]] custom_graph change_order(const std::vector<size_type>& order) const;
    [[nodiscard]] size_type get_subgraph_edges(const custom_bitset &subset) const;
    [[nodiscard]] std::vector<size_type> get_subgraph_vertices_degree(const custom_bitset &subset) const;

    void resize(size_type new_size);
};

inline custom_graph::custom_graph(const std::string& filename) {
    std::ifstream inf(filename);
    assert(inf);

    std::string strInput;
    while (std::getline(inf, strInput)) {
        if (strInput[0] == 'p') {
            size_type nodes;
            std::istringstream(strInput.substr(7)) >> nodes;

            graph.reserve(nodes);
            order_conversion.resize(nodes);
            std::iota(order_conversion.begin(), order_conversion.end(), 0);
            for (size_type i = 0; i < nodes; ++i) {
                graph.emplace_back(nodes);
            }
        }
        else if (strInput[0] == 'e') {
            size_type node1, node2;
            std::istringstream(strInput.substr(2)) >> node1 >> node2;

            add_edge(node1-1, node2-1);
        }
    }
}

inline custom_graph::custom_graph(const size_type size)
    : graph(size, custom_bitset(size)), order_conversion(size) {
    std::iota(order_conversion.begin(), order_conversion.end(), 0);
}

inline custom_graph::custom_graph(const custom_graph& g, const size_type size) {
    *this = g;
    resize(size);
}

// Rest of the implementation follows the same pattern as custom_graph
inline void custom_graph::add_edge(const size_type u, const size_type v) {
    if (u >= size() || v >= size() || graph[u][v]) return;

    graph[u].set(v);
    graph[v].set(u);
}

inline void custom_graph::remove_edge(const size_type u, const size_type v) {
    if (u >= size() || v >= size() || !graph[u][v]) return;

    graph[u].reset(v);
    graph[v].reset(u);
}

inline custom_graph::size_type custom_graph::get_n_edges() const {
    size_type n_edges = 0;
    for (const auto& bitset : graph) {
        n_edges += bitset.count();
    }
    return n_edges / 2; // Each edge is counted twice
}

inline custom_graph::size_type custom_graph::degree() const {
    size_type degree = 0;
    for (const auto& edge: graph)
        degree = std::max(degree, edge.degree());
    return degree;
}

inline custom_graph::size_type custom_graph::vertex_degree(const size_type v) const {
    return graph[v].count();
}

inline float custom_graph::get_density() const {
    return 2.0f * static_cast<float>(get_n_edges()) /
           (static_cast<float>(size()) * static_cast<float>(size() - 1));
}

inline std::vector<custom_graph::size_type> custom_graph::convert_back_set(const std::vector<size_type> &v) const {
    std::vector<size_type> set;
    set.reserve(v.size());
    for (const auto vertex : v) {
        set.push_back(order_conversion[vertex]);
    }
    return set;
}

inline custom_bitset custom_graph::convert_back_set(const custom_bitset &bb) const {
    custom_bitset set(bb.size());
    for (const auto v : bb) {
        set.set(order_conversion[v]);
    }
    return set;
}

inline custom_graph custom_graph::get_complement() const {
    custom_graph complement(*this);
    std::iota(complement.order_conversion.begin(), complement.order_conversion.end(), 0);

    for (size_type i = 0; i < complement.size(); i++) {
        complement.graph[i].flip();
        complement.graph[i].reset(i);
    }

    return complement;
}

inline custom_graph custom_graph::change_order(const std::vector<size_type> &order) const {
    custom_graph new_g(size());
    new_g.order_conversion = order;

    std::vector<size_type> conversion_helper(size());
    for (size_type i = 0; i < graph.size(); i++) {
        conversion_helper[order[i]] = i;
    }

    for (size_type i = 0; i < size(); i++) {
        const auto current_vertex = conversion_helper[i];
        auto set = static_cast<std::vector<size_type>>(graph[i]);
        for (auto& v : set) v = conversion_helper[v];
        new_g.graph[current_vertex] = custom_bitset(set, size());
    }

    return new_g;
}

inline custom_graph::size_type custom_graph::get_subgraph_edges(const custom_bitset &subset) const {
    size_type edges = 0;
    for (const auto v : subset) edges += (get_neighbor_set(v) & subset).count();
    return edges/2;
}

inline std::vector<custom_graph::size_type> custom_graph::get_subgraph_vertices_degree(const custom_bitset &subset) const {
    std::vector<size_type> d(subset.size());
    for (const auto v : subset) d[v] = vertex_degree(v);
    return d;
}

inline void custom_graph::resize(const size_type new_size) {
    if (size() == new_size) return;

    // add new bitsets if the new size is larger than the current size
    const custom_bitset default_bitset(new_size);
    graph.resize(new_size, default_bitset);

    // resize each bitset in the graph
    for (auto& bitset : graph) {
        bitset.resize(new_size);
    }

    // TODO: missing order_conversion resizing
}
