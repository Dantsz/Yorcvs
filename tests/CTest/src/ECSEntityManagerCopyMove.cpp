#include "common/ecs.h"
#include <cassert>

int main()
{
    yorcvs::entity_manager mngr1 {};
    mngr1.addEntity();
    yorcvs::entity_manager MNGR2 = mngr1;
    assert(MNGR2.entitySignatures.size() == 1);
    return 0;
}
