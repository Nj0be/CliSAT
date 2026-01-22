# CliSAT
Implementation of CliSAT algorithm. ([paper](https://doi.org/10.1016/j.ejor.2022.10.028))
## Building
This project uses CMake for building the binaries.

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
use `cmake -DCMAKE_BUILD_TYPE=Debug ..` for Debug builds.

Then to build the executable run
```
cmake --build .
```
