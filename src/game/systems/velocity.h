#pragma once
#include "../../common/ecs.h"
#include "../components.h"
/**
 * @brief Handles the movement of entities
 *
 */
class VelocitySystem
{
  public:
    explicit VelocitySystem(yorcvs::ECS *parent) : world(parent)
    {
        world->register_system<VelocitySystem>(*this);
        world->add_criteria_for_iteration<VelocitySystem, positionComponent, velocityComponent>();
    }
    void update(float dt) const
    {
        for (const auto &ID : entityList->entitiesID)
        {
            yorcvs::Vec2<float> posOF = world->get_component<velocityComponent>(ID).vel;
            posOF *= dt; // multiply by passed time`
            world->get_component<positionComponent>(ID).position += posOF;

            if (std::abs(posOF.x) > std::numeric_limits<float>::epsilon())
            {
                world->get_component<velocityComponent>(ID).facing.x = (posOF.x < 0.0f);
            }
            if (std::abs(posOF.y) > std::numeric_limits<float>::epsilon())
            {
                world->get_component<velocityComponent>(ID).facing.y = (posOF.y < 0.0f);
            }
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};
