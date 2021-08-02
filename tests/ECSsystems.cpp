#include "../src/ecs/ecs.h"
#include <cassert>

struct Transform
{
    float x; 
    float y;
    int id = 0;
};

struct Size
{
    size_t w;
    size_t h;
};

class TestSystem
{
    public:
    TestSystem(yorcvs::ECS* pare)
    {
        parent = pare;
    }
    void test()
    {
        for(const auto& i : entityList->entitiesID)
        {
            assert(parent->getComponent<Transform>(i).id == 1);
        }
    }
 
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* parent;
};


class SecondTestSystem
{
    public:
    SecondTestSystem(yorcvs::ECS* pare)
    {
        parent = pare;
    }
    void test()
    {
        for(const auto& i : entityList->entitiesID)
        {
            assert(parent->getComponent<Transform>(i).id == 1);
            assert(parent->getComponent<Size>(i).w == parent->getComponent<Size>(i).h);
        }
    }
 
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* parent;
};


constexpr size_t numberOfEntities = 2000;
int main()
{

    yorcvs::ECS world{};
    world.registerComponent<Transform>();
    world.registerComponent<Size>();

    TestSystem tester(&world);
    world.registerSystem<TestSystem>(tester);
    

    world.addCriteriaForIteration<TestSystem,Transform>();
    SecondTestSystem secontester(&world);
    world.registerSystem<SecondTestSystem>(secontester);
    world.addCriteriaForIteration<SecondTestSystem ,Transform, Size>();
    
    std::vector<yorcvs::Entity> entities{};
    for(size_t i = 0 ; i < numberOfEntities ; i++ )
    {
        entities.emplace_back(&world);
        world.addComponent<Transform>(entities[i].id,{1.0f,2.0f,1});
        if(i%2 == 1)
        {
            world.addComponent<Size>(entities[i].id,{i,i});
        }
    }

    tester.test();
    secontester.test();

    for(size_t i = 0 ;i < numberOfEntities; i++)
    {
        if(i%2 == 0)
        {
            world.removeComponent<Transform>(entities[i].id);
        }
    }
    assert(world.getEntitiesWithComponent<Transform>() == world.getEntitiesWithComponent<Size>());
    assert(tester.entityList->entitiesID.size() == numberOfEntities/2);
    for(size_t i = 1 ;i < numberOfEntities; i++)
    {
        if(i%2 == 1)
        {
            world.removeComponent<Size>(entities[i].id);
        }
    }
    assert(secontester.entityList->entitiesID.empty());
    return 0;
}