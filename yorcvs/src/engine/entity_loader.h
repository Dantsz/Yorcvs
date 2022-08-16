#include "../common/ecs.h"
#include "../game/componentSerialization.h"
#pragma once

template <typename... Components>
class Entity_Loader {
public:
    Entity_Loader(yorcvs::ECS* world)
        : world { world }
    {
    }
    void load_entity_from_path()
    {
    }
    std::string save_entity_as_string()
    {
    }

private:
    yorcvs::ECS* world;
};
