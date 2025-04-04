//
// Created by Beniamino Vagnarelli on 31/03/25.
//

#include <iostream>
//#include "graph/graph.h"
#include "bitscan_benchmark.h"
#include "CliSAT.h"
#include "custom_bitset.h"
#include "custom_graph.h"

int main() {
    /*
    ugraph g(100); //creates an empty (undirected) graph with 100 vertices
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 2);
    */

    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/c-fat200-1.clq"); //12
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/csplib/aim_A/aim-100-1-6-unsat-3.clq"); //151
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/csplib/aim_A/aim-100-1-6-sat-1.clq"); //154
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/example.txt"); //4
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/C1000.9.clq"); //68*
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/C500.9.clq"); //57*
    const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/C250.9.clq"); //44
    auto result = run_BB_Max_Clique(g);
    for (auto v : result) {
        std::cout << v+1 << " ";
    }
    std::cout << std::endl << result.size() << std::endl;

    //bitscan_benchmark1();
    //test_custom_bitset();
    //subtraction_benchmark();
    //bit_scan_forward_benchmark();
    //bit_scan_forward_destructive_benchmark();
    //bit_scan_reverse_benchmark();
    //bitwise_and_benchmark();

    return 0;
}
