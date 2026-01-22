# CliSAT
Implementation of CliSAT algorithm. ([paper](https://doi.org/10.1016/j.ejor.2022.10.028))
## Building
This project uses CMake for building the binaries.

This project requires a compiler compatible with the C++23 feature set.

To build the project first clone the repository and pull all the submodules:
```
git clone https://github.com/Nj0be/CliSAT
cd CliSAT
git submodule update --init --recursive
```
Then create a build folder and initialize CMake files:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```
use `cmake -DCMAKE_BUILD_TYPE=Debug ..` for Debug builds (on linux).

Then to build the executable run:
 - on Linux -> `cmake --build .`
   The binary is located in the following path: `build/CliSAT`
 - on Windows -> `cmake --build . --config Release`. The binary is located in the following path: `build/Release/CliSAT.exe`
   
   For debug mode use `cmake --build . --config Debug`. The binary is located in the following path: `build/Debug/CliSAT.exe`
## Usage
Run CliSAT specifying as the first argument the type of problem to resolve. Currently, CliSAT can solve Maximum Clique Problems and Maximum Independent Set Problems.

To solve MCP use `CliSAT mcp`.

To solve MISP use `CliSAT misp`.

To specify the graph file (DIMACS/MTX formats) use `--graph filename`.

To specify a sorting method use `--sorting SORTING_METHOD`.

There are 5 sorting methods:
 - NO_SORT -> no sorting is applied
 - DEG_SORT
 - COLOUR_SORT
 - NEW_SORT -> (default) decides automatically between DEG_SORT and COLOUR_SORT
 - RANDOM_SORT -> nodes are ordered in a random fashion

To apply a time limit use `--time-limit SECONDS`

To apply a time limit to the colour_sort procedure use `--cs-time-limit SECONDS`

To change thread count use `--threads THREAD_COUNT`

To enable AMTS use `--amts BOOLEAN`

To enable verbose loggin use `--verbose`

# Examples

From the root directory.

Linux -> `./build/CliSAT mcp --graph examples/C250.9.clq --sorting DEG_SORT`

Windows -> `build\Release\CliSAT.exe mcp --graph examples/C250.9.clq --sorting DEG_SORT`
