#pragma once
#include "components.h"
#include "ecs/ecs.h"
#include "window/windowSDL2.h"
class CollisionSystem
{
  public:
    CollisionSystem(yorcvs::ECS *parent)
    {
        world = parent;
        // is this legal? yee
        if (!world->is_component_registered<hitboxComponent>())
        {
            world->register_component<hitboxComponent>();
        }
        if (!world->is_component_registered<positionComponent>())
        {
            world->register_component<positionComponent>();
        }

        world->register_system<CollisionSystem>(*this);

        world->add_criteria_for_iteration<CollisionSystem, positionComponent, hitboxComponent>();
    }
   
    void update(float elapsedTime) const // checks and resolves collisions
    {
        yorcvs::Rect<float> rectA{};
        yorcvs::Rect<float> rectB{};
        for (const auto &IDA : entityList->entitiesID)
        {
            rectA.x = world->get_component<positionComponent>(IDA).position.x;
            rectA.y = world->get_component<positionComponent>(IDA).position.y;
            rectA.w = world->get_component<hitboxComponent>(IDA).hitbox.w;
            rectA.h = world->get_component<hitboxComponent>(IDA).hitbox.h;

            for (const auto &IDB : entityList->entitiesID)
            {
                rectB.x = world->get_component<positionComponent>(IDB).position.x;
                rectB.y = world->get_component<positionComponent>(IDB).position.y;
                rectB.w = world->get_component<hitboxComponent>(IDB).hitbox.w;
                rectB.h = world->get_component<hitboxComponent>(IDB).hitbox.h;
                if (IDA != IDB && AABBColision(rectA, rectB))
                {
                    yorcvs::log("Collision between " + std::to_string(IDA) + " and " + std::to_string(IDB));
                }
            }
        }
    }

    void render(yorcvs::Window<yorcvs::SDL2> *testWindow) const
    {
        for (const auto &ID : entityList->entitiesID)
        {
            yorcvs::Vec2<float> pos = world->get_component<positionComponent>(ID).position;
            pos += {world->get_component<hitboxComponent>(ID).hitbox.x,
                    world->get_component<hitboxComponent>(ID).hitbox.y};
            yorcvs::Vec2<float> size = {world->get_component<hitboxComponent>(ID).hitbox.w,
                                        world->get_component<hitboxComponent>(ID).hitbox.h};
            testWindow->draw_sprite("assets/lettuce.png", pos, size, {0, 0, 200, 200});
        }
    }
    static constexpr bool AABBColision(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB)
    {
        return ((rectA.x + rectA.w > rectB.x) && (rectA.x < rectB.x + rectB.w) && (rectA.y + rectA.h > rectB.y) &&
                (rectA.y < rectB.y + rectB.h));
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};

class VelocitySystem
{
  public:
    VelocitySystem(yorcvs::ECS *parent)
    {
        world = parent;
        if (!world->is_component_registered<positionComponent>())
        {
            world->register_component<positionComponent>();
        }
        if (!world->is_component_registered<velocityComponent>())
        {
            world->register_component<velocityComponent>();
        }
        world->register_system<VelocitySystem>(*this);
        world->add_criteria_for_iteration<VelocitySystem, positionComponent, velocityComponent>();
    }
    void update(float dt) const
    {
        for (const auto &ID : entityList->entitiesID)
        {
            yorcvs::Vec2<float> posOF = world->get_component<velocityComponent>(ID).vel;
            posOF *= dt;
            world->get_component<positionComponent>(ID).position += posOF;
            world->get_component<velocityComponent>(ID) = {{0, 0}};
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};
