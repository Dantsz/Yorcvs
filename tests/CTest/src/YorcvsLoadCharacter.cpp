#include "Yorcvs.h"
#include "common/utilities.h"
#include <cassert>
#include <iostream>
int main(int argc, char** argv)
{
    yorcvs::Timer timy {};
    yorcvs::ECS world {};
    timy.start();
    yorcvs::Map map { TEST_MAP_FILE, &world };
    yorcvs::Entity entity { &world };
    map.load_character_from_path(entity.id, TEST_LOAD_ENTITY_FILE);
    const auto ok = world.has_components<identificationComponent, hitboxComponent, healthComponent, spriteComponent>(entity.id);
    assert(ok);
    map.load_character_from_path(entity.id, "file does not exist");
    return 0;
}
