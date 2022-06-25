#pragma once
#include "../../common/ecs.h"
#include "../components.h"
/**
 * @brief Handles collision between entities
 *
 */
class CollisionSystem
{
  public:
    explicit CollisionSystem(yorcvs::ECS *parent) : world(parent)
    {
        // is this legal? yee
        world->register_system<CollisionSystem>(*this);
        world->add_criteria_for_iteration<CollisionSystem, positionComponent, hitboxComponent>();
    }
    /**
     * @brief
     *
     * @param dt time passed
     */
    void update(float dt) const // checks and resolves collisions
    {
        yorcvs::Rect<float> rectA{};
        yorcvs::Rect<float> rectB{};

        for (const auto &IDA : entityList->entitiesID)
        {
            if (world->has_components<velocityComponent>(IDA))
            {
                rectA.x = world->get_component<positionComponent>(IDA).position.x +
                          world->get_component<hitboxComponent>(IDA).hitbox.x;
                rectA.y = world->get_component<positionComponent>(IDA).position.y +
                          world->get_component<hitboxComponent>(IDA).hitbox.y;
                rectA.w = world->get_component<hitboxComponent>(IDA).hitbox.w;
                rectA.h = world->get_component<hitboxComponent>(IDA).hitbox.h;
                yorcvs::Vec2<float> &rectAvel = world->get_component<velocityComponent>(IDA).vel;
                rectAvel *= dt;
                for (const auto &IDB : entityList->entitiesID)
                {
                    if (!world->has_components<velocityComponent>(IDB))
                    {
                        rectB.x = world->get_component<positionComponent>(IDB).position.x +
                                  world->get_component<hitboxComponent>(IDB).hitbox.x;
                        rectB.y = world->get_component<positionComponent>(IDB).position.y +
                                  world->get_component<hitboxComponent>(IDB).hitbox.y;

                        rectB.w = world->get_component<hitboxComponent>(IDB).hitbox.w;

                        rectB.h = world->get_component<hitboxComponent>(IDB).hitbox.h;
                        if (IDA != IDB)
                        {
                            // left to right
                            check_collision_left_right(rectA, rectB, rectAvel, dt);
                            // right to left
                            check_collision_right_left(rectA, rectB, rectAvel, dt);
                            // up to down
                            check_collision_up_down(rectA, rectB, rectAvel, dt);
                            // down to up
                            check_collision_down_up(rectA, rectB, rectAvel, dt);

                            // top right corner
                            check_collision_corner_top_right(rectA, rectB, rectAvel, dt);
                            // top left corner
                            check_collision_corner_top_left(rectA, rectB, rectAvel, dt);
                            // bottom right corner
                            check_collision_corner_bottom_right(rectA, rectB, rectAvel, dt);
                            // bottom left corner
                            check_collision_corner_bottom_left(rectA, rectB, rectAvel, dt);
                        }
                    }
                }
                rectAvel /= dt;
            }
        }
    }

  private:
    static bool check_collision_left_right(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                           yorcvs::Vec2<float> &rectAvel, float /*dt*/)
    {
        if (rectA.x + rectA.w <= rectB.x && rectA.x + rectA.w + (rectAvel.x) > rectB.x && rectA.y + rectA.h > rectB.y &&
            rectA.y - rectB.y < rectB.h)
        {
            rectAvel.x = (rectB.x - rectA.x - rectA.w);
            return true;
        }
        return false;
    }
    static bool check_collision_right_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                           yorcvs::Vec2<float> &rectAvel, float /*dt*/)
    {
        if (rectA.x >= rectB.x + rectB.w && rectA.x + (rectAvel.x) < rectB.x + rectB.w && rectA.y + rectA.h > rectB.y &&
            rectA.y - rectB.y < rectB.h)
        {
            rectAvel.x = (rectB.x + rectB.w - rectA.x);
            return true;
        }
        return false;
    }
    static bool check_collision_up_down(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                        yorcvs::Vec2<float> &rectAvel, float /*dt*/)
    {
        if (rectA.y <= rectB.y && rectA.y + rectA.h + rectAvel.y > rectB.y && rectA.x - rectB.x < rectB.w &&
            rectA.x + rectA.w > rectB.x)
        {
            rectAvel.y = (rectB.y - rectA.y - rectA.h);
            return true;
        }
        return false;
    }
    static bool check_collision_down_up(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                        yorcvs::Vec2<float> &rectAvel, float /*dt*/)
    {
        if ((rectA.y >=  rectB.y + rectB.h || std::fabs(rectA.y - rectB.y - rectB.h) <= fp_epsilon) && rectA.y + rectAvel.y <= rectB.y + rectB.h && rectA.x - rectB.x < rectB.w &&
            rectA.x + rectA.w > rectB.x)
        {
            rectAvel.y = (rectB.y + rectB.h - rectA.y);
            return true;
        }
        return false;
    }

    static bool check_collision_corner_top_right(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                 yorcvs::Vec2<float> &rectAvel, float /*dt*/)
    {
        if (rectA.x + (rectAvel.x) + rectA.w > rectB.x && rectA.x + (rectAvel.x) + rectA.w < (rectB.x + rectB.w) &&
            rectA.y + rectA.h + (rectAvel.y) > rectB.y && rectA.y + rectA.h + (rectAvel.y) < rectB.y + rectB.h &&
            rectA.x < rectB.x && rectA.y < rectB.y)
        {
            rectAvel.x = (rectB.x - (rectA.x + rectA.w));
            rectAvel.y = ((rectA.y + rectA.h) - rectB.y);
            return true;
        }
        return false;
    }
    static bool check_collision_corner_top_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                yorcvs::Vec2<float> &rectAvel, float /*dt*/)
    {
        if (rectA.x + (rectAvel.x) > rectB.x && rectA.x + (rectAvel.x) < rectB.x + rectB.w &&
            rectA.y + rectA.h + (rectAvel.y) > rectB.y && rectA.y + rectA.h + (rectAvel.y) < rectB.y + rectB.h &&
            rectA.x >= rectB.x && rectA.x + (rectAvel.x) + rectA.w > rectB.x + rectB.w && rectA.y < rectB.y)
        {
            rectAvel.x = ((rectB.x + rectB.w) - rectA.x);
            rectAvel.y = ((rectA.y + rectA.h) - rectB.y);
            return true;
        }
        return false;
    }
    static bool check_collision_corner_bottom_right(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                    yorcvs::Vec2<float> &rectAvel, float /*dt*/)
    {
        if (rectA.x + (rectAvel.x) < rectB.x && rectA.x + (rectAvel.x) + rectA.w > rectB.x &&
            rectA.x + (rectAvel.x) + rectA.w < rectB.x + rectB.w && rectA.x < rectB.x &&
            rectA.y + (rectAvel.y) > rectB.y && rectA.y + (rectAvel.y) < rectB.y + rectB.h &&
            rectA.y + rectA.h + (rectAvel.y) > rectB.y + rectB.h)
        {
            rectAvel.x = (rectB.x - (rectA.x + rectA.w));
            rectAvel.y = ((rectB.y + rectB.h) - rectA.y);
            return true;
        }
        return false;
    }
    static bool check_collision_corner_bottom_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                   yorcvs::Vec2<float> &rectAvel, float /*dt*/)
    {
        if (rectA.x + (rectAvel.x) > rectB.x && rectA.x + (rectAvel.x) < rectB.x + rectB.w &&
            rectA.x + (rectAvel.x) + rectA.w > rectB.x + rectB.w && rectA.y + (rectAvel.y) > rectB.y &&
            rectA.y + (rectAvel.y) < rectB.y + rectB.h && rectA.y + (rectAvel.y) + rectA.h > rectB.y + rectB.h)
        {
            rectAvel.x = ((rectB.x + rectB.w) - rectA.x);
            rectAvel.y = ((rectB.y + rectB.h) - rectA.y);
            return true;
        }
        return false;
    }

  public:
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
    static constexpr float fp_epsilon = .01f;
};
