#pragma once
#include "../common/assetmanager.h"
#include "imgui.h"
#include <functional>
namespace yorcvs::ui {
template <typename asset_type>
inline void draw_asset_manager_tree(const yorcvs::AssetManager<asset_type>& asset_manager, std::function<void(const asset_type&)> draw_asset_type, const std::string& widget_name = "Assets")
{
    for (const auto& [asset_name, asset] : asset_manager) {
        if (ImGui::TreeNode(asset_name.c_str())) {

            ImGui::TreePop();
        }
    }
}

}
