#include "../../../src/common/ecs.h"
#include <cassert>
struct Transform
{
    float x{}; 
    float y{};
    int id = 0;
};

int main()
{

    yorcvs::ECS world{};
    world.register_component<Transform>();
    yorcvs::Entity position(&world);
    
    world.add_component<Transform>(position.id,{1.0f,1.0f});

    yorcvs::Entity e1{&world};
    yorcvs::Entity e2{&world};
    world.add_component<Transform>(e1.id,{});
    world.copy_components_to_from_entity(e2.id,e1.id);
    const auto& sig1 = world.get_entity_signature(e1.id);
    const auto& sig2 = world.get_entity_signature(e2.id);
    assert(sig1.size() == sig2.size());
    for(size_t i = 0; i < sig1.size(); i ++)
    {
        assert(sig1[i] == sig2[i]);
    }
    return 0;
}