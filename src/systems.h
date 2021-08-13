#pragma once
#include "common/ecs.h"
#include "components.h"
#include "windowSDL2.h"
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

                rectA.x = world->get_component<positionComponent>(IDA).position.x +
                          world->get_component<hitboxComponent>(IDA).hitbox.x;
                rectA.y = world->get_component<positionComponent>(IDA).position.y +
                          world->get_component<hitboxComponent>(IDA).hitbox.y;
                rectA.w = world->get_component<hitboxComponent>(IDA).hitbox.w;
                rectA.h = world->get_component<hitboxComponent>(IDA).hitbox.h;
                yorcvs::Vec2<float> &rectAvel = world->get_component<velocityComponent>(IDA).vel;
                for (const auto &IDB : entityList->entitiesID)
                {

                    rectB.x = world->get_component<positionComponent>(IDB).position.x;
                    rectB.y = world->get_component<positionComponent>(IDB).position.y;
                    rectB.w = world->get_component<hitboxComponent>(IDB).hitbox.w +
                              world->get_component<hitboxComponent>(IDB).hitbox.x;
                    rectB.h = world->get_component<hitboxComponent>(IDB).hitbox.h +
                              world->get_component<hitboxComponent>(IDB).hitbox.y;
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
        if (rectA.x + (rectAvel.x * elapsedTime) + rectA.w > rectB.x &&
            rectA.x + (rectAvel.x * elapsedTime) + rectA.w < (rectB.x + rectB.w) &&
            rectA.y + rectA.h + (rectAvel.y * elapsedTime) > rectB.y &&
            rectA.y + rectA.h + (rectAvel.y * elapsedTime) < rectB.y + rectB.h && rectA.x < rectB.x &&
            rectA.y < rectB.y)
        {

            rectAvel.x = (rectB.x - (rectA.x + rectA.w)) / elapsedTime;
            rectAvel.y = ((rectA.y + rectA.h) - rectB.y) / elapsedTime;
            return true;
        }
        return false;
    }
    static bool check_collision_corner_top_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                yorcvs::Vec2<float> &rectAvel, float elapsedTime)
    {
        if (rectA.x + (rectAvel.x * elapsedTime) > rectB.x &&
            rectA.x + (rectAvel.x * elapsedTime) < rectB.x + rectB.w &&
            rectA.y + rectA.h + (rectAvel.y * elapsedTime) > rectB.y &&
            rectA.y + rectA.h + (rectAvel.y * elapsedTime) < rectB.y + rectB.h && rectA.x >= rectB.x &&
            rectA.x + (rectAvel.x * elapsedTime) + rectA.w > rectB.x + rectB.w && rectA.y < rectB.y)
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
        if (rectA.x + (rectAvel.x * elapsedTime) < rectB.x &&
            rectA.x + (rectAvel.x * elapsedTime) + rectA.w > rectB.x &&
            rectA.x + (rectAvel.x * elapsedTime) + rectA.w < rectB.x + rectB.w && rectA.x < rectB.x &&
            rectA.y + (rectAvel.y * elapsedTime) > rectB.y &&
            rectA.y + (rectAvel.y * elapsedTime) < rectB.y + rectB.h &&
            rectA.y + rectA.h + (rectAvel.y * elapsedTime) > rectB.y + rectB.h)

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
        if (rectA.x + (rectAvel.x * elapsedTime) > rectB.x &&
            rectA.x + (rectAvel.x * elapsedTime) < rectB.x + rectB.w &&
            rectA.x + (rectAvel.x * elapsedTime) + rectA.w > rectB.x + rectB.w &&
            rectA.y + (rectAvel.y * elapsedTime) > rectB.y &&
            rectA.y + (rectAvel.y * elapsedTime) < rectB.y + rectB.h &&
            rectA.y + (rectAvel.y * elapsedTime) + rectA.h > rectB.y + rectB.h)
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
            world->get_component<velocityComponent>(ID).vel = {0, 0};
            if (std::abs(posOF.x) > std::numeric_limits<float>::epsilon())
            {
                world->get_component<velocityComponent>(ID).facing.x = (posOF.x > 0.0f);
            }
            if (std::abs(posOF.y) > std::numeric_limits<float>::epsilon())
            {
                world->get_component<velocityComponent>(ID).facing.y = (posOF.y > 0.0f);
            }
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};

class AnimationSystem
{
  public:
    AnimationSystem(yorcvs::ECS *parent)
    {
        world = parent;
        if (!world->is_component_registered<animationComponent>())
        {
            world->register_component<animationComponent>();
        }
        if (!world->is_component_registered<spriteComponent>())
        {
            world->register_component<spriteComponent>();
        }
        world->register_system<AnimationSystem>(*this);
        world->add_criteria_for_iteration<AnimationSystem, animationComponent, spriteComponent>();
    }

    void update(float elapsed) const
    {
        for (const auto &ID : entityList->entitiesID)
        {
            world->get_component<animationComponent>(ID).cur_elapsed += elapsed;
            if (world->get_component<animationComponent>(ID).cur_elapsed >
                world->get_component<animationComponent>(ID).speed)
            {
                world->get_component<animationComponent>(ID).cur_frame++;
                world->get_component<animationComponent>(ID).cur_elapsed = 0.0f;
                if (world->get_component<animationComponent>(ID).cur_frame <
                    world->get_component<animationComponent>(ID).frames)
                {
                    world->get_component<spriteComponent>(ID).srcRect.x =
                        world->get_component<spriteComponent>(ID).srcRect.x +
                        world->get_component<spriteComponent>(ID).srcRect.w;
                }
                else
                {
                    world->get_component<spriteComponent>(ID).srcRect.x -=
                        world->get_component<spriteComponent>(ID).srcRect.w *
                        (world->get_component<animationComponent>(ID).frames - 1);
                    world->get_component<animationComponent>(ID).cur_frame = 0;
                }
            }
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};

class HealthSystem
{
  public:
    HealthSystem(yorcvs::ECS *parent)
    {
        world = parent;
        if (!world->is_component_registered<healthComponent>())
        {
            world->register_component<healthComponent>();
        }
        world->register_system<HealthSystem>(*this);
        world->add_criteria_for_iteration<HealthSystem, healthComponent>();
    }
    void update(float dt)
    {
        cur_time += dt;
        if (cur_time >= update_time)
        {
            for (const auto &ID : entityList->entitiesID)
            {
                if (world->get_component<healthComponent>(ID).HP < 0.0f)
                {
                    world->get_component<healthComponent>(ID).is_dead = true;
                    continue;
                }
                world->get_component<healthComponent>(ID).HP += world->get_component<healthComponent>(ID).health_regen;
                if (world->get_component<healthComponent>(ID).HP > world->get_component<healthComponent>(ID).maxHP)
                {
                    world->get_component<healthComponent>(ID).HP = world->get_component<healthComponent>(ID).maxHP;
                }
            }
            cur_time = 0.0f;
        }
    }
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
    static constexpr float update_time = 1000.0f; // update once a second
    float cur_time = 0.0f;
};

class PlayerMovementControl
{
  public:
    PlayerMovementControl(yorcvs::ECS *parent, yorcvs::Window<yorcvs::graphics> *parent_window)
    {
        window = parent_window;
        world = parent;
        if (!world->is_component_registered<playerMovementControlledComponent>())
        {
            world->register_component<playerMovementControlledComponent>();
        }
        if (!world->is_component_registered<velocityComponent>())
        {
            world->register_component<velocityComponent>();
        }
        if (!world->is_component_registered<positionComponent>())
        {
            world->register_component<positionComponent>();
        }
        if (!world->is_component_registered<spriteComponent>())
        {
            world->register_component<spriteComponent>();
        }
        world->register_system<PlayerMovementControl>(*this);
        world->add_criteria_for_iteration<PlayerMovementControl, playerMovementControlledComponent, velocityComponent,
                                          positionComponent, spriteComponent>();
    }

    void updateControls()
    {

        w_pressed = window->is_key_pressed({SDL_SCANCODE_W});
        a_pressed = window->is_key_pressed({SDL_SCANCODE_A});
        s_pressed = window->is_key_pressed({SDL_SCANCODE_S});
        d_pressed = window->is_key_pressed({SDL_SCANCODE_D});

        for (const auto &ID : entityList->entitiesID)
        {
            window->set_drawing_offset(world->get_component<positionComponent>(ID).position -
                                       (window->get_window_size() - world->get_component<spriteComponent>(ID).size) /
                                           2);
            dir = yorcvs::Vec2<float>(static_cast<float>(d_pressed) + static_cast<float>(a_pressed) * -1.0f,
                                      static_cast<float>(w_pressed) * -1.0f + static_cast<float>(s_pressed));
            dir.normalize();

            world->get_component<velocityComponent>(ID).vel = dir;
        }
    }
    void updateAnimations() const
    {
        for (const auto &ID : entityList->entitiesID)
        {

            if (d_pressed)
            {
                world->get_component<spriteComponent>(ID).srcRect.y =
                    2 * world->get_component<spriteComponent>(ID).srcRect.h;
            }
            else if (a_pressed)
            {
                world->get_component<spriteComponent>(ID).srcRect.y =
                    3 * world->get_component<spriteComponent>(ID).srcRect.h;
            }
            else if (s_pressed || w_pressed)
            {
                world->get_component<spriteComponent>(ID).srcRect.y =
                    (static_cast<size_t>(!world->get_component<velocityComponent>(ID).facing.x) + 2) *
                    world->get_component<spriteComponent>(ID).srcRect.h;
            }
            else if (world->get_component<velocityComponent>(ID).facing.x)
            {
                world->get_component<spriteComponent>(ID).srcRect.y =
                    0 * world->get_component<spriteComponent>(ID).srcRect.h;
            }
            else
            {
                world->get_component<spriteComponent>(ID).srcRect.y =
                    1 * world->get_component<spriteComponent>(ID).srcRect.h;
            }
        }
    }
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
    yorcvs::Window<yorcvs::graphics> *window;
    yorcvs::Vec2<float> dir;
    bool w_pressed{};
    bool a_pressed{};
    bool s_pressed{};
    bool d_pressed{};
};

class SpriteSystem
{
  public:
    SpriteSystem(yorcvs::ECS *parent, yorcvs::Window<yorcvs::graphics> *parentWindow)
    {
        world = parent;
        window = parentWindow;
        if (!world->is_component_registered<positionComponent>())
        {
            world->register_component<positionComponent>();
        }
        if (!world->is_component_registered<spriteComponent>())
        {
            world->register_component<spriteComponent>();
        }
        world->register_system<SpriteSystem>(*this);
        world->add_criteria_for_iteration<SpriteSystem, positionComponent, spriteComponent>();
    }
    void renderSprites() const
    {
        std::sort(entityList->entitiesID.begin(), entityList->entitiesID.end(), [&](size_t ID1, size_t ID2) {
            return (world->get_component<spriteComponent>(ID1).offset.y +
                    world->get_component<positionComponent>(ID1).position.y) <
                   (world->get_component<spriteComponent>(ID2).offset.y +
                    world->get_component<positionComponent>(ID2).position.y);
        });
        for (const auto &ID : entityList->entitiesID)
        {
            window->draw_texture(
                world->get_component<spriteComponent>(ID).texture,
                world->get_component<spriteComponent>(ID).offset + world->get_component<positionComponent>(ID).position,
                world->get_component<spriteComponent>(ID).size, world->get_component<spriteComponent>(ID).srcRect, 0.0);
        }
        std::sort(entityList->entitiesID.begin(), entityList->entitiesID.end(),
                  [&](size_t ID1, size_t ID2) { return ID1 < ID2; });
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
    yorcvs::Window<yorcvs::graphics> *window;
};
