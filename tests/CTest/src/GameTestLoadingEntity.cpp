#include "../../../src/Yorcvs.h"
#include "../../../src/common/utilities.h"
#include <cassert>
#include <iostream>

int main(int argc, char** argv)
{
    yorcvs::ECS world {};
    yorcvs::Map map(&world);
    yorcvs::Entity duck { &world };
    map.load_character_from_path(duck, TEST_TEMP_ENTITY_FILE);
    bool duck_check = world.has_components<healthComponent>(duck.id);
    assert(duck_check == true);
    duck_check = world.has_components<hitboxComponent>(duck.id);
    assert(duck_check == true);
    assert((world.get_component<hitboxComponent>(duck.id).hitbox == yorcvs::Rect<float> { 6, 6, 4, 4 }));
    duck_check = world.has_components<spriteComponent>(duck.id);
    assert(duck_check == true);
    assert((world.get_component<spriteComponent>(duck.id).src_rect == yorcvs::Rect<size_t> { 0, 0, 16, 16 }));
    duck_check = world.has_components<animationComponent>(duck.id);
    assert(duck_check == true);
    assert(world.get_component<animationComponent>(duck.id).animation_name_to_start_frame_index.size() == 4);
    duck_check = world.has_components<velocityComponent>(duck.id);
    assert(duck_check == true);
    duck_check = world.has_components<hitboxComponent, spriteComponent, animationComponent, velocityComponent>(duck.id);
    assert(duck_check == true);

    return 0;
}
