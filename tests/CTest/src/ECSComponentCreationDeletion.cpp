#include "common/ecs.h"
#include <cassert>
struct Transform {
    float x {};
    float y {};
    int id = 0;
};
constexpr size_t nrentities = 100;
int main()
{
    yorcvs::ECS world {};
    world.register_component<Transform>();
    std::vector<yorcvs::entity> entities {};
    for (auto i = 0; i < nrentities; i++) {
        entities.emplace_back(&world);
        world.add_component<Transform>(entities[i].id, { 1.0f, 1.0f, i });
    }
    assert(world.get_entities_with_component<Transform>() == 100);
    world.destroy_entity(0);
    assert(world.get_entities_with_component<Transform>() == 99);
    assert(world.has_components<Transform>(0) == 0);
    return 0;
}
