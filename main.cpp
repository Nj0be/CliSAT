//
// Created by Beniamino Vagnarelli on 31/03/25.
//

#include <iostream>
//#include "graph/graph.h"
#include "bitscan_benchmark.h"
#include "custom_bitset.h"
#include "custom_graph.h"

int main() {
    /*
    ugraph g(100); //creates an empty (undirected) graph with 100 vertices
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 2);
    */

    custom_graph("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/c-fat200-1.clq");

    //bitscan_benchmark1();
    //test_custom_biset();
    //bit_scan_forward_benchmark();
    //bit_scan_reverse_benchmark();
    //bitwise_and_benchmark();

    return 0;
}
