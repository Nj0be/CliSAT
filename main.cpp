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

    auto bb = custom_bitset(128);
    bb.set_bit(0);
    bb.set_bit(5);
    bb.set_bit(10);
    bb.set_bit(20);
    bb.set_bit(50);
    bb.set_bit(64);
    bb.set_bit(63);
    bb.set_bit(80);
    bb.set_bit(127);
    std::cout << (~bb) << std::endl;

    uint64_t bit = bb.first_bit();
    do {
        std::cout << bit << " ";
    } while((bit = bb.next_bit()) != bb.size());
    std::cout << std::endl;

    bit = bb.last_bit();
    do {
        std::cout << bit << " ";
    } while((bit = bb.prev_bit()) != bb.size());
    std::cout << std::endl;

    auto bb2 = custom_bitset(bb);
    auto bb3 = bb & bb2;

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


    //bitscan_benchmark1();
    bit_scan_forward_benchmark();
    bit_scan_reverse_benchmark();
    //bitwise_and_benchmark();

    return 0;
}
