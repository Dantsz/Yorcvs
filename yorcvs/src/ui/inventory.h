#pragma once
#include "../common/ecs.h"
#include "../engine/window/windowsdl2.h"
#include "../game/components.h"
#include "imgui.h"
namespace yorcvs::ui {

void show_entity_inventory(yorcvs::sdl2_window& window, [[maybe_unused]] yorcvs::ECS* appECS, [[maybe_unused]] size_t ID)
{
    constexpr size_t inventory_collumns = 4;
    constexpr ImVec2 icon_size = { 32, 32 };
    const auto inventory = appECS->get_component_checked<inventory_component>(ID);
    if (inventory.has_value()) {
        ImGui::Columns(inventory_collumns, 0, false);
        for (size_t i = 0; i < inventory.value().get().items.size(); i++) {
            if (inventory->get().items[i].has_value()) {
                const size_t item_id = inventory->get().items[i].value();
                ImGui::PushID(static_cast<int>(i));
                if (appECS->has_components<sprite_component>(item_id)) {
                    ImGui::Image(window.assetm->load_from_file(appECS->get_component<sprite_component>(item_id).texture_path).get(), icon_size);

                } else if (appECS->has_components<identification_component>(item_id)) {
                    ImGui::Text("%s", appECS->get_component<identification_component>(item_id).name.c_str());
                }
                ImGui::PopID();
                ImGui::NextColumn();
            }
        }
    }
    ImGui::Columns(1);
    ImGui::End();
}
}
