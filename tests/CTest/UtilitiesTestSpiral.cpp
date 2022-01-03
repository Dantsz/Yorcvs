#include<iostream>
#include <cassert>
#include "../../src/common/utilities.h"

constexpr size_t limit = 100000;

int main(int argc, char **argv) 
{
    for(size_t i = 0; i < limit; i++)
    {
        std::cout<< std::get<0>(yorcvs::spiral::wrap(i)) << ' ' << std::get<1>(yorcvs::spiral::wrap(i)) << '\n';
        assert(i == yorcvs::spiral::unwrap(std::get<0>(yorcvs::spiral::wrap(i)),std::get<1>(yorcvs::spiral::wrap(i))));
    }
    return 0;
}