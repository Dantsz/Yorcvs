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

    
    std::vector<yorcvs::Entity> entities{};
    for(auto i = 0 ; i < 100 ; i ++)
    {
        entities.emplace_back(&world);
        world.addComponent<Transform>(entities[i].id,{1.0f,1.0f,i});
    }
    assert(world.getEntitiesWithComponent<Transform>() == 100);
    world.destroyEntity(0);
    assert(world.getEntitiesWithComponent<Transform>() == 99);
    assert(world.hasComponents<Transform>(0) == 0);
    return 0;
}
