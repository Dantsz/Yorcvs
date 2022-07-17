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
    yorcvs::Entity position(&world);

    world.add_component<Transform>(position.id, { 1.0f, 1.0f });

    // ok
    const auto sig_ok = world.get_entity_signature(position.id);
    assert(sig_ok.size() == 1);
    // not ok
    world.remove_component<Transform>(position.id);
    const auto sig_zerod = world.get_entity_signature(position.id);
    assert(sig_zerod.size() == 1 && sig_zerod[0] == false);

    const auto sig_empty = world.get_entity_signature(2);
    assert(sig_empty.empty());
}
