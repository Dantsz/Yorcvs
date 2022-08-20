#pragma once
#include "../../common/ecs.h"
#include "../components.h"
/**
 * @brief Handles the movement of entities
 *
 */
class VelocitySystem {
public:
    explicit VelocitySystem(yorcvs::ECS* parent)
        : world(parent)
    {
        world->register_system<VelocitySystem>(*this);
        world->add_criteria_for_iteration<VelocitySystem, position_component, velocity_component>();
    }
    void update(float dt) const
    {
        for (const auto& ID : *entityList) {
            yorcvs::Vec2<float> posOF = world->get_component<velocity_component>(ID).vel;
            posOF *= dt; // multiply by passed time`
            world->get_component<position_component>(ID).position += posOF;

            if (std::abs(posOF.x) > std::numeric_limits<float>::epsilon()) {
                world->get_component<velocity_component>(ID).facing.x = (posOF.x < 0.0f);
            }
            if (std::abs(posOF.y) > std::numeric_limits<float>::epsilon()) {
                world->get_component<velocity_component>(ID).facing.y = (posOF.y < 0.0f);
            }
        }
    }
    std::shared_ptr<yorcvs::entity_system_list> entityList;
    yorcvs::ECS* world;
};
