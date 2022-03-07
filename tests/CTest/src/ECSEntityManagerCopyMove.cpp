#include "../../../src/common/ecs.h"
#include <cassert>

int main()
{
    yorcvs::EntityManager mngr1{};
    mngr1.addEntity();
    yorcvs::EntityManager MNGR2 = mngr1;
    assert(MNGR2.lowestUnallocatedID == 1);

    return 0;
}