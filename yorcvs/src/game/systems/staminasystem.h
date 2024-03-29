#pragma once
#include "../../common/ecs.h"
#include "../components.h"
/**
 * @brief Handles stamina and stamina regeneration
 *
 */
class stamina_system {
public:
    explicit stamina_system(yorcvs::ECS* parent)
        : world(parent)
    {
        world->register_system(*this);
        world->add_criteria_for_iteration<stamina_system, stamina_component, stamina_stats_component>();
    }

    void update(const float dt)
    {
        cur_time += dt;
        if (cur_time >= update_time) {
            for (const auto& ID : *entityList) {
                world->get_component<stamina_component>(ID).stamina += world->get_component<stamina_stats_component>(ID).stamina_regen;
                if (world->get_component<stamina_component>(ID).stamina > world->get_component<stamina_stats_component>(ID).max_stamina) {
                    world->get_component<stamina_component>(ID).stamina = world->get_component<stamina_stats_component>(ID).max_stamina;
                }
            }
            cur_time = 0.0f;
        }
    }
    static constexpr float update_time = 1000.0f;
    float cur_time = 0.0f;
    yorcvs::ECS* world = nullptr;
    std::shared_ptr<yorcvs::entity_system_list> entityList = nullptr;
};
