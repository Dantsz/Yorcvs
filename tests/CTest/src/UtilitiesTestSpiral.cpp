#include "common/utilities.h"
#include <cassert>
#include <fstream>
#include <iostream>

constexpr size_t limit = 1000;
std::ifstream in("testInput/spiralNumbersDiagonal.txt");

int main(int argc, char** argv)
{
    for (size_t i = 0; i < limit; i++) {
        size_t value = 0;
        in >> value;
        size_t actual = yorcvs::spiral::unwrap(i, i);
        assert(actual == value);
    }

    for (size_t i = 0; i < limit; i++) {
        // std::cout<< std::get<0>(yorcvs::spiral::wrap(i)) << ' ' << std::get<1>(yorcvs::spiral::wrap(i)) << '\n';
        assert(i == yorcvs::spiral::unwrap(std::get<0>(yorcvs::spiral::wrap(i)), std::get<1>(yorcvs::spiral::wrap(i))));
    }
    return 0;
}
