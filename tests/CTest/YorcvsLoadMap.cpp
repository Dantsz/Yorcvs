#include<iostream>
#include "../../src/Yorcvs.h"
#include "../../src/common/utilities.h"
int main(int argc, char **argv) 
{
    yorcvs::Timer timy{};
    yorcvs::ECS world{};
    timy.start();
    yorcvs::Map map{"assets/map.tmx",&world};
    std::cout<< "Loading map took " << timy.get_ticks() << '\n';

    
    return 0;
}