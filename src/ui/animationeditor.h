#pragma once
#include "../common/ecs.h"
#include "../game/components.h"
#include "../game/systems.h"
namespace yorcvs::ui {

void show_current_animator_selector(yorcvs::ECS* appECS, size_t ID)
{
    const animationComponent& anim_comp = appECS->get_component<animationComponent>(ID);
    static std::string current_item {};
    //    if (ImGui::BeginCombo("Animation", anim_comp.cur_animation.c_str())) {
    //        for (const auto& [animation_name, animation] : anim_comp.animations) {
    //            bool is_selected = (current_item == animation_name);
    //            if (ImGui::Selectable(animation_name.c_str(), is_selected)) {
    //                current_item = animation_name;
    //                AnimationSystem::set_animation(appECS, ID, animation_name);
    //            }
    //            if (is_selected) {
    //                ImGui::SetItemDefaultFocus();
    //            }
    //        }
    //        ImGui::EndCombo();
    //    }
    ImGui::End();
}

}
