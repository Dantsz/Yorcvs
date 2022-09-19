#include "Yorcvs.h"
#include "common/utilities.h"
#include <cassert>
#include <iostream>
int main(int argc, char** argv)
{
    yorcvs::timer timy {};
    yorcvs::ECS world {};
    timy.start();
    yorcvs::map map { TEST_MAP_FILE, &world };
    yorcvs::entity entity { &world };
    map.load_entity_from_path(entity.id, TEST_LOAD_ENTITY_FILE);
    const auto ok = world.has_components<identification_component, hitbox_component, health_component, sprite_component>(entity.id);
    assert(ok);
    // map.load_entity_from_path(entity.id, "file does not exist");
    return 0;
}
