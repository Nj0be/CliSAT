#pragma once

#include "custom_graph.h"

/* In this function we are reading chunks of 8196 bytes from the file
 * based on the line type we do 3 things:
 *  - line type 'c' is a comment, skip
 *  - line type 'p edge' is the graph node and edges count, parse only the third and fourth elements
 *  - line type 'e' is and edge, parse only the second and third elements (node1 and node2)
 *  - line type '\n' is an empty line
 *
 *  We introduced a new extension to the DIMACS format, adding "p clique" problem type
 *  In "p clique" we specify number of nodes, number of edges to read and number of cliques to read.
 *  Then we add a new type of line 'q', that specify as second element the number n of nodes of the clique
 *  and as the n elements if specify the nodes of a clique
 */
inline custom_graph parse_dimacs_extended(const std::string& filename, const bool complementary = false) {
    // possible improvement: memory mapped files (mmap)
    
    custom_graph G;
    
    std::ifstream inf(filename);
    char buf[0x2000];
    char line_type = 'c';
    bool first_char = true;
    bool last_space_tab = false;
    std::size_t num1 = 0;
    std::size_t num2 = 0;
    std::size_t num3 = 0;
    std::string type;
    int spaces = 0;
    custom_bitset numbers;

    std::size_t remaining_edges = 1;
    std::size_t remaining_cliques = 1;

    while (inf) {
        if ((remaining_edges + remaining_cliques) == 0) break;
        inf.read(buf, sizeof(buf));
        std::streamsize bytes_read = inf.gcount();
        if (bytes_read <= 0) break;

        for (int i = 0; i < bytes_read; ++i) {
            auto c = buf[i];
            if (c == '\r') continue;
            if (c == '\n') {
                // avoid empty lines
                if (first_char == true) continue;

                first_char = true;
                switch (line_type) {
                    case 'c':
                        break;
                    case 'p':
                        // num1 == nodes
                        // num2 == edges
                        // num3 == cliques

                        G.resize(num1, complementary);
                        numbers.resize(num1);

                        remaining_edges = num2;
                        remaining_cliques = num3;

                        /*
                        for (size_type j = 0; j < num1; ++j) {
                            G[j].reset(j);
                        }
                        */

                        num1 = 0;
                        num2 = 0;
                        num3 = 0;

                        break;
                    case 'e':
                        // num1 == node1
                        // num2 == node2
                        // num3 == nothing
                        if (num1 <= 0 || num1 > G.size() || num2 <= 0 || num2 > G.size()) {
                            std::cerr << "Error while parsing input file: wrong edge in DIMACS format" << std::endl;
                            exit(1);
                        }
                        
                        if (complementary) G.remove_edge(num1-1, num2-1);
                        else G.add_edge(num1-1, num2-1);
                        remaining_edges--;

                        num1 = 0;
                        num2 = 0;
                        break;
                    case 'q':
                        // num1 == numbers of nodes in clique
                        // num2 == last node read
                        // num3 == number of nodes read

                        // if last number
                        if (num3 < num1) {
                            if (last_space_tab || num3+1 < num1) {
                                std::cerr << "Error while parsing input file: incomplete clique in DIMACS_EXTENDED format" << std::endl;
                                exit(1);
                            }
                            if (num2 <= 0 || num2 > G.size()) {
                                std::cerr << "Error while parsing input file: wrong node in DIMACS_EXTENDED format" << std::endl;
                                exit(1);
                            }
                            numbers.set(num2-1);
                            num3++;
                        } else if (!last_space_tab) {
                            std::cerr << "Error while parsing input file: too many nodes in clique in DIMACS_EXTENDED format" << std::endl;
                            exit(1);
                        }

                        auto first = numbers.front();
                        auto last = numbers.back();
                        
                        for (auto v = first; v != custom_bitset::npos; v = numbers.next(v)) {
                            if (complementary) {
                                custom_bitset::DIFF(G[v], numbers, first, last);
                            }
                            else {
                                custom_bitset::OR(G[v], numbers, first, last);
                                G[v].reset(v);
                            }
                        }
                        remaining_cliques--;

                        num1 = 0;
                        num2 = 0;
                        num3 = 0;
                        numbers.reset();
                        break;
                }

                // nothing to read
                if ((remaining_edges + remaining_cliques) == 0) break;

                continue;
            }

            // skip spaces/tabs
            if ((c == ' ' || c == '\t')) {
                if (!last_space_tab) {
                    switch(line_type) {
                        case 'p':
                            if (spaces == 1 && type != "edge" && type != "clique") {
                                std::cerr << "Error while parsing input file: unknown problem \"" << type << "\" in DIMACS_EXTENDED format" << std::endl;
                                exit(1);
                            }
                            break;
                        case 'q':
                            if (spaces >= 2 && num3 < num1) {
                                if (num2 <= 0 || num2 > G.size()) {
                                    std::cerr << "Error while parsing input file: wrong node in DIMACS_EXTENDED format" << std::endl;
                                    exit(1);
                                }
                                numbers.set(num2-1);
                                num3++;
                                num2 = 0;
                            }
                    }
                    spaces++;
                    last_space_tab = true;
                }
                continue;
            }

            if (first_char) {
                line_type = c;
                if (line_type != 'c' && line_type != 'p' && line_type != 'e' && line_type != 'q') {
                    std::cerr << "Error while parsing input file: invalid DIMACS/DIMACS_EXTENDED format" << std::endl;
                    exit(1);
                }
                //first_char = (c == '\n');
                first_char = false;
                last_space_tab = false;
                spaces = 0;
                continue;
            }

            if (line_type == 'c') continue;

            last_space_tab = false;
            if (line_type == 'p') {
                if (spaces == 1) type += c;
                else if (spaces == 2) num1 = num1*10 + (c - '0');
                else if (spaces == 3) num2 = num2*10 + (c - '0');
                else if (type == "clique" && spaces == 4) num3 = num3*10 + (c - '0');
            } else if (line_type == 'e') {
                if (spaces == 1) num1 = num1*10 + (c - '0');
                else if (spaces == 2) num2 = num2*10 + (c - '0');
            } else if (line_type == 'q') {
                if (spaces == 1) num1 = num1*10 + (c - '0');
                else num2 = num2*10 + (c - '0');
            }
        }
    }

    return G;
}

/* In this function we are reading chunks of 8196 bytes from the file
 * based on the line type we do 3 things:
 *  - line type 'c' is a comment, skip
 *  - line type 'p edge' is the graph node and edges count, parse only the third and fourth elements
 *  - line type 'e' is and edge, parse only the second and third elements (node1 and node2)
 *  - line type '\n' is an empty line
 *
 *  We introduced a new extension to the DIMACS format, adding "p clique" problem type
 *  In "p clique" we specify number of nodes, number of edges to read and number of cliques to read.
 *  Then we add a new type of line 'q', that specify as second element the number n of nodes of the clique
 *  and as the n elements if specify the nodes of a clique
 */
inline custom_graph parse_matrix_market(const std::string& filename, const bool complementary = false) {
    // possible improvement: memory mapped files (mmap)

    custom_graph G;

    std::ifstream inf(filename);
    char buf[0x2000];
    char line_type = 'c';
    bool first_char = true;
    bool last_space_tab = false;
    std::size_t num1 = 0;
    std::size_t num2 = 0;
    std::size_t num3 = 0;
    std::string type;
    int spaces = 0;
    custom_bitset numbers;

    std::size_t remaining_edges = 1;
    std::size_t remaining_cliques = 1;

    while (inf) {
        if ((remaining_edges + remaining_cliques) == 0) break;
        inf.read(buf, sizeof(buf));
        std::streamsize bytes_read = inf.gcount();
        if (bytes_read <= 0) break;

        for (int i = 0; i < bytes_read; ++i) {
            auto c = buf[i];
            if (c == '\r') continue;
            if (c == '\n') {
                // avoid empty lines
                if (first_char == true) continue;

                first_char = true;
                switch (line_type) {
                    case 'c':
                        break;
                    case 'p':
                        // num1 == nodes
                        // num2 == edges
                        // num3 == cliques

                        G.resize(num1, complementary);
                        numbers.resize(num1);

                        remaining_edges = num2;
                        remaining_cliques = num3;

                        /*
                        for (size_type j = 0; j < num1; ++j) {
                            G[j].reset(j);
                        }
                        */

                        num1 = 0;
                        num2 = 0;
                        num3 = 0;

                        break;
                    case 'e':
                        // num1 == node1
                        // num2 == node2
                        // num3 == nothing
                        if (num1 <= 0 || num1 > G.size() || num2 <= 0 || num2 > G.size()) {
                            std::cerr << "Error while parsing input file: wrong edge in DIMACS format" << std::endl;
                            exit(1);
                        }

                        if (complementary) G.remove_edge(num1-1, num2-1);
                        else G.add_edge(num1-1, num2-1);
                        remaining_edges--;

                        num1 = 0;
                        num2 = 0;
                        break;
                    case 'q':
                        // num1 == numbers of nodes in clique
                        // num2 == last node read
                        // num3 == number of nodes read

                        // if last number
                        if (num3 < num1) {
                            if (last_space_tab || num3+1 < num1) {
                                std::cerr << "Error while parsing input file: incomplete clique in DIMACS_EXTENDED format" << std::endl;
                                exit(1);
                            }
                            if (num2 <= 0 || num2 > G.size()) {
                                std::cerr << "Error while parsing input file: wrong node in DIMACS_EXTENDED format" << std::endl;
                                exit(1);
                            }
                            numbers.set(num2-1);
                            num3++;
                        } else if (!last_space_tab) {
                            std::cerr << "Error while parsing input file: too many nodes in clique in DIMACS_EXTENDED format" << std::endl;
                            exit(1);
                        }

                        auto first = numbers.front();
                        auto last = numbers.back();

                        for (auto v = first; v != custom_bitset::npos; v = numbers.next(v)) {
                            if (complementary) {
                                custom_bitset::DIFF(G[v], numbers, first, last);
                            }
                            else {
                                custom_bitset::OR(G[v], numbers, first, last);
                                G[v].reset(v);
                            }
                        }
                        remaining_cliques--;

                        num1 = 0;
                        num2 = 0;
                        num3 = 0;
                        numbers.reset();
                        break;
                }

                // nothing to read
                if ((remaining_edges + remaining_cliques) == 0) break;

                continue;
            }

            // skip spaces/tabs
            if ((c == ' ' || c == '\t')) {
                if (!last_space_tab) {
                    switch(line_type) {
                        case 'p':
                            if (spaces == 1 && type != "edge" && type != "clique") {
                                std::cerr << "Error while parsing input file: unknown problem \"" << type << "\" in DIMACS_EXTENDED format" << std::endl;
                                exit(1);
                            }
                            break;
                        case 'q':
                            if (spaces >= 2 && num3 < num1) {
                                if (num2 <= 0 || num2 > G.size()) {
                                    std::cerr << "Error while parsing input file: wrong node in DIMACS_EXTENDED format" << std::endl;
                                    exit(1);
                                }
                                numbers.set(num2-1);
                                num3++;
                                num2 = 0;
                            }
                    }
                    spaces++;
                    last_space_tab = true;
                }
                continue;
            }

            if (first_char) {
                line_type = c;
                if (line_type != 'c' && line_type != 'p' && line_type != 'e' && line_type != 'q') {
                    std::cerr << "Error while parsing input file: invalid DIMACS/DIMACS_EXTENDED format" << std::endl;
                    exit(1);
                }
                //first_char = (c == '\n');
                first_char = false;
                last_space_tab = false;
                spaces = 0;
                continue;
            }

            if (line_type == 'c') continue;

            last_space_tab = false;
            if (line_type == 'p') {
                if (spaces == 1) type += c;
                else if (spaces == 2) num1 = num1*10 + (c - '0');
                else if (spaces == 3) num2 = num2*10 + (c - '0');
                else if (type == "clique" && spaces == 4) num3 = num3*10 + (c - '0');
            } else if (line_type == 'e') {
                if (spaces == 1) num1 = num1*10 + (c - '0');
                else if (spaces == 2) num2 = num2*10 + (c - '0');
            } else if (line_type == 'q') {
                if (spaces == 1) num1 = num1*10 + (c - '0');
                else num2 = num2*10 + (c - '0');
            }
        }
    }

    return G;
}

/*
inline custom_graph parse_graph(const std::string& filename, const bool complementary = false) {

}
*/
