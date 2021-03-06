#pragma once
#include "../../common/ecs.h"
#include "../components.h"
/**
 * @brief Handles the health of an entity, the regeneration of health , and it deletes the entity if the health is
 * negative
 *
 */
class HealthSystem {
public:
    explicit HealthSystem(yorcvs::ECS* parent)
        : world(parent)
    {
        world->register_system<HealthSystem>(*this);
        world->add_criteria_for_iteration<HealthSystem, healthComponent>();
    }
    void update(const float dt)
    {
        cur_time += dt;
        for (size_t i = 0; i < entityList->entitiesID.size(); i++) // enchanced for doesn't work here because it can invalidate iterators
        {
            const size_t ID = entityList->entitiesID[i];
            if (world->get_component<healthComponent>(ID).HP < 0.0f) {
                world->get_component<healthComponent>(ID).is_dead = true;
                world->destroy_entity(ID);
                i--;
                continue;
            }
        }
        if (cur_time >= update_time) {
            for (const auto& ID : entityList->entitiesID) {
                world->get_component<healthComponent>(ID).HP += world->get_component<healthComponent>(ID).health_regen;
                if (world->get_component<healthComponent>(ID).HP > world->get_component<healthComponent>(ID).max_HP) {
                    world->get_component<healthComponent>(ID).HP = world->get_component<healthComponent>(ID).max_HP;
                }
            }
            cur_time = 0.0f;
        }
    }
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* world;
    static constexpr float update_time = 1000.0f; // update once a second
    float cur_time = 0.0f;
};
