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
        world->add_criteria_for_iteration<PlayerMovementControl, player_movement_controlled_component, velocity_component,
            position_component, sprite_component>();
    }

    void updateControls(const yorcvs::vec2<float>& render_size, float dt)
    {
        const bool w_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_W);
        const bool a_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_A);
        const bool s_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_S);
        const bool d_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_D);
        const bool q_pressed = window->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_Q);
        if (entityList->empty()) {
            return;
        }
        const size_t ID = entityList->at(entityList->size() - 1);
        float& cur_time = world->get_component<player_movement_controlled_component>(ID).update_time;
        cur_time += dt;
        const bool update = cur_time >= update_time;
        const bool has_sprint_stamina = world->has_components<stamina_component, stamina_stats_component>(ID);
        window->set_drawing_offset(world->get_component<position_component>(ID).position + dir - (render_size - world->get_component<sprite_component>(ID).size) / 2);
        if (!controls_enable) {
            return;
        }

        dir = compute_movement_direction(static_cast<float>(d_pressed), static_cast<float>(a_pressed), static_cast<float>(w_pressed), static_cast<float>(s_pressed));

        if (q_pressed && (!has_sprint_stamina || (has_sprint_stamina && world->get_component<stamina_component>(ID).stamina - world->get_component<stamina_stats_component>(ID).stamina_regen > 0))) {
            dir *= PlayerMovementControl::sprint_multiplier;
            if (update) {
                world->get_component<stamina_component>(ID).stamina -= 2 * world->get_component<stamina_stats_component>(ID).stamina_regen;
            }
        }
        world->get_component<velocity_component>(ID).vel = dir;

        auto& player_state = world->get_component<player_movement_controlled_component>(ID);
        if (update) {
            if (player_state.current_state == player_movement_controlled_component::PLAYER_ATTACK_L) {
                player_state.current_state = player_movement_controlled_component::PLAYER_IDLE_L;
            }
            if (player_state.current_state == player_movement_controlled_component::PLAYER_ATTACK_R) {
                player_state.current_state = player_movement_controlled_component::PLAYER_IDLE_R;
            }
        } else if (a_pressed) {
            player_state.current_state = player_movement_controlled_component::PLAYER_MOVING_L;
        } else if (d_pressed || s_pressed || w_pressed) {
            player_state.current_state = player_movement_controlled_component::PLAYER_MOVING_R;
        } else if (player_state.current_state == player_movement_controlled_component::PLAYER_MOVING_R) {
            player_state.current_state = player_movement_controlled_component::PLAYER_IDLE_R;
        } else if (player_state.current_state == player_movement_controlled_component::PLAYER_MOVING_L) {
            player_state.current_state = player_movement_controlled_component::PLAYER_IDLE_L;
        }

        const char* new_animation = select_animation(player_state);
        AnimationSystem::set_animation_global(world, ID, new_animation);
        if (update) {
            cur_time = 0.0f;
        }
    }
    static constexpr float sprint_multiplier = 1.5f;
    static constexpr float update_time = 500.0f;
    static constexpr float player_default_speed = 0.033f;
    std::shared_ptr<yorcvs::entity_system_list> entityList;
    bool controls_enable = true;

private:
    static constexpr yorcvs::vec2<float> compute_movement_direction(float move_right, float move_left, float move_up, float move_down)
    {
        // normalizing means moving at 1000 units per second
        return yorcvs::vec2<float>(move_right + move_left * -1.0f, move_up * -1.0f + move_down).normalize() * player_default_speed; // move 30 units per second
    }
    static const char* select_animation(player_movement_controlled_component& player_state)
    {
        switch (player_state.current_state) {
        case player_movement_controlled_component::PLAYER_IDLE_R:
            return "idleR";
        case player_movement_controlled_component::PLAYER_IDLE_L:
            return "idleL";
        case player_movement_controlled_component::PLAYER_MOVING_R:
            return "walkingR";
        case player_movement_controlled_component::PLAYER_MOVING_L:
            return "walkingL";
        case player_movement_controlled_component::PLAYER_USE_R:
            return "hold_R";
        case player_movement_controlled_component::PLAYER_USE_L:
            return "hold_L";
        case player_movement_controlled_component::PLAYER_ATTACK_R:
            return "attackR";
        case player_movement_controlled_component::PLAYER_ATTACK_L:
            return "attackL";
        }
        return "";
    }

    yorcvs::ECS* world;
    yorcvs::sdl2_window* window;
    yorcvs::vec2<float> dir;
};
