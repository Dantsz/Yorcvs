#include "common/ecs.h"
#include <cassert>
#include <memory>

struct testComp {
    int x;
    int y;
};

class testSystem {
public:
    testSystem() = default;
    testSystem(yorcvs::ECS* p)
        : parent(p)
    {
    }
    [[nodiscard]] int do_work() const
    {
        for (const auto& i : *entityList) {
            if (parent->has_components<testComp>(i)) {
                std::cout << parent->get_component<testComp>(i).x << ' ' << parent->get_component<testComp>(i).y << '\n';
                return (parent->get_component<testComp>(i).x + parent->get_component<testComp>(i).y);
            }
        }
        return 0;
    }
    yorcvs::ECS* parent {};
    std::shared_ptr<yorcvs::entity_system_list> entityList;
};

int main()
{
    yorcvs::ECS ecs {};
    ecs.register_component<testComp>();
    testSystem sys(&ecs);
    ecs.register_system(sys);
    ecs.add_criteria_for_iteration<testSystem, testComp>();
    yorcvs::Entity enti { &ecs };
    ecs.add_component<testComp>(enti.id, { 1, 69 });
    assert(sys.do_work() == 70);
    testSystem system2 {};
    system2.parent = &ecs;
    system2.entityList = ecs.get_system_entity_list<testSystem>();
    assert(system2.do_work() == 70);
}
