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
        parent->set_criteria_for_iteration<TestSystem>();
    }
    void test() const
    {
        for (const auto& i : *entityList) {
            assert(parent->get_component<Transform>(i).id == 1);
        }
    }

    std::shared_ptr<yorcvs::entity_system_list> entityList;
    yorcvs::ECS* parent;
};
int main()
{
    yorcvs::ECS world {};
    world.register_component<Transform>();
    world.register_component<Size>();

    TestSystem tester(&world);
    world.register_system<TestSystem>(tester);

    const auto E1 = world.create_entity_ID();
    const auto E2 = world.create_entity_ID();
    const auto E3 = world.create_entity_ID();
    world.add_default_components<Transform, Size>(E1);
    world.add_default_component<Size>(E2);
    world.add_default_component<Transform>(E3);
    assert(tester.entityList->size() == 3);
    world.add_criteria_for_iteration<TestSystem, Size>();
    assert(tester.entityList->size() == 2);
    world.add_criteria_for_iteration<TestSystem, Transform>();
    assert(tester.entityList->size() == 1);

    const auto sig1 = world.get_system_signature<TestSystem>();
    world.set_criteria_for_iteration<TestSystem, Transform>();
    const auto sig2 = world.get_system_signature<TestSystem>();
    assert(tester.entityList->size() == 2);
}
