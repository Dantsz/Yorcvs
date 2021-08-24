#include "../src/common/ecs.h"
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

    assert(world.get_entities_with_component<Transform>() == 1);

    assert(world.get_component<Transform>(position.id).id == 0);

    world.get_component<Transform>(position.id).id = 42;
    yorcvs::Entity second = position;

    assert(world.get_component<Transform>(second.id).id == 42);

    world.get_component<Transform>(second.id).id = 120;

    assert(world.get_component<Transform>(second.id).id == 120);
    assert(world.get_component<Transform>(position.id).id == 42);

    return 0;
}
