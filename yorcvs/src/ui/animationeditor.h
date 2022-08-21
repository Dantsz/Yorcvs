#pragma once
#include "../common/ecs.h"
#include "../game/components.h"
#include "../game/systems/animation.h"
#include "imgui.h"
namespace yorcvs::ui {

void show_current_animator_selector([[maybe_unused]] yorcvs::ECS* appECS, [[maybe_unused]] size_t ID)
{
    const animation_component& anim_comp = appECS->get_component<animation_component>(ID);
    static std::string current_item {};
    if (ImGui::BeginCombo("Animation", current_item.c_str())) {
        for (const auto& [animation_name, animation] : anim_comp.animation_name_to_start_frame_index) {
            bool is_selected = (current_item == animation_name);
            if (ImGui::Selectable(animation_name.c_str(), is_selected)) {
                current_item = animation_name;
                animation_system::set_animation_global(appECS, ID, animation_name);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::End();
}

}
