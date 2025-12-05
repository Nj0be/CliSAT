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
                            throw std::runtime_error("parse_dimacs_extended: wrong edge");
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
                                throw std::runtime_error("parse_dimacs_extended: incomplete clique");
                            }
                            if (num2 <= 0 || num2 > G.size()) {
                                throw std::runtime_error("parse_dimacs_extended: wrong node in clique");
                            }
                            numbers.set(num2-1);
                            num3++;
                        } else if (!last_space_tab) {
                            throw std::runtime_error("parse_dimacs_extended: too many nodes in clique");
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
                                throw std::runtime_error("parse_dimacs_extended: unknown problem type");
                            }
                            break;
                        case 'q':
                            if (spaces >= 2 && num3 < num1) {
                                if (num2 <= 0 || num2 > G.size()) {
                                    throw std::runtime_error("parse_dimacs_extended: wrong node");
                                }
                                numbers.set(num2-1);
                                num3++;
                                num2 = 0;
                            }
                            break;
                    }
                    spaces++;
                    last_space_tab = true;
                }
                continue;
            }

            if (first_char) {
                line_type = c;
                if (line_type != 'c' && line_type != 'p' && line_type != 'e' && line_type != 'q') {
                    throw std::runtime_error("parse_dimacs_extended: invalid line format");
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

struct MTX_header {
    enum class object_enum { NONE, MATRIX, VECTOR };
    enum class format_enum { NONE, COORDINATE, ARRAY };
    enum class field_enum { NONE, REAL, DOUBLE, COMPLEX, INTEGER, PATTERN };
    enum class symmetry_enum { NONE, GENERAL, SYMMETRIC, SKEW_SYMMETRIC, HERMITIAN };

    object_enum object = object_enum::NONE;
    format_enum format = format_enum::NONE;
    field_enum field = field_enum::NONE;
    symmetry_enum symmetry = symmetry_enum::NONE;

    bool is_valid() const {
        if (object == object_enum::NONE || format == format_enum::NONE || field == field_enum::NONE || symmetry == symmetry_enum::NONE) return false;

        switch (symmetry) {
            case symmetry_enum::GENERAL:
            case symmetry_enum::SYMMETRIC:
                if (field != field_enum::REAL && field != field_enum::COMPLEX && field != field_enum::INTEGER && field != field_enum::PATTERN) return false;
                break;

            case symmetry_enum::SKEW_SYMMETRIC:
                if (field != field_enum::REAL && field != field_enum::COMPLEX && field != field_enum::INTEGER) return false;
                break;

            case symmetry_enum::HERMITIAN:
                if (field != field_enum::COMPLEX) return false;
                break;
        }

        return true;
    }
};

// Garbage parser!
// USE AT YOUR OWN RISK
inline custom_graph parse_matrix_market(const std::string& filename, const bool complementary = false) {
    // possible improvement: memory mapped files (mmap)

    custom_graph G;

    std::ifstream inf(filename);
    char buf[0x2000];
    bool first_char = true;
    bool line_has_percentage = false;
    int spaces = 0;
    bool last_space_tab = false;
    std::size_t num1 = 0;
    std::size_t num2 = 0;
    std::size_t num3 = 0;

    MTX_header header;
    bool header_read = false;
    bool is_header = true;
    bool metadata_read = false;
    std::string header_tmp;


    while (inf) {
        inf.read(buf, sizeof(buf));
        std::streamsize bytes_read = inf.gcount();
        if (bytes_read <= 0) break;

        for (int i = 0; i < bytes_read; ++i) {
            auto c = buf[i];
            if (c == '\r') continue;
            if (c == '\n') {
                if (line_has_percentage) {
                    if (is_header && spaces == 4) {
                        if (header_tmp == "general") header.symmetry = MTX_header::symmetry_enum::GENERAL;
                        else if (header_tmp == "symmetric") header.symmetry = MTX_header::symmetry_enum::SYMMETRIC;
                        else if (header_tmp == "skew-symmetric") header.symmetry = MTX_header::symmetry_enum::SKEW_SYMMETRIC;
                        else if (header_tmp == "hermitian") header.symmetry = MTX_header::symmetry_enum::HERMITIAN;

                        if (!header.is_valid()) {
                            throw std::runtime_error("parse_matrix_market: invalid MTX header");
                        }
                        header_read = true;

                        header_tmp.clear();
                    }
                } else if (metadata_read) {
                    // TODO: improve
                    if (num1 != num2) G.add_edge(num1-1, num2-1);
                } else { // metadata line
                    G.resize(num1);
                    metadata_read = true;
                }

                first_char = true;
                line_has_percentage = false;
                spaces = 0;
                last_space_tab = false;
                is_header = !header_read;
                num1 = 0;
                num2 = 0;
                num3 = 0;
                continue;
            }

            // skip spaces/tabs
            if ((c == ' ' || c == '\t')) {
                if (!last_space_tab) {
                    if (line_has_percentage) {
                        if (header_read) continue;
                        switch (spaces) {
                            case 0:
                                if (header_tmp != "MatrixMarket") is_header = false;
                                header_tmp.clear();
                                break;
                            case 1:
                                if (!is_header) continue;

                                if (header_tmp == "matrix") header.object = MTX_header::object_enum::MATRIX;
                                else if (header_tmp == "vector") header.object = MTX_header::object_enum::VECTOR;

                                header_tmp.clear();
                                break;
                            case 2:
                                if (!is_header) continue;

                                if (header_tmp == "coordinate") header.format = MTX_header::format_enum::COORDINATE;
                                else if (header_tmp == "array") header.format = MTX_header::format_enum::ARRAY;

                                header_tmp.clear();
                                break;
                            case 3:
                                if (!is_header) continue;

                                if (header_tmp == "real") header.field= MTX_header::field_enum::REAL;
                                else if (header_tmp == "double") header.field= MTX_header::field_enum::DOUBLE;
                                else if (header_tmp == "complex") header.field= MTX_header::field_enum::COMPLEX;
                                else if (header_tmp == "integer") header.field= MTX_header::field_enum::INTEGER;
                                else if (header_tmp == "pattern") header.field= MTX_header::field_enum::PATTERN;

                                header_tmp.clear();
                                break;
                        }
                    } else if (metadata_read) {

                    } else { // metadata line

                    }
                    spaces++;
                    last_space_tab = true;
                }
                continue;
            }

            if (first_char) {
                first_char = false;
                last_space_tab = false;

                if (c == '%') {
                    line_has_percentage = true;
                } else {
                    num1 = c - '0';
                }

                continue;
            }

            last_space_tab = false;

            if (line_has_percentage) {
                if (header_tmp.empty() && c == '%') continue;

                if (is_header) header_tmp += c;
            } else if (metadata_read) {
                if (spaces == 0) num1 = num1*10 + (c - '0');
                else if (spaces == 1) num2 = num2*10 + (c - '0');
                else if (spaces == 2) num3 = num3*10 + (c - '0');
            } else { // metadata line
                if (spaces == 0) num1 = num1*10 + (c - '0');
                else if (spaces == 1) num2 = num2*10 + (c - '0');
                else if (spaces == 2) num3 = num3*10 + (c - '0');
            }
        }
    }

    if (complementary) G.complement();

    return G;
}

inline custom_graph parse_graph(const std::string& filename, const bool complementary = false) {
    // list of parser attempts
    using parser_t = custom_graph(*)(const std::string&, bool);

    static const parser_t parsers[] = {
        &parse_dimacs_extended,
        &parse_matrix_market
    };

    std::vector<std::string> errors;

    for (auto p : parsers) {
        try {
            return p(filename, complementary);
        } catch (const std::exception& e) {
            errors.emplace_back(e.what());
        }
    }

    // If we reach this point, all failed
    std::string msg = "parse_graph: all parsing attempts failed:\n";
    for (auto& e : errors)
        msg += "  - " + e + "\n";

    throw std::runtime_error(msg);
}
