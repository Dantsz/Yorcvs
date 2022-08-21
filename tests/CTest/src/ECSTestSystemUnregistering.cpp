#include "common/ecs.h"
#include <cassert>
struct Transform {
    float x {};
    float y {};
    int id = 0;
};
class TestSystem {
public:
    TestSystem(yorcvs::ECS* pare)
        : parent(pare)
    {
    }
    void test_size_one_id_one() const
    {
        assert(entityList->size() == 1);
        for (const auto& i : *entityList) {
            assert(parent->get_component<Transform>(i).id == 1);
        }
    }
    void test_size_two_id_one() const
    {
        assert(entityList->size() == 2);
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
    TestSystem tester(&world);
    world.register_system<TestSystem>(tester);
    world.add_criteria_for_iteration<TestSystem, Transform>();

    assert(world.is_system_registered<TestSystem>());
    yorcvs::entity en { &world };
    world.add_component<Transform>(en.id, { .id = 1 });
    assert(tester.entityList->size() == 1);
    tester.test_size_one_id_one();

    world.unregister_system<TestSystem>();
    assert(!world.is_system_registered<TestSystem>());
    assert(tester.entityList->empty());
    yorcvs::entity en2 { &world };
    world.add_component<Transform>(en2.id, { .id = 1 });

    world.register_system<TestSystem>(tester);
    world.add_criteria_for_iteration<TestSystem, Transform>();
    assert(world.is_system_registered<TestSystem>());
    assert(tester.entityList->size() == 2);
    tester.test_size_two_id_one();

    world.unregister_system<TestSystem>();
    assert(!world.is_system_registered<TestSystem>());
    assert(tester.entityList->empty());
}
