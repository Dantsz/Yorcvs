#pragma once
#include "../common/ecs.h"
#include "../engine/window/windowsdl2.h"
#include "../game/components.h"
#include "imgui.h"
#include <functional>
namespace yorcvs::ui {

inline void show_entity_inventory(yorcvs::sdl2_window& window, [[maybe_unused]] yorcvs::ECS* appECS, [[maybe_unused]] size_t ID, std::function<void(size_t, size_t, size_t)> on_item_clicked)
{
    constexpr size_t inventory_collumns = 4;
    constexpr ImVec2 icon_size = { 32, 32 };
    const auto inventory = appECS->get_component_checked<inventory_component>(ID);
    if (inventory.has_value()) {
        ImGui::Columns(inventory_collumns, nullptr, false);
        for (size_t i = 0; i < inventory.value().get().items.size(); i++) {
            if (inventory->get().items[i].has_value()) {
                const size_t item_id = inventory->get().items[i].value();
                ImGui::PushID(static_cast<int>(i));
                if (appECS->has_components<sprite_component>(item_id)) {
                    if (ImGui::ImageButton(window.assetm->load_from_file(appECS->get_component<sprite_component>(item_id).texture_path).get(), icon_size)) {
                        ImGui::OpenPopup("Item");
                    }
                } else if (appECS->has_components<identification_component>(item_id)) {
                    ImGui::Text("%s", appECS->get_component<identification_component>(item_id).name.c_str());
                }
                if (ImGui::BeginPopup("Item")) {
                    on_item_clicked(inventory->get().items[i].value(), ID, i);
                    ImGui::EndPopup();
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
