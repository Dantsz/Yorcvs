#include "common/ecs.h"
#include <cassert>

struct Transform {
    float x {};
    float y {};
    int id = 0;
};

struct Size {
    size_t w;
    size_t h;
};

class TestSystem {
public:
    TestSystem(yorcvs::ECS* pare)
    {
        parent = pare;
    }
    void test() const
    {
        for (const auto& i : *entityList) {
            assert(parent->get_component<Transform>(i).id == 1);
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* parent;
};

class SecondTestSystem {
public:
    SecondTestSystem(yorcvs::ECS* pare)
    {
        parent = pare;
    }
    void test() const
    {
        for (const auto& i : *entityList) {
            assert(parent->get_component<Transform>(i).id == 1);
            assert(parent->get_component<Size>(i).w == parent->get_component<Size>(i).h);
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* parent;
};

constexpr size_t numberOfEntities = 2000;
int main()
{
    yorcvs::ECS world {};
    world.register_component<Transform>();
    world.register_component<Size>();

    TestSystem tester(&world);
    world.register_system<TestSystem>(tester);

    world.add_criteria_for_iteration<TestSystem, Transform>();
    SecondTestSystem secontester(&world);
    world.register_system<SecondTestSystem>(secontester);
    world.add_criteria_for_iteration<SecondTestSystem, Transform, Size>();

    std::vector<yorcvs::Entity> entities {};
    for (size_t i = 0; i < numberOfEntities; i++) {
        entities.emplace_back(&world);
        world.add_component<Transform>(entities[i].id, { 1.0f, 2.0f, 1 });
        if (i % 2 == 1) {
            world.add_component<Size>(entities[i].id, { i, i });
        }
    }

    tester.test();
    secontester.test();

    for (size_t i = 0; i < numberOfEntities; i++) {
        if (i % 2 == 0) {
            world.remove_component<Transform>(entities[i].id);
        }
    }
    assert(world.get_entities_with_component<Transform>() == world.get_entities_with_component<Size>());
    assert(tester.entityList->size() == numberOfEntities / 2);
    for (size_t i = 1; i < numberOfEntities; i++) {
        if (i % 2 == 1) {
            world.remove_component<Size>(entities[i].id);
        }
    }
    assert(secontester.entityList->empty());
    return 0;
}
