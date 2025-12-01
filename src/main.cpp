//
// Created by Beniamino Vagnarelli on 31/03/25.
//

#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <CLI/CLI.hpp>

#include "CliSAT.h"
#include "custom_bitset.h"
#include "custom_graph.h"

const inline std::string PROGRAM_NAME = "CliSAT";

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define PATCH_VERSION 0

struct options {
    std::string graph_filename;
    std::string constraints_filename; // only for nesting
    // we set it to half the maximum rapresentable value in a steady_clock (nanoseconds), used for operations with program timer
    // if seconds or milliseconds would be used, an overflow would happen
    std::chrono::seconds time_limit = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::duration::max()/2);
    std::chrono::milliseconds cs_time_limit = std::chrono::milliseconds(50);
    size_t threads = std::thread::hardware_concurrency();
    SORTING_METHOD sorting_method = NEW_SORT;
    bool AMTS_enabled = false;
};

int main(int argc, char *argv[]) {
    auto transform_time_limit = CLI::Validator(
        [](std::string& input) {
            if (input == "0") input = std::to_string(std::numeric_limits<int>::max());
            return "";
        }, "VALIDATOR DESCRIPTION", "Validator name");

    CLI::App app{"Algorithm for MCP/MISP", PROGRAM_NAME};
    //app.require_subcommand(1); // exactly one of mcp/misp/nesting

    // Define options in the same positional order as your original interface
    options opts;

    app.set_version_flag("-v,--version", std::format("{} version {}.{}.{}", PROGRAM_NAME, MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION));

    // Subcommands
    auto mcp = app.add_subcommand("mcp", "Maximum Clique (MCP)");
    mcp->alias("c");

    auto misp = app.add_subcommand("misp", "Maximum Independent Set (MISP)");
    misp->alias("i");

    auto nesting = app.add_subcommand("nesting", "Nesting with extra constraints");
    nesting->alias("n");

    for (auto cmd : {mcp, misp, nesting}) {
        // 1) filename (must exist)
        cmd->add_option("-g, --graph", opts.graph_filename, "Input graph file (DIMACS/EXTENDED)")
            ->check(CLI::ExistingFile)
            ->required();

        // 2) time_limit (seconds, non-negative)
        cmd->add_option("-T, --time-limit", opts.time_limit, "Time limit in seconds")
            ->check(CLI::Range(0, std::numeric_limits<int>::max()))
            ->transform(transform_time_limit);

        // 2) time_limit colour sort (seconds, non-negative)
        cmd->add_option("--cs-time-limit", opts.cs_time_limit, "Colour Sort Time limit in milliseconds")
            ->check(CLI::Range(0, std::numeric_limits<int>::max()))
            ->transform(transform_time_limit);

        // 3) threads
        cmd->add_option("-t, --threads", opts.threads, "Number of threads")
            ->check(CLI::Range(size_t{0}, std::numeric_limits<size_t>::max()));

        // 4) sorting_method: 0..3
        cmd->add_option_function<std::string>("-s, --sorting", 
                        [&opts](const std::string& sorting_method) {
                            if (sorting_method == "NO_SORT") opts.sorting_method = NO_SORT;
                            else if (sorting_method == "NEW_SORT") opts.sorting_method = NEW_SORT;
                            else if (sorting_method == "DEG_SORT") opts.sorting_method = DEG_SORT;
                            else if (sorting_method == "COLOUR_SORT") opts.sorting_method = COLOUR_SORT;
                            else throw CLI::ValidationError("--sorting must be one of { NO_SORT, NEW_SORT, DEG_SORT, COLOUR_SORT }");
                        },
                       "Sorting method: 0-none, 1-auto(NEW_SORT), 2-DEG_SORT, 3-COLOUR_SORT");
            //->check(CLI::IsMember({"NO_SORT", "NEW_SORT", "DEG_SORT", "COLOUR_SORT"}));

        // 5) AMTS_enabled: 0 or 1
        cmd->add_option("-a, --amts", opts.AMTS_enabled, "AMTS enabled: 0-disabled, 1-enabled")
            ->check(CLI::Range(0, 1));
    }

    // Only nesting has constraints; make them required there
    nesting->add_option("-c,--constraints", opts.constraints_filename,
                        "Constraints file (required for nesting)")
           ->check(CLI::ExistingFile)
           ->required();

    if (argc == 1) {
        std::cout << app.help() << std::endl;
        return 0;
    }

    // Parse (handles --help, validation errors, etc.)
    argv = app.ensure_utf8(argv);
    CLI11_PARSE(app, argc, argv);

    if (*mcp) {
        std::cout << custom_bitset(CliSAT(opts.graph_filename, opts.time_limit, opts.cs_time_limit, false, opts.sorting_method, opts.AMTS_enabled, opts.threads)) << std::endl;
    } else if (*misp) {
        std::cout << custom_bitset(CliSAT(opts.graph_filename, opts.time_limit, opts.cs_time_limit, true, opts.sorting_method, opts.AMTS_enabled, opts.threads)) << std::endl;
    } else if (*nesting) {
        // std::cout << custom_bitset(CliSAT(opts.graph_filename, time_limit, true, opts.sorting_method, opts.AMTS_enabled, opts.constraints_filename)) << std::endl;
    }

    return 0;
}
