#include <iostream>
//#include "graph/graph.h"
#include "bitscan_benchmark.h"
#include "custom_bitset.h"

int main() {
    /*
    ugraph g(100); //creates an empty (undirected) graph with 100 vertices
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 2);
    */

    auto bb = custom_bitset(100);
    bb.setBit(5);
    bb.setBit(10);
    bb.setBit(20);
    bb.setBit(50);
    bb.setBit(80);
    bb.print();
    uint64_t bit = 0;
    do {
        std::cout << bit << " ";
    } while((bit = bb.next_bit()) != bb.size());

    std::cout << std::endl;

    bitarray bbi(100);
    bbi.set_bit(10);
    bbi.set_bit(20);
    bbi.set_bit(25);
    bbi.set_bit(70);
    bbi.set_bit(85);
    std::cout << bbi;

    bbi.init_scan(bbo::NON_DESTRUCTIVE);
    int nBit = 0;
    while( (nBit = bbi.next_bit()) != BBObject::noBit ) {
        std::cout << nBit << " ";
    }
    std::cout << std::endl;


    /*
    bitscan_benchmark();

    constexpr uint64_t a = 0x0800000000000800;

    // best balance between various x86_64 architectures and portability
    std::cout << std::bit_width(a) - 1 << std::endl;
    std::cout << std::countr_zero(a) << std::endl;
    */

    //bitscan_benchmark1();
    std::cout << __builtin_ctzll(0b001) << std::endl;
    //bitscan_benchmark2();
    bitwise_and_benchmark();

    return 0;
}
