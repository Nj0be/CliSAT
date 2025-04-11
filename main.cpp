//
// Created by Beniamino Vagnarelli on 31/03/25.
//

#include <iostream>
//#include "graph/graph.h"
#include "BBMCR.h"
#include "BBMC.h"
#include "custom_bitset.h"
#include "custom_graph.h"
#include "sorting.h"
#include <chrono>

#include "CliSAT.h"

int main() {
    /*
    ugraph g(100); //creates an empty (undirected) graph with 100 vertices
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 2);
    */

    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/c-fat200-1.clq"); //12
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/c-fat200-2.clq"); //24
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/csplib/aim_A/aim-100-1-6-unsat-3.clq"); //151
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/csplib/aim_A/aim-100-1-6-sat-1.clq"); //154
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/example.txt"); //4
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/C1000.9.clq"); //68*
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/C500.9.clq"); //57*
    //const custom_graph g("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/C250.9.clq"); //44
    //BB_Max_Clique_cl bb("/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/C125.9.clq"); //34

    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/c-fat200-1.clq"; //12
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/brock200_1.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/MANN_a45.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/brock200_2.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/c-fat200-2.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/C125.9.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/example.txt";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/example2.txt";
    auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/C250.9.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/dsjc1000.5.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/gen200_p0.9_44.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/gen200_p0.9_55.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/san1000.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/san200_0.9_1.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/san200_0.9_2.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/san400_0.7_1.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/san400_0.7_3.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/sanr200_0.9.clq"; //42
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/sanr200_0.7.clq";
    //auto filename = "/home/benia/uni/Tesi_triennale/CliSAT_instances/dimacs/sanr400_0.7.clq";


    auto begin = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();

    begin = std::chrono::steady_clock::now();
    std::cout << CliSAT(custom_graph(filename)) << std::endl;
    end = std::chrono::steady_clock::now();
    std::cout << "CliSAT = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    const custom_graph g1(filename);
    auto g2 = g1.change_order(NEW_SORT(g1, 2).first);
    std::cout << run_BBMC(g2) << std::endl;
    end = std::chrono::steady_clock::now();
    std::cout << "BBMC = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    /*begin = std::chrono::steady_clock::now();
    result = BB_Max_Clique_iter(g);
    end = std::chrono::steady_clock::now();
    std::cout << "BBMC iter = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;*/

    begin = std::chrono::steady_clock::now();
    const custom_graph g3(filename);
    auto g4 = g3.change_order(NEW_SORT(g3, 2).first);
    std::cout << run_BBMCR(g4) << std::endl;
    end = std::chrono::steady_clock::now();
    std::cout << "BBMCR = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    /*for (auto v : result) {
        std::cout << v+1 << " ";
    }
    std::cout << std::endl << result.size() << std::endl;*/

    //bitscan_benchmark1();
    //popcount_benchmark();
    //test_custom_bitset();
    //subtraction_benchmark();
    //bit_scan_forward_benchmark();
    //bit_scan_forward_destructive_benchmark();
    //bit_scan_reverse_benchmark();
    //bitwise_and_benchmark();

    return 0;
}
