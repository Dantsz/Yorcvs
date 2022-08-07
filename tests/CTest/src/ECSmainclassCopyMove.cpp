#include "common/ecs.h"
#include <cassert>
#include <memory>

struct point {
    int x, y;
};

struct pointLister {
    void list_points() const
    {
        for (const auto& i : *entityList) {
            std::cout << parent->get_component<point>(i).x << ' ' << parent->get_component<point>(i).y << '\n';
        }
    }
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* parent {};
};

int main()
{
    yorcvs::ECS ecs;
    ecs.register_component<point>();
    pointLister lister;
    ecs.register_system(lister);
    yorcvs::Entity origin { &ecs };
    ecs.add_component<point>(origin.id, { 0, 0 });
    lister.parent = &ecs;
    lister.list_points();

    yorcvs::ECS ecsd;
    ecsd = ecs;
    ecs.register_system(lister);
    lister.parent = &ecsd;
    lister.list_points();
}
