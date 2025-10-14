//
// Created by Beniamino Vagnarelli on 31/03/25.
//

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>

#include "CliSAT.h"
#include "custom_bitset.h"
#include "custom_graph.h"

int main(int argc, char *argv[]) {
    std::string usage = "Usage: CliSAT filename(DIMACS/EXTENDED) [time_limit(s)] [problem_type] [sorting_method]\nproblem_type is -c for the MCP and -i for the MISP\nsorting method can be: 0 - none, 1 - auto (NEW_SORT), 2 - DEG_SORT, 3 - COLOUR_SORT\n";
    if (argc < 5) {
        std::cerr << usage;
        return 1;
    }

    auto filename = argv[1];
    if (!std::filesystem::exists(filename)) {
        std::cerr << "Error: file not found: " << filename << std::endl;
        return 1;
    }

    bool MISP = false;
    auto problem_type = argv[2];
    if (strcmp(problem_type, "-c") == 0) MISP = false;
    else if (strcmp(problem_type, "-i") == 0) MISP = true;
    else {
        std::cerr << "Error: invalid problem type: " << problem_type << std::endl;
        std::cerr << usage;
        return 1;
    }

    int time_limit_int = 0;

    try {
        time_limit_int = std::stoi(argv[3]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: not a valid time limit: " << argv[3] << std::endl;
        std::cerr << usage;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: time limit out of range: " << argv[3] << std::endl;
        std::cerr << usage;
        return 1;
    }

    if (time_limit_int < 0) {
        std::cerr << "Error: time limit negative: " << argv[3] << std::endl;
        std::cerr << usage;
        return 1;
    }
    auto time_limit = std::chrono::seconds(time_limit_int);

    int sorting_method = 0;


    try {
        sorting_method = std::stoi(argv[4], nullptr, 10);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: not a valid sorting method: " << argv[4] << std::endl;
        std::cerr << usage;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: sorting method out of range: " << argv[4] << std::endl;
        std::cerr << usage;
        return 1;
    }

    if (sorting_method < 0 || sorting_method > 3) {
        std::cerr << "Error: time limit not in range [0-4]: " << argv[4] << std::endl;
        std::cerr << usage;
        return 1;
    }

    std::cout << custom_bitset(CliSAT(filename, time_limit, MISP, sorting_method)) << std::endl;

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
