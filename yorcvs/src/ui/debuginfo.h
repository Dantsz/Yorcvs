#pragma once
#include "../common/ecs.h"
#include "../common/types.h"
#include "../engine/map.h"
#include "../engine/window/windowsdl2.h"
#include "../game/components.h"
#include "animationeditor.h"
#include "assetmanagerviewer.h"
#include "entityinteraction.h"
#include "imgui.h"
#include "inventory.h"
#include "misc/cpp/imgui_stdlib.h"
#include <optional>
namespace yorcvs {
class application;
class debug_info {
public:
    debug_info() = delete;
    debug_info(yorcvs::application* parentAPP, yorcvs::sdl2_window* parentW, yorcvs::map* map_object, player_movement_control* pms, collision_system* cols,
        health_system* healthS, combat_system* combat_sys, sol::state* lua)
        : parentWindow(parentW)
        , appECS(map_object->ecs)
        , map(map_object)
        , lua_state(lua)
        , parent_app(parentAPP)
        , player_move_sys(pms)
        , colission_sys(cols)
        , combat_sys(combat_sys)
    {
        attach(parentW, map_object, pms, cols, healthS, lua);
    }
    ~debug_info() = default;

    debug_info(const debug_info& other) = delete;
    debug_info(debug_info&& other) = delete;
    debug_info operator=(const debug_info& other) = delete;
    debug_info operator=(debug_info&& other) = delete;

    void update(const float elapsed, yorcvs::vec2<float>& render_dimensions)
    {
        time_accumulator += elapsed;
        if (time_accumulator >= ui_controls_update_time) {
            if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_LCTRL)) {
                if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_E)) {
                    debug_window_opened = !debug_window_opened;
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(Events::Key::YORCVS_KEY_TILDE)) {
                    player_move_sys->controls_enable = !player_move_sys->controls_enable;
                    console_opened = !console_opened;
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_I)) {
                    render_dimensions -= render_dimensions * zoom_power;
                    time_accumulator = 0;
                }

                if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_K)) {
                    render_dimensions += render_dimensions * zoom_power;
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_C)) {
                    yorcvs::log("Saving player...");
                    std::ofstream out("assets/testPlayer.json");
                    out << map->save_entity((*player_move_sys->entityList)[0]);
                    yorcvs::log("Done.");
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(yorcvs::Events::YORCVS_KEY_R)) {
                    reset();
                }
            }
        }
        if (!player_move_sys->entityList->empty()) {
            (*lua_state)["playerID"] = (*player_move_sys->entityList)[0];
        }
    }

    void render_hitboxes(yorcvs::sdl2_window& window, const yorcvs::vec2<float>& render_dimensions, const uint8_t r,
        const uint8_t g, const uint8_t b, const uint8_t a)
    {
        yorcvs::vec2<float> old_rs = window.get_render_scale();
        window.set_render_scale(window.get_window_size() / render_dimensions);
        yorcvs::rect<float> rect {};
        for (const auto& ID : *colission_sys->entityList) {
            rect.x = appECS->get_component<position_component>(ID).position.x + appECS->get_component<hitbox_component>(ID).hitbox.x;
            rect.y = appECS->get_component<position_component>(ID).position.y + appECS->get_component<hitbox_component>(ID).hitbox.y;
            rect.w = appECS->get_component<hitbox_component>(ID).hitbox.w;
            rect.h = appECS->get_component<hitbox_component>(ID).hitbox.h;
            window.draw_rect(rect, r, g, b, a);
            draw_entity_health_bar(window, ID, rect);
            draw_entity_stamina_bar(window, ID, rect);
        }

        window.set_render_scale(old_rs);
    }

    void render(yorcvs::vec2<float>& render_dimensions)
    {
        if (debug_window_opened) {
            show_debug_window(render_dimensions);
        }
        if (console_opened) {
            show_console_window();
            ImGui::ShowDemoWindow();
            ImGui::Begin("Monitor");
            show_entities_table();
            if (ImGui::CollapsingHeader("Loaded Textures")) {
                ui::draw_asset_manager_tree<SDL_Texture>(
                    *parentWindow->assetm, [&](const std::string& tex_name) {
                        constexpr ImVec2 texture_size = { 1000, 1000 };
                        ImGui::Image(parentWindow->assetm->load_from_file(tex_name).get(), texture_size);
                    },
                    "Textures");
            }

            ImGui::End();
        }
    }

    void attach(yorcvs::sdl2_window* parentW, yorcvs::map* map_object, player_movement_control* pms,
        collision_system* cols, health_system* healthS, sol::state* lua)
    {
        lua_state = lua;
        attach_lua();
        parentWindow = parentW;
        map = map_object;
        appECS = map->ecs;
        player_move_sys = pms;
        colission_sys = cols;
        health_sys = healthS;
    }
    void attach_lua()
    {
        lua_state->set_function("internal_log", &debug_info::add_log, this);
        lua_state->script("function log(message) internal_log(tostring(message)) end");
        (*lua_state)["print"] = (*lua_state)["log"];
    }
    void end_frame()
    {
    }
    [[nodiscard]] bool is_debug_window_open() const
    {
        return debug_window_opened;
    }

private:
    void show_debug_window(yorcvs::vec2<float>& render_dimensions)
    {
        render_hitboxes(*parentWindow, render_dimensions, hitbox_color[0], hitbox_color[1], hitbox_color[2],
            hitbox_color[3]);

        if (!player_move_sys->entityList->empty()) {
            const size_t ID = (*player_move_sys->entityList)[0];
            ImGui::Begin("Player");
            show_entity_stats(ID, "Player : ");
            if (appECS->has_components<animation_component>(ID)) {
                ui::show_current_animator_selector(appECS, ID);
            }
            ImGui::Begin("Inventory");
            yorcvs::ui::show_entity_inventory(*parentWindow, appECS, ID, [&](size_t item_id) {
                if (appECS->has_components<identification_component>(item_id)) {
                    ImGui::Text(appECS->get_component<identification_component>(item_id).name.c_str());
                }
                if (ImGui::Button("Drop")) {
                    // TODO:make use of a function from an item_system in order to remove the item from holder inventory and give it components necessary to be put on the ground
                }
            });
        }
    }
    void show_entity_stats(size_t ID, [[maybe_unused]] std::string pre_name = "Entity : ")
    {
        if (appECS->has_components<identification_component>(ID)) {
            pre_name += appECS->get_component<identification_component>(ID).name + " (" + std::to_string(ID) + ")";
        }
        ImGui::Text("%s", pre_name.c_str());
        if (appECS->has_components<sprite_component>(ID)) {
            static constexpr float size_multiplier = 4.0f;
            const sprite_component& comp = appECS->get_component<sprite_component>(ID);

            int texture_size_x {};
            int texture_size_y {};

            SDL_QueryTexture(parentWindow->assetm->load_from_file(comp.texture_path).get(), nullptr, nullptr,
                &texture_size_x, &texture_size_y);

            const yorcvs::vec2<float> top_corner = {
                static_cast<float>(comp.src_rect.x) / static_cast<float>(texture_size_x),
                static_cast<float>(comp.src_rect.y) / static_cast<float>(texture_size_y)
            };
            const yorcvs::vec2<float> bottom_corner = {
                static_cast<float>(comp.src_rect.x + comp.src_rect.w) / static_cast<float>(texture_size_x),
                static_cast<float>(comp.src_rect.y + comp.src_rect.h) / static_cast<float>(texture_size_y)
            };
            ImGui::Image(parentWindow->assetm->load_from_file(comp.texture_path).get(),
                { size_multiplier * comp.size.x, size_multiplier * comp.size.y }, { top_corner.x, top_corner.y },
                { bottom_corner.x, bottom_corner.y });
        }
        if (appECS->has_components<position_component>(ID)) {
            ImGui::Text("Position: (%f,%f)", appECS->get_component<position_component>(ID).position.x,
                appECS->get_component<position_component>(ID).position.y);
        }
        if (appECS->has_components<velocity_component>(ID)) {
            ImGui::Text("Velocity: (%f,%f)", appECS->get_component<velocity_component>(ID).vel.x,
                appECS->get_component<velocity_component>(ID).vel.y);
        }
        if (appECS->has_components<health_component, health_stats_component>(ID)) {
            auto& playerHealthC = appECS->get_component<health_component>(ID);
            auto& playerHealthStatsC = appECS->get_component<health_stats_component>(ID);
            ImGui::Text("Health: (%f/%f)", playerHealthC.HP, playerHealthStatsC.max_HP);
        }
        if (appECS->has_components<stamina_component, stamina_stats_component>(ID)) {
            auto& playerStaminaC = appECS->get_component<stamina_component>(ID);
            auto& playerStamStatC = appECS->get_component<stamina_stats_component>(ID);
            ImGui::Text("Stamina: (%f/%f)", playerStaminaC.stamina, playerStamStatC.max_stamina);
        }

        if (appECS->has_components<offensive_stats_component>(ID)) {
            auto& offStatsC = appECS->get_component<offensive_stats_component>(ID);
            ImGui::Text("Strength : (%f)", offStatsC.strength);
            ImGui::Text("Agility : (%f)", offStatsC.agility);
            ImGui::Text("Dexterity : (%f)", offStatsC.dexterity);
            ImGui::Text("Piercing : (%f)", offStatsC.piercing);
            ImGui::Text("Intellect : (%f)", offStatsC.intellect);
        }
        if (appECS->has_components<defensive_stats_component>(ID)) {
            auto& defstats = appECS->get_component<defensive_stats_component>(ID);
            ImGui::Text("Defense : (%f)", defstats.defense);
            ImGui::Text("Block : (%f)", defstats.block);
            ImGui::Text("Dodge : (%f)", defstats.dodge);
            ImGui::Text("Spirit : (%f)", defstats.spirit);
        }
    }
    void show_console_window()
    {
        ImGui::Begin("Console");
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -3 * footer_height_to_reserve), false,
            ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& item : console_logs) {
            ImGui::TextUnformatted(item.c_str());
        }
        ImGui::EndChild();
        ImGui::Separator();
        bool reclaim_focus = false;
        if (ImGui::InputText("##", &console_text, input_text_flags, &debug_info::text_edit_callback_stub, (void*)this) && !console_text.empty()) {
            console_logs.push_back(console_text);
            history_pos = -1;
            console_previous_commands.push_back(console_text);
            auto rez = lua_state->safe_script(console_text,
                [](lua_State*, sol::protected_function_result pfr) { return pfr; });
            if (!rez.valid()) {
                sol::error err = rez;
                std::string text = err.what();
                console_logs.push_back(std::move(text));
            }
            console_text = "";
            reclaim_focus = true;
        }
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus) {
            ImGui::SetKeyboardFocusHere(-1);
        }
        if (ImGui::SmallButton("Clear")) {
            clear_logs();
        }
        ImGui::End();
    }
    void show_entities_table()
    {
        const int collumn_count = 5;
        const ImGuiTableFlags flags1 = ImGuiTableFlags_BordersV | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Resizable;
        if (!(ImGui::CollapsingHeader("Entities") && ImGui::BeginTable("table1", collumn_count, flags1))) {
            return;
        }
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Signature");
        ImGui::TableSetupColumn("Position");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < appECS->get_entity_list_size(); i++) {
            if (!appECS->is_valid_entity(i)) {
                continue;
            }
            ImGui::PushID(static_cast<int>(i));
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            if (ImGui::Selectable("", false)) {
                ImGui::OpenPopup("Entity");
            }
            if (ImGui::BeginPopup("Entity")) {
                ImGui::Text("%s", std::to_string(i).c_str());
                show_entity_stats(i);
                yorcvs::ui::show_entity_interaction_window(appECS, combat_sys, get_first_player_id(), i);
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::Text("%zu", i);

            ImGui::TableSetColumnIndex(1);
            if (appECS->has_components<identification_component>(i)) {
                ImGui::Text("%s", appECS->get_component<identification_component>(i).name.c_str());
            } else {
                ImGui::Text("%s", "Unknown");
            }
            ImGui::TableSetColumnIndex(2);
            std::string signature {};
            const size_t signatureSize = appECS->get_entity_signature(i).size();
            signature.resize(signatureSize);
            for (size_t j = 0; j < signatureSize; j++) {
                signature[j] = appECS->get_entity_signature(i)[j] ? '1' : '0';
            }
            ImGui::Text("%s", signature.c_str());

            ImGui::TableSetColumnIndex(3);
            if (appECS->has_components<position_component>(i)) {
                const auto& position = appECS->get_component<position_component>(i).position;
                ImGui::Text("%f/%f", position.x, position.y);
            } else {
                ImGui::Text("(-/-)");
            }

            ImGui::TableSetColumnIndex(4);

            if (ImGui::SmallButton("Delete")) {
                yorcvs::log("Deleting entity with index : " + std::to_string(i));
                appECS->destroy_entity(i);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Duplicate")) {
                size_t ID = appECS->create_entity_ID();
                appECS->copy_components_to_from_entity(ID, i);
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    void reset()
    {
    }

    void add_log(const std::string& message)
    {
        console_logs.push_back(message);
    }
    void clear_logs()
    {
        console_logs.clear();
        console_previous_commands.clear();
        history_pos = -1;
    }
    static int text_edit_callback_stub(ImGuiInputTextCallbackData* data)
    {
        auto* console = static_cast<debug_info*>(data->UserData);
        return console->text_edit_callback(data);
    }

    int text_edit_callback(ImGuiInputTextCallbackData* data)
    {
        // AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag) {
        case ImGuiInputTextFlags_CallbackHistory: {
            // Example of HISTORY
            const int prev_history_pos = history_pos;
            if (data->EventKey == ImGuiKey_UpArrow) {
                if (history_pos == -1) {
                    history_pos = static_cast<int>(console_previous_commands.size()) - 1;
                } else if (history_pos > 0) {
                    history_pos--;
                }
            } else if (data->EventKey == ImGuiKey_DownArrow) {
                if (history_pos != -1) {
                    if (++history_pos >= static_cast<int>(console_previous_commands.size())) {
                        history_pos = -1;
                    }
                }
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != history_pos) {
                const char* history_str = (history_pos >= 0) ? console_previous_commands[history_pos].c_str() : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
        }
        return 1;
    }

    size_t get_first_player_id()
    {
        if (!player_move_sys->entityList->empty()) {
            return (*player_move_sys->entityList)[0];
        }
        const size_t invalidID = appECS->create_entity_ID();
        appECS->destroy_entity(invalidID);
        return invalidID;
    }
    void draw_entity_health_bar(yorcvs::sdl2_window& window, size_t ID, const yorcvs::rect<float>& offset_rect)
    {
        if (appECS->has_components<health_component, health_stats_component>(ID)) {
            // draw health bar
            yorcvs::rect<float> healthBarRect {};
            if (appECS->has_components<sprite_component>(
                    ID)) // if the entity has a sprite component , render the health above it, not above the hitbox
            {
                healthBarRect.y = offset_rect.y - appECS->get_component<sprite_component>(ID).size.y / 2;
            } else {
                healthBarRect.y = offset_rect.y - offset_rect.h;
            }
            healthBarRect.x = offset_rect.x - health_bar_x_offset + offset_rect.w / 2;
            healthBarRect.w = health_full_bar_dimension.x;
            healthBarRect.h = health_full_bar_dimension.y;
            if (appECS->has_components<stamina_component>(ID)) {
                healthBarRect.y -= health_full_bar_dimension.y * 2;
            }
            draw_status_bar(window, healthBarRect, (appECS->get_component<health_component>(ID).HP / appECS->get_component<health_stats_component>(ID).max_HP),
                health_bar_full_color, health_bar_empty_color);
        }
    }
    void draw_entity_stamina_bar(yorcvs::sdl2_window& window, size_t ID, const yorcvs::rect<float>& offset_rect)
    {
        if (appECS->has_components<stamina_component, stamina_stats_component>(ID)) {
            yorcvs::rect<float> staminaBarRect {};
            if (appECS->has_components<sprite_component>(
                    ID)) // if the entity has a sprite component , render the health above it, not above the hitbox
            {
                staminaBarRect.y = offset_rect.y - appECS->get_component<sprite_component>(ID).size.y / 2;
            } else {
                staminaBarRect.y = offset_rect.y - offset_rect.h;
            }

            staminaBarRect.x = offset_rect.x - health_bar_x_offset + offset_rect.w / 2;

            staminaBarRect.w = health_full_bar_dimension.x;
            staminaBarRect.h = health_full_bar_dimension.y;
            draw_status_bar(window, staminaBarRect, (appECS->get_component<stamina_component>(ID).stamina / appECS->get_component<stamina_stats_component>(ID).max_stamina),
                stamina_bar_full_color, stamina_bar_empty_color);
        }
    }
    static void draw_status_bar(yorcvs::sdl2_window& window, yorcvs::rect<float> rect, float value, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> full_color, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> empty_color)
    {
        window.draw_rect(rect, std::get<0>(empty_color), std::get<1>(empty_color), std::get<2>(empty_color), std::get<3>(empty_color));
        rect.w = std::max(value * rect.w, 0.0f);
        window.draw_rect(rect, std::get<0>(full_color), std::get<1>(full_color), std::get<2>(full_color), std::get<3>(full_color));
    }

    yorcvs::sdl2_window* parentWindow {};

    yorcvs::ECS* appECS {};
    yorcvs::map* map {};
    sol::state* lua_state {};
    yorcvs::application* parent_app;
    // console
    std::string console_text;
    std::vector<std::string> console_logs;
    std::vector<std::string> console_previous_commands;
    std::optional<size_t> select_target {};
    player_movement_control* player_move_sys {};

    collision_system* colission_sys {};
    health_system* health_sys {};
    combat_system* combat_sys {};

    // controls
    bool debug_window_opened = false;
    bool console_opened = false;
    float time_accumulator = 0;
    int history_pos = 0;

    static constexpr yorcvs::vec2<float> health_full_bar_dimension = { 32.0f, 4.0f };

    const std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> health_bar_full_color { 255, 0, 0, 255 };
    const std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> health_bar_empty_color { 100, 0, 0, 255 };
    static constexpr float health_bar_x_offset = 16.0f;
    const std::vector<uint8_t> hitbox_color = { 255, 0, 0, 100 };

    static constexpr size_t text_R = 255;
    static constexpr size_t text_G = 255;
    static constexpr size_t text_B = 255;
    static constexpr size_t text_A = 255;
    static constexpr size_t text_char_size = 100;
    static constexpr size_t text_line_length = 10000;

    const std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> stamina_bar_full_color { 0, 255, 0, 100 };
    const std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> stamina_bar_empty_color { 0, 100, 0, 100 };
    static constexpr float ui_controls_update_time = 250.0f;
    static constexpr float zoom_power = 0.1f;
};

}
