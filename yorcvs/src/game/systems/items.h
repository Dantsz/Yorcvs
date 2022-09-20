#pragma once
#include "../../common/ecs.h"

class item_system {
public:
    explicit item_system(yorcvs::ECS* parent)
        : world(parent)
    {
        world->register_system<item_system>(*this);
    }
    void on_entity_removed(size_t entity_id)
    {
        yorcvs::log("Deleting inventory from : " + std::to_string(entity_id));
    }
    std::shared_ptr<yorcvs::entity_system_list> entityList;

private:
    yorcvs::ECS* world;
};
