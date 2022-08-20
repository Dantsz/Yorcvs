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
        world->add_criteria_for_iteration<HealthSystem, health_component, health_stats_component>();
    }
    void update(const float dt)
    {
        cur_time += dt;
        for (size_t i = 0; i < entityList->size(); i++) // enchanced for doesn't work here because it can invalidate iterators
        {
            const size_t ID = (*entityList)[i];
            if (world->get_component<health_component>(ID).HP < 0.0f) {
                world->get_component<health_component>(ID).is_dead = true;
                world->destroy_entity(ID);
                i--;
                continue;
            }
        }
        if (cur_time >= update_time) {
            for (const auto& ID : *entityList) {
                world->get_component<health_component>(ID).HP += world->get_component<health_stats_component>(ID).health_regen;
                if (world->get_component<health_component>(ID).HP > world->get_component<health_stats_component>(ID).max_HP) {
                    world->get_component<health_component>(ID).HP = world->get_component<health_stats_component>(ID).max_HP;
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
