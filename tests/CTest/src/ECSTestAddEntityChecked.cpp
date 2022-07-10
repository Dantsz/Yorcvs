#include "../../../src/common/ecs.h"
#include <cassert>

struct Transform {
    float x {};
    float y {};
    int id = 0;
};

int main()
{
    yorcvs::ECS world {};
    world.register_component<Transform>();
    yorcvs::Entity ent { &world };
    yorcvs::Entity ent2 { &world };

    world.add_component<Transform>(ent.id, { .x = 10.0f, .y = 15.0f, .id = 15 });
    const auto cmp = world.get_component_checked<Transform>(ent.id);
    assert(cmp.has_value());
    const auto cmp2 = world.get_component_checked<Transform>(ent2.id);
    assert(!cmp2.has_value());

    assert(cmp->get().id == 15);
    assert(cmp.value().get().id == 15);
    return 0;
}
