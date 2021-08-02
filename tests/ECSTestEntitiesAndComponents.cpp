#include "../src/ecs/ecs.h"
#include <cassert>
struct Transform
{
    float x; 
    float y;
    int id = 0;
};

int main()
{

    yorcvs::ECS world{};
    world.registerComponent<Transform>();
    yorcvs::Entity position(&world);
    
    world.addComponent<Transform>(position.id,{1.0f,1.0f});

    assert(world.getEntitiesWithComponent<Transform>() == 1);

    assert(world.getComponent<Transform>(position.id).id == 0);

    world.getComponent<Transform>(position.id).id = 42;
    yorcvs::Entity second = position;

    assert(world.getComponent<Transform>(second.id).id == 42);

    world.getComponent<Transform>(second.id).id = 120;

    assert(world.getComponent<Transform>(second.id).id == 120);
    assert(world.getComponent<Transform>(position.id).id == 42);

    return 0;
}
