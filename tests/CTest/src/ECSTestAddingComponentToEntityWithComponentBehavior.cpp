#include "../../../src/common/ecs.h"
#include <cassert>
struct Transform {
    float x {};
    float y {};
    int id = 0;
};

int main()
{
    yorcvs::ComponentManager mngr {};
    assert(mngr.component_type.empty());
    mngr.register_component<Transform>();
    const size_t ID = 0;
    Transform comp {};
    mngr.add_component(ID, comp);
    assert(mngr.component_type.size() == 1);
    assert(mngr.componentContainers.size() == 1);
    for (const auto& [name, components] : mngr.componentContainers) {
        assert(components->get_allocated_components() == 1);
    }
    mngr.add_component(ID, comp);
    for (const auto& [name, components] : mngr.componentContainers) {
        assert(components->get_allocated_components() == 1);
    }
}
