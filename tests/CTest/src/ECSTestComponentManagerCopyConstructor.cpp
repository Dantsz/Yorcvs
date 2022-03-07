#include "../../../src/common/ecs.h"
#include <cassert>


struct testC
{
    int x;
    int y;
};

int main()
{
    assert(1 == 1);
    yorcvs::ComponentManager managerOriginal{};
    yorcvs::ComponentManager managerNew{};
   
    managerOriginal.register_component<testC>();
    testC c{1,1};
    managerOriginal.add_component<testC>(0,c);
    managerOriginal.get_component<testC>(0).x = 10;
    assert(managerOriginal.get_component<testC>(0).x == 10);
    managerNew = managerOriginal;
    assert(managerNew.get_component<testC>(0).x == 10);
    return 0;
}