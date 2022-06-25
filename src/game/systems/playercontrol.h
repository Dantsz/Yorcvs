#pragma once
#include "../../common/ecs.h"
#include "../components.h"
#include "../../engine/window/windowsdl2.h"
/**
 * @brief Handles player input
 *
 */
class PlayerMovementControl
{
  public:
    static constexpr float sprint_multiplier = 1.5f;

    PlayerMovementControl(yorcvs::ECS *parent, yorcvs::sdl2_window *parent_window)
        : world(parent), window(parent_window)
    {
        world->register_system<PlayerMovementControl>(*this);
        world->add_criteria_for_iteration<PlayerMovementControl, playerMovementControlledComponent, velocityComponent,
                                          positionComponent, spriteComponent>();
    }

    void updateControls(const yorcvs::Vec2<float> &render_size, float dt)
    {
        if (controls_enable)
        {
            w_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_W);
            a_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_A);
            s_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_S);
            d_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_D);
            q_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_Q);
            cur_time += dt;
            for (const auto &ID : entityList->entitiesID)
            {
                dir = yorcvs::Vec2<float>(static_cast<float>(d_pressed) + static_cast<float>(a_pressed) * -1.0f,
                                          static_cast<float>(w_pressed) * -1.0f + static_cast<float>(s_pressed));
                dir.normalize();             // now moving at 1000 units per second
                dir *= player_default_speed; // move 30 units per second
                if (q_pressed)
                {
                    if ((!world->has_components<staminaComponent>(ID)))
                    {
                        dir *= PlayerMovementControl::sprint_multiplier;
                    }
                    else if ((world->has_components<staminaComponent>(ID) &&
                              world->get_component<staminaComponent>(ID).stamina -
                                      world->get_component<staminaComponent>(ID).stamina_regen >
                                  0))
                    {
                        dir *= PlayerMovementControl::sprint_multiplier;
                        if (cur_time >= update_time)
                        {
                            world->get_component<staminaComponent>(ID).stamina -=
                                2 * world->get_component<staminaComponent>(ID).stamina_regen;
                        }
                    }
                }
                if (cur_time >= update_time)
                {
                    cur_time = 0.0f;
                }
                world->get_component<velocityComponent>(ID).vel = dir;
                window->set_drawing_offset(world->get_component<positionComponent>(ID).position + dir -
                                           (render_size - world->get_component<spriteComponent>(ID).size) / 2);
                if (a_pressed)
                {
                    AnimationSystem::set_animation(world, ID, "walkingL");
                }
                else if (d_pressed)
                {
                    AnimationSystem::set_animation(world, ID, "walkingR");
                }
                else if (s_pressed || w_pressed)
                {
                    AnimationSystem::set_animation(world, ID, "walkingR");
                }
                else if (world->get_component<velocityComponent>(ID).facing.x)
                {
                    AnimationSystem::set_animation(world, ID, "idleL");
                }
                else
                {
                    AnimationSystem::set_animation(world, ID, "idleR");
                }
            }
        }
    }

    static constexpr float update_time = 1000.0f;
    static constexpr float player_default_speed = 0.033f;
    float cur_time{};

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
    yorcvs::sdl2_window* window;
    yorcvs::Vec2<float> dir;

    bool controls_enable = true;
    bool w_pressed{};
    bool a_pressed{};
    bool s_pressed{};
    bool d_pressed{};
    bool q_pressed{};
};
