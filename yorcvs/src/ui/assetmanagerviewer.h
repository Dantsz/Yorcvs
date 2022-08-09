#pragma once
#include "../common/assetmanager.h"
#include "imgui.h"
#include <functional>
namespace yorcvs::ui {
template <typename asset_type>
inline void draw_asset_manager_tree(const yorcvs::AssetManager<asset_type>& asset_manager, std::function<void(const std::string&)> draw_asset_type, [[maybe_unused]] const std::string& widget_name = "Assets")
{
    for (const auto& [asset_name, asset] : asset_manager.get_assetmap()) {
        if (ImGui::TreeNode(asset_name.c_str())) {
            draw_asset_type(asset_name);
            ImGui::TreePop();
        }
    }
}

}
