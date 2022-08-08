#pragma once
#include "../../common/ecs.h"
#include "../components.h"
/**
 * @brief Handles stamina and stamina regeneration
 *
 */
class StaminaSystem {
public:
    explicit StaminaSystem(yorcvs::ECS* parent)
        : world(parent)
    {
        world->register_system(*this);
        world->add_criteria_for_iteration<StaminaSystem, staminaComponent, staminaStatsComponent>();
    }

    void update(const float dt)
    {
        cur_time += dt;
        if (cur_time >= update_time) {
            for (const auto& ID : *entityList) {
                world->get_component<staminaComponent>(ID).stamina += world->get_component<staminaStatsComponent>(ID).stamina_regen;
                if (world->get_component<staminaComponent>(ID).stamina > world->get_component<staminaStatsComponent>(ID).max_stamina) {
                    world->get_component<staminaComponent>(ID).stamina = world->get_component<staminaStatsComponent>(ID).max_stamina;
                }
            }
            cur_time = 0.0f;
        }
    }
    static constexpr float update_time = 1000.0f;
    float cur_time = 0.0f;
    yorcvs::ECS* world = nullptr;
    std::shared_ptr<yorcvs::EntitySystemList> entityList = nullptr;
};
