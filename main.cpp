//
// Created by Beniamino Vagnarelli on 31/03/25.
//

#include <iostream>
#include <chrono>

import CliSAT;
import custom_graph;
import custom_bitset;
import instructions;

int main(int argc, char *argv[]) {
    initialize();
    /*
    custom_bitset a(64, true);
    std::cout << a << std::endl;
    exit(1);
    */

    //auto filename = "../../CliSAT_instances/dimacs/c-fat200-1.clq"; //12
    //auto filename = "../../CliSAT_instances/dimacs/brock200_1.clq"; //21
    //auto filename = "../../CliSAT_instances/dimacs/MANN_a45.clq"; //345
    //auto filename = "../../CliSAT_instances/dimacs/brock200_2.clq"; //12
    //auto filename = "../../CliSAT_instances/dimacs/c-fat200-2.clq"; //24
    //auto filename = "../../CliSAT_instances/dimacs/C125.9.clq"; //34
    //auto filename = "../../CliSAT_instances/dimacs/C250.9.clq"; //44
    //auto filename = "../../CliSAT_instances/example.txt"; //4
    //auto filename = "../../CliSAT_instances/example2.txt"; //3
    //auto filename = "../../CliSAT_instances/dimacs/dsjc1000.5.clq";
    //auto filename = "../../CliSAT_instances/dimacs/gen200_p0.9_44.clq"; //44
    //auto filename = "../../CliSAT_instances/dimacs/gen200_p0.9_55.clq"; //55
    //auto filename = "../../CliSAT_instances/dimacs/san1000.clq"; //15
    //auto filename = "../../CliSAT_instances/dimacs/san200_0.9_1.clq"; //70
    //auto filename = "../../CliSAT_instances/dimacs/san200_0.9_2.clq"; //60
    auto filename = "../../CliSAT_instances/dimacs/san400_0.7_1.clq"; //40
    //auto filename = "../../CliSAT_instances/dimacs/san400_0.7_3.clq"; //22
    //auto filename = "../../CliSAT_instances/dimacs/sanr200_0.9.clq"; //42
    //auto filename = "../../CliSAT_instances/dimacs/sanr200_0.7.clq"; //18
    //auto filename = "../../CliSAT_instances/dimacs/sanr400_0.7.clq"; //21
    //auto filename = "../../CliSAT_instances/misclib/evil/evil-N120-p98-myc5x24.clq";
    //auto filename = "../../CliSAT_instances/misclib/mon/monoton-7.clq"; //19
    //auto filename = "../../CliSAT_instances/misclib/sudoku/gordon_royle_1.txt";
    //auto filename = "../../CliSAT_instances/misclib/vc/vc-exact_005.gr.clqc"; //71
    //auto filename = "../../CliSAT_instances/misclib/vc/vc-exact_006.gr.clqc"; //74
    //auto filename = "../../CliSAT_instances/csplib/aim_A/aim-50-1-6-sat-1.clq"; //77
    //auto filename = "../../CliSAT_instances/csplib/aim_A/aim-50-1-6-sat-2.clq"; //76
    //auto filename = "../../CliSAT_instances/csplib/aim_A/aim-50-1-6-unsat-1.clq"; //68
    //auto filename = "../../CliSAT_instances/csplib/lat/qcp-10-67-00_X2.clq"; //100
    //auto filename = "../../CliSAT_instances/bhoshlib/frb30-15-1.clq";

    //auto filename = argv[1];


    auto begin = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();

    begin = std::chrono::steady_clock::now();
    std::cout << custom_bitset(CliSAT(custom_graph(filename))) << std::endl;
    end = std::chrono::steady_clock::now();
    std::cout << "CliSAT = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    /*
    begin = std::chrono::steady_clock::now();
    const custom_graph g1(filename);
    auto g2 = g1.change_order(NEW_SORT(g1, 2).first);
    std::cout << run_BBMC(g2) << std::endl;
    end = std::chrono::steady_clock::now();
    std::cout << "BBMC = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    result = BB_Max_Clique_iter(g);
    end = std::chrono::steady_clock::now();
    std::cout << "BBMC iter = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;#1#

    begin = std::chrono::steady_clock::now();
    const custom_graph g3(filename);
    auto g4 = g3.change_order(NEW_SORT(g3, 2).first);
    std::cout << run_BBMCR(g4) << std::endl;
    end = std::chrono::steady_clock::now();
    std::cout << "BBMCR = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    */

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
