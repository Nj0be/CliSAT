//
// Created by Beniamino Vagnarelli on 31/03/25.
//

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>

#include "CliSAT.h"
#include "custom_bitset.h"
#include "custom_graph.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage: CliSAT filename(DIMACS/EXTENDED) [flag]/n flag is -c for the MCP and -i for the MISP" << std::endl;
        return 1;
    }

    auto filename = argv[1];

    bool MISP = false;
    if (strcmp(argv[2], "-c") == 0) MISP = false;
    else if (strcmp(argv[2], "-i") == 0) MISP = true;


    std::cout << custom_bitset(CliSAT(filename, MISP)) << std::endl;

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
