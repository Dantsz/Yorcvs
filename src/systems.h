#pragma once
#include "components.h"
#include "ecs/ecs.h"
#include "window/windowSDL2.h"
#include <array>
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
            if (world->has_components<velocityComponent>(IDA))
            {
            
                rectA.x = world->get_component<positionComponent>(IDA).position.x + world->get_component<hitboxComponent>(IDA).hitbox.x;
                rectA.y = world->get_component<positionComponent>(IDA).position.y + world->get_component<hitboxComponent>(IDA).hitbox.y;
                rectA.w = world->get_component<hitboxComponent>(IDA).hitbox.w;
                rectA.h = world->get_component<hitboxComponent>(IDA).hitbox.h;
                yorcvs::Vec2<float> &rectAvel = world->get_component<velocityComponent>(IDA).vel;
                for (const auto &IDB : entityList->entitiesID)
                {

                    rectB.x = world->get_component<positionComponent>(IDB).position.x;
                    rectB.y = world->get_component<positionComponent>(IDB).position.y;
                    rectB.w = world->get_component<hitboxComponent>(IDB).hitbox.w + world->get_component<hitboxComponent>(IDB).hitbox.x;
                    rectB.h = world->get_component<hitboxComponent>(IDB).hitbox.h + world->get_component<hitboxComponent>(IDB).hitbox.y;
                    if (IDA != IDB)
                    {

                        // left to right
                        check_collision_left_right(rectA, rectB, rectAvel, elapsedTime);
                        // right to left
                        check_collision_right_left(rectA, rectB, rectAvel, elapsedTime);
                        // up to down
                        check_collision_up_down(rectA, rectB, rectAvel, elapsedTime);
                        // down to up
                        check_collision_down_up(rectA, rectB, rectAvel, elapsedTime);

                        // top right corner
                        check_collision_corner_top_right(rectA, rectB, rectAvel, elapsedTime);
                        // top left corner
                        check_collision_corner_top_left(rectA, rectB, rectAvel, elapsedTime);
                        // bottom right corner
                        check_collision_corner_bottom_right(rectA, rectB, rectAvel, elapsedTime);
                        // bottom left corner
                        check_collision_corner_bottom_left(rectA, rectB, rectAvel, elapsedTime);
                    }
                }
            }
        }
    }

    static bool check_collision_left_right(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                           yorcvs::Vec2<float> &rectAvel, float elapsedTime)
    {
        if (rectA.x + rectA.w <= rectB.x && rectA.x + rectA.w + (rectAvel.x * elapsedTime) > rectB.x &&
            rectA.y + rectA.h > rectB.y && rectA.y - rectB.y < rectB.h)
        {
            rectAvel.x = (rectB.x - rectA.x - rectA.w) / elapsedTime;
            return true;
        }
        return false;
    }
    static bool check_collision_right_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                           yorcvs::Vec2<float> &rectAvel, float elapsedTime)
    {
        if (rectA.x >= rectB.x + rectB.w && rectA.x + (rectAvel.x * elapsedTime) < rectB.x + rectB.w &&
            rectA.y + rectA.h > rectB.y && rectA.y - rectB.y < rectB.h)
        {
            rectAvel.x = (rectB.x + rectB.w - rectA.x) / elapsedTime;
            return true;
        }
        return false;
    }
    static bool check_collision_up_down(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                        yorcvs::Vec2<float> &rectAvel, float elapsedTime)
    {
        if (rectA.y + rectA.h <= rectB.y && rectA.y + rectA.h + (rectAvel.y * elapsedTime) > rectB.y &&
            rectA.x - rectB.x < rectB.w && rectA.x + rectA.w > rectB.x)
        {
            rectAvel.y = (rectB.y - rectA.y - rectA.h) / elapsedTime;
            return true;
        }
        return false;
    }
    static bool check_collision_down_up(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                        yorcvs::Vec2<float> &rectAvel, float elapsedTime)
    {
        if (rectA.y >= rectB.y + rectB.h && rectA.y + (rectAvel.y * elapsedTime) < rectB.y + rectB.h &&
            rectA.x - rectB.x < rectB.w && rectA.x + rectA.w > rectB.x)
        {
            rectAvel.y = (rectB.y + rectB.h - rectA.y) / elapsedTime;
            return true;
        }
        return false;
    }

    static bool check_collision_corner_top_right(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                 yorcvs::Vec2<float> &rectAvel, float elapsedTime)
    {
        if (rectB.x + rectB.w > rectA.x + (rectAvel.x * elapsedTime) &&
            rectA.y + rectA.h + (rectAvel.y * elapsedTime) > rectB.h &&
            rectA.x + (rectAvel.x * elapsedTime) > rectB.x && rectA.y < rectB.y)
        {

            rectAvel.x = ((rectB.x + rectB.w) - rectA.x) / elapsedTime;
            rectAvel.y = ((rectA.y + rectA.h) - rectB.y) / elapsedTime;
            return true;
        }
        return false;
    }
    static bool check_collision_corner_top_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                yorcvs::Vec2<float> &rectAvel, float elapsedTime)
    {
        if (rectB.x + rectB.w > rectA.x + (rectAvel.x * elapsedTime) &&
            rectA.y + rectA.h + (rectAvel.y * elapsedTime) > rectB.h &&
            rectA.x + (rectAvel.x * elapsedTime) > rectB.x && rectA.y < rectB.y)
        {

            rectAvel.x = ((rectB.x + rectB.w) - rectA.x) / elapsedTime;
            rectAvel.y = ((rectA.y + rectA.h) - rectB.y) / elapsedTime;
            return true;
        }
        return false;
    }
    static bool check_collision_corner_bottom_right(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                    yorcvs::Vec2<float> &rectAvel, float elapsedTime)
    {
        if (rectB.x < rectA.x + rectA.w + (rectAvel.x * elapsedTime) &&
            (rectB.y + rectB.h) > rectA.y + (rectAvel.y * elapsedTime) && rectA.x < rectB.x &&
            rectA.y + rectA.h > rectB.h)
        {
            rectAvel.x = (rectB.x - (rectA.x + rectA.w)) / elapsedTime;
            rectAvel.y = ((rectB.y + rectB.h) - rectA.y) / elapsedTime;
            return true;
        }
        return false;
    }
    static bool check_collision_corner_bottom_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                   yorcvs::Vec2<float> &rectAvel, float elapsedTime)
    {
        if (rectB.x + rectB.w > rectA.x + (rectAvel.x * elapsedTime) &&
            (rectB.y + rectB.h) > rectA.y + (rectAvel.y * elapsedTime) && rectA.x > rectB.x && rectB.y < rectA.y)
        {
            rectAvel.x = ((rectB.x + rectB.w) - rectA.x) / elapsedTime;
            rectAvel.y = ((rectB.y + rectB.h) - rectA.y) / elapsedTime;
            return true;
        }
        return false;
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



class AnimationSystem
{
    public:

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};
