#include "Yorcvs.h"
#include "common/utilities.h"
#include <cassert>
#include <iostream>

int main(int argc, char** argv)
{
    yorcvs::ECS world {};
    yorcvs::map map(&world);
    yorcvs::Entity duck { &world };
    map.load_character_from_path(duck.id, TEST_TEMP_ENTITY_FILE);
    bool duck_check = world.has_components<health_component>(duck.id);
    assert(duck_check == true);
    duck_check = world.has_components<hitbox_component>(duck.id);
    assert(duck_check == true);
    assert((world.get_component<hitbox_component>(duck.id).hitbox == yorcvs::Rect<float> { 6, 6, 4, 4 }));
    duck_check = world.has_components<sprite_component>(duck.id);
    assert(duck_check == true);
    assert((world.get_component<sprite_component>(duck.id).src_rect == yorcvs::Rect<size_t> { 0, 0, 16, 16 }));
    duck_check = world.has_components<animation_component>(duck.id);
    assert(duck_check == true);
    assert(world.get_component<animation_component>(duck.id).animation_name_to_start_frame_index.size() == 4);
    duck_check = world.has_components<velocity_component>(duck.id);
    assert(duck_check == true);
    duck_check = world.has_components<hitbox_component, sprite_component, animation_component, velocity_component>(duck.id);
    assert(duck_check == true);

    return 0;
}
