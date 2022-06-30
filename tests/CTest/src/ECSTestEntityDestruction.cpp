#include "common/ecs.h"
#include <cassert>
int main()
{
    yorcvs::ECS world {};
    const auto ID1 = world.create_entity_ID();
    world.destroy_entity(1000);
    assert(world.get_active_entities_number() == 1);
    world.destroy_entity(ID1);
    assert(world.get_active_entities_number() == 0);
    return 0;
}
