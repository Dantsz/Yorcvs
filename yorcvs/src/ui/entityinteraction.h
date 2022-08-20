#pragma once
#include "../common/ecs.h"
#include "../engine/window/eventhandler.h"
#include "../engine/window/window.h"
#include "../game/systems/collision.h"
#include "../game/systems/combat.h"
#include "../game/systems/playercontrol.h"
#include "imgui.h"
namespace yorcvs::ui {
static inline void show_entity_interaction_window(yorcvs::ECS* world, CombatSystem* combat_system, size_t sender, size_t target)
{
    if (ImGui::Button("go to") && world->has_components<position_component>(target) && world->has_components<position_component>(target)) {
        world->get_component<position_component>(sender).position = world->get_component<position_component>(target).position;
    }
    if (ImGui::Button("teleport here") && world->has_components<position_component>(target) && world->has_components<position_component>(target)) {
        world->get_component<position_component>(target).position = world->get_component<position_component>(sender).position;
    }
    if (world->has_components<offensive_stats_component>(sender) && world->has_components<health_component>(target) && world->has_components<defensive_stats_component>(target) && ImGui::Button("attack")) {
        const auto damage = combat_system->attack(sender, target);
        yorcvs::log(std::to_string(sender) + " dealt " + std::to_string(damage) + " to " + std::to_string(target));
        const auto sender_state = world->get_component_checked<player_movement_controlled_component>(sender);
        const auto sender_vel = world->get_component_checked<velocity_component>(sender);
        if (sender_state.has_value() && sender_vel.has_value()) {
            sender_state->get().current_state = (!sender_vel->get().facing.x) ? player_movement_controlled_component::PLAYER_ATTACK_R : player_movement_controlled_component::PLAYER_ATTACK_L;
            sender_state->get().update_time = 0.0f;
        }
    }
}
}
template <typename eventhandler_impl, typename window_impl>
class EntityInteractionWidget {
public:
    EntityInteractionWidget(yorcvs::event_handler<eventhandler_impl>& event_handler, yorcvs::window<window_impl>& window, yorcvs::ECS& world, CollisionSystem& collision_system, CombatSystem& combat_system, PlayerMovementControl& player_move_system)
        : event_handler(&event_handler)
        , window(&window)
        , world(&world)
        , collision_system(&collision_system)
        , combat_system(&combat_system)
        , player_move_system(&player_move_system)
        , entity_is_clicked_callback(event_handler.add_callback_on_event(yorcvs::Events::Type::MOUSE_CLICKED,
              [widget = this](const yorcvs::event&) {
                  yorcvs::Vec2<float> old_rs = widget->window->get_render_scale();
                  widget->window->set_render_scale(widget->window->get_window_size() / widget->render_dimensions);
                  bool clicked_any_entity = false;
                  const auto pointer_position = widget->event_handler->get_pointer_position();
                  for (const auto& ID : *(widget->collision_system->entityList)) {
                      yorcvs::Rect<float> rect {};
                      rect.x = widget->world->template get_component<position_component>(ID).position.x + widget->world->template get_component<hitbox_component>(ID).hitbox.x;
                      rect.y = widget->world->template get_component<position_component>(ID).position.y + widget->world->template get_component<hitbox_component>(ID).hitbox.y;
                      rect.w = widget->world->template get_component<hitbox_component>(ID).hitbox.w;
                      rect.h = widget->world->template get_component<hitbox_component>(ID).hitbox.h;
                      if (rect.contains(pointer_position / widget->window->get_render_scale() + widget->window->get_drawing_offset())) {
                          widget->targetID = ID;
                          widget->target_window_position = pointer_position;
                          widget->select_target_opened = true;
                          clicked_any_entity = true;
                      }
                  }
                  if (!clicked_any_entity) {
                      widget->targetID.reset();
                  }
                  widget->window->set_render_scale(old_rs);
              }))
    {
    }
    ~EntityInteractionWidget()
    {
        event_handler->unregister_callback(entity_is_clicked_callback);
    }
    void render([[maybe_unused]] yorcvs::Vec2<float>& render_dim)
    {
        if (player_move_system->entityList->empty()) {
            return;
        }
        render_dimensions = render_dim;
        if (targetID.has_value() && world->is_valid_entity(targetID.value()) && select_target_opened) {
            ImGui::SetNextWindowPos({ target_window_position.x, target_window_position.y });
            ImGui::SetNextWindowSize({ target_window_size.x, target_window_size.y });
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
            ImGui::SetNextWindowBgAlpha(target_window_alpha);
            ImGui::Begin("Target", &select_target_opened, window_flags);
            if (world->has_components<identification_component>(targetID.value())) {
                ImGui::Text("Name: %s", world->get_component<identification_component>(targetID.value()).name.c_str());
            }
            yorcvs::ui::show_entity_interaction_window(world, combat_system, get_last_player_id().value(), targetID.value());
            ImGui::End();
        } else {
            targetID.reset();
        }
    }
    [[nodiscard]] std::optional<size_t> get_first_player_id() const
    {
        if (player_move_system->entityList->empty()) {
            return {};
        }
        return (*player_move_system->entityList)[0];
    }
    [[nodiscard]] std::optional<size_t> get_last_player_id() const
    {
        if (player_move_system->entityList->empty()) {
            return {};
        }
        return (*player_move_system->entityList)[player_move_system->entityList->size() - 1];
    }

private:
    yorcvs::event_handler<eventhandler_impl>* const event_handler;
    yorcvs::window<window_impl>* const window;
    yorcvs::ECS* const world;
    CollisionSystem* collision_system;
    CombatSystem* combat_system;
    PlayerMovementControl* player_move_system;
    const size_t entity_is_clicked_callback;

    bool select_target_opened = false;
    yorcvs::Vec2<float> target_window_position {};
    yorcvs::Vec2<float> render_dimensions {};
    std::optional<size_t> targetID = 0;
    static constexpr yorcvs::Vec2<float> target_window_size { 150, 100 };
    static constexpr float target_window_alpha = 0.5f;
};
