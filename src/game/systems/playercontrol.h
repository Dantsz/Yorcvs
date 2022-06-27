#pragma once
#include "../../common/ecs.h"
#include "../../engine/window/windowsdl2.h"
#include "../components.h"
#include "animation.h"
/**
 * @brief Handles player input
 *
 */
class PlayerMovementControl {
public:
    PlayerMovementControl(yorcvs::ECS* parent, yorcvs::sdl2_window* parent_window)
        : world(parent)
        , window(parent_window)
    {
        world->register_system<PlayerMovementControl>(*this);
        world->add_criteria_for_iteration<PlayerMovementControl, playerMovementControlledComponent, velocityComponent,
            positionComponent, spriteComponent>();
    }

    void updateControls(const yorcvs::Vec2<float>& render_size, float dt)
    {
        cur_time += dt;
        for (const auto& ID : entityList->entitiesID) {
            if (controls_enable) {
                const bool w_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_W);
                const bool a_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_A);
                const bool s_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_S);
                const bool d_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_D);
                const bool q_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_Q);
                dir = compute_movement_direction(static_cast<float>(d_pressed), static_cast<float>(a_pressed), static_cast<float>(w_pressed), static_cast<float>(s_pressed));
                if (q_pressed) {
                    if ((!world->has_components<staminaComponent>(ID))) {
                        dir *= PlayerMovementControl::sprint_multiplier;
                    } else if (world->get_component<staminaComponent>(ID).stamina - world->get_component<staminaComponent>(ID).stamina_regen > 0) {
                        dir *= PlayerMovementControl::sprint_multiplier;
                        if (cur_time >= update_time) {
                            world->get_component<staminaComponent>(ID).stamina -= 2 * world->get_component<staminaComponent>(ID).stamina_regen;
                        }
                    }
                }
                if (cur_time >= update_time) {
                    cur_time = 0.0f;
                }
                world->get_component<velocityComponent>(ID).vel = dir;
                AnimationSystem::set_animation(world, ID, select_facing_animation(a_pressed, d_pressed, w_pressed, s_pressed, world->get_component<velocityComponent>(ID).facing.x));
            }
            window->set_drawing_offset(world->get_component<positionComponent>(ID).position + dir - (render_size - world->get_component<spriteComponent>(ID).size) / 2);
        }
    }
    static constexpr float sprint_multiplier = 1.5f;
    static constexpr float update_time = 1000.0f;
    static constexpr float player_default_speed = 0.033f;
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    bool controls_enable = true;

private:
    static constexpr yorcvs::Vec2<float> compute_movement_direction(float move_right, float move_left, float move_up, float move_down)
    {
        // normalizing means moving at 1000 units per second
        return yorcvs::Vec2<float>(move_right + move_left * -1.0f, move_up * -1.0f + move_down).normalize() * player_default_speed; // move 30 units per second
    }
    static const char* select_facing_animation(bool a_pressed, bool d_pressed, bool w_pressed, bool s_pressed, bool facing)
    {
        if (a_pressed) {
            return "walkingL";
        }
        if (d_pressed || s_pressed || w_pressed) {
            return "walkingR";
        }
        if (facing) {
            return "idleL";
        }
        return "idleR";
    }
    float cur_time {};
    yorcvs::ECS* world;
    yorcvs::sdl2_window* window;
    yorcvs::Vec2<float> dir;
};
