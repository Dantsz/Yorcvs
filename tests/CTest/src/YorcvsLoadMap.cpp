#include "Yorcvs.h"
#include "common/utilities.h"
#include <cassert>
#include <iostream>
int main(int argc, char** argv)
{
    yorcvs::Timer timy {};
    yorcvs::ECS world {};
    timy.start();
    yorcvs::map map { TEST_MAP_FILE, &world };
    std::cout << "Loading map took " << timy.get_ticks() << '\n';

    assert(map.tilesSize == yorcvs::Vec2<float>({ 32, 32 }));

    return 0;
}
