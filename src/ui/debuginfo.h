#pragma once
#include "../common/ecs.h"
#include "../common/types.h"
#include "../engine/map.h"
#include "../engine/window/windowsdl2.h"
#include "../game/components.h"
#include "../game/systems.h"
#include "imgui.h"
#include "imgui_sdl.h"
#include "misc/cpp/imgui_stdlib.h"
namespace yorcvs {
class DebugInfo {
public:
    DebugInfo() = delete;

    DebugInfo(yorcvs::sdl2_window* parentW, yorcvs::Map* map_object, PlayerMovementControl* pms, CollisionSystem* cols,
        HealthSystem* healthS, sol::state* lua)
        : parentWindow(parentW)
        , appECS(map_object->ecs)
        , map(map_object)
        , lua_state(lua)
        , playerMoveSystem(pms)
        , colSystem(cols)
    {

        attach(parentW, map_object, pms, cols, healthS, lua);
    }
    ~DebugInfo() = default;
    DebugInfo(const DebugInfo& other) = delete;
    DebugInfo(DebugInfo&& other) = delete;
    DebugInfo operator=(const DebugInfo& other) = delete;
    DebugInfo operator=(DebugInfo&& other) = delete;

    void update(const float elapsed, yorcvs::Vec2<float>& render_dimensions)
    {
        time_accumulator += elapsed;
        if (time_accumulator >= ui_controls_update_time) {
            if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_LCTRL)) {
                if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_E)) {
                    showDebugWindow = !showDebugWindow;
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(Events::Key::YORCVS_KEY_TILDE)) {
                    playerMoveSystem->controls_enable = !playerMoveSystem->controls_enable;
                    showConsole = !showConsole;
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
                    out << map->save_character(playerMoveSystem->entityList->entitiesID[0]);
                    yorcvs::log("Done.");
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(yorcvs::Events::YORCVS_KEY_R)) {
                    reset();
                }
            }
        }
        if (!playerMoveSystem->entityList->entitiesID.empty()) {
            (*lua_state)["playerID"] = playerMoveSystem->entityList->entitiesID[0];
        }
    }

    void render_hitboxes(yorcvs::sdl2_window& window, const yorcvs::Vec2<float>& render_dimensions, const uint8_t r,
        const uint8_t g, const uint8_t b, const uint8_t a)
    {
        yorcvs::Vec2<float> old_rs = window.get_render_scale();
        window.set_render_scale(window.get_window_size() / render_dimensions);

        yorcvs::Rect<float> rect {};
        for (const auto& ID : colSystem->entityList->entitiesID) {
            rect.x = appECS->get_component<positionComponent>(ID).position.x + appECS->get_component<hitboxComponent>(ID).hitbox.x;
            rect.y = appECS->get_component<positionComponent>(ID).position.y + appECS->get_component<hitboxComponent>(ID).hitbox.y;
            rect.w = appECS->get_component<hitboxComponent>(ID).hitbox.w;
            rect.h = appECS->get_component<hitboxComponent>(ID).hitbox.h;
            window.draw_rect(rect, r, g, b, a);
            if (appECS->has_components<healthComponent>(ID)) {
                /// draw health bar
                yorcvs::Rect<float> healthBarRect {};
                if (appECS->has_components<spriteComponent>(
                        ID)) // if the entity has a sprite component , render the health above it, not above the hitbox
                {
                    healthBarRect.y = rect.y - appECS->get_component<spriteComponent>(ID).size.y / 2;
                } else {
                    healthBarRect.y = rect.y - rect.h;
                }
                healthBarRect.x = rect.x - health_bar_x_offset + rect.w / 2;
                healthBarRect.w = health_full_bar_dimension.x;
                healthBarRect.h = health_full_bar_dimension.y;
                if (appECS->has_components<staminaComponent>(ID)) {
                    healthBarRect.y -= health_full_bar_dimension.y * 2;
                }
                window.draw_rect(healthBarRect, health_bar_empty_color[0], health_bar_empty_color[1],
                    health_bar_empty_color[2], health_bar_empty_color[3]);
                healthBarRect.w = (appECS->get_component<healthComponent>(ID).HP / appECS->get_component<healthComponent>(ID).max_HP) * health_bar_base_width;
                window.draw_rect(healthBarRect, health_bar_full_color[0], health_bar_full_color[1],
                    health_bar_full_color[2], health_bar_full_color[3]);
            }
            if (appECS->has_components<staminaComponent>(ID)) {
                yorcvs::Rect<float> staminaBarRect {};
                if (appECS->has_components<spriteComponent>(
                        ID)) // if the entity has a sprite component , render the health above it, not above the hitbox
                {
                    staminaBarRect.y = rect.y - appECS->get_component<spriteComponent>(ID).size.y / 2;
                } else {
                    staminaBarRect.y = rect.y - rect.h;
                }

                staminaBarRect.x = rect.x - health_bar_x_offset + rect.w / 2;

                staminaBarRect.w = health_full_bar_dimension.x;
                staminaBarRect.h = health_full_bar_dimension.y;
                window.draw_rect(staminaBarRect, stamina_bar_empty_color[0], stamina_bar_empty_color[1],
                    stamina_bar_empty_color[2], stamina_bar_empty_color[3]);
                staminaBarRect.w = (appECS->get_component<staminaComponent>(ID).stamina / appECS->get_component<staminaComponent>(ID).max_stamina) * health_bar_base_width;
                window.draw_rect(staminaBarRect, stamina_bar_full_color[0], stamina_bar_full_color[1],
                    stamina_bar_full_color[2], stamina_bar_full_color[3]);
            }
        }
        window.set_render_scale(old_rs);
    }

    void render(yorcvs::Vec2<float>& render_dimensions)
    {

        if (showDebugWindow) {
            show_performance_window();
            show_debug_window(render_dimensions);
        }
        if (showConsole) {
            show_console_window();
            show_entities_table();
        }
    }

    void attach(yorcvs::sdl2_window* parentW, yorcvs::Map* map_object, PlayerMovementControl* pms,
        CollisionSystem* cols, HealthSystem* healthS, sol::state* lua)
    {
        lua_state = lua;
        attach_lua();
        parentWindow = parentW;
        map = map_object;
        appECS = map->ecs;
        playerMoveSystem = pms;
        colSystem = cols;
        healthSys = healthS;
    }
    void attach_lua()
    {
        lua_state->set_function("internal_log", &DebugInfo::add_log, this);
        lua_state->script("function log(message) internal_log(tostring(message)) end");
        (*lua_state)["print"] = (*lua_state)["log"];
    }

    enum update_time_item : size_t {
        collision = 0,
        health = 1,
        stamina = 2,
        velocity = 3,
        animation = 4,
        behaviour = 5,
        overall = 6,
        update_time_tracked
    };
    // samples , max , min , avg
    enum update_time_sample_tuple_element : size_t {
        samples = 0,
        max = 1,
        min = 2,
        avg = 3,
        update_time_sample_tuple_elements
    };

    template <update_time_item item>
    void record_update_time(float value)
    {
        std::deque<float>& queue = std::get<1>(update_time_history[static_cast<size_t>(item)]);
        auto& statistics = update_time_statistics[static_cast<size_t>(item)];
        if (queue.size() == update_time_maximum_samples) {
            queue.pop_front();
        }
        queue.push_back(value);

        // compute statistics
        std::get<update_time_sample_tuple_element::avg>(statistics) *= std::get<0>(statistics);
        std::get<update_time_sample_tuple_element::samples>(statistics) += 1.0f;
        if (std::get<update_time_sample_tuple_element::max>(statistics) < value) {
            std::get<update_time_sample_tuple_element::max>(statistics) = value;
        }
        if (std::get<update_time_sample_tuple_element::min>(statistics) > value) {
            std::get<update_time_sample_tuple_element::min>(statistics) = value;
        }
        std::get<update_time_sample_tuple_element::avg>(statistics) += value;
        std::get<update_time_sample_tuple_element::avg>(statistics) /= std::get<0>(statistics);
    }

private:
    static float get_update_time_sample(void* data, int index)
    {
        std::deque<float>* queue = static_cast<std::deque<float>*>(data);
        return (*queue)[index];
    }
    void show_performance_window()
    {
        ImGui::Begin("Performance");
        for (size_t i = 0; i < update_time_item::update_time_tracked; i++) {
            show_performance_parameter(i);
        }
        ImGui::End();
    }

    void show_performance_parameter(size_t index)
    {
        const auto& [label, queue] = update_time_history.at(index);
        const auto [samples, max, min, avg] = update_time_statistics.at(index);
        if (ImGui::CollapsingHeader(label.c_str())) {
            ImGui::PlotLines("", get_update_time_sample, (void*)(&queue), static_cast<int>(queue.size()));
            ImGui::Text("Current: %f", queue.back());
            ImGui::Text("Max: %f", max);
            ImGui::Text("Avg: %f", avg);
            ImGui::Text("Min: %f", min);
        }
    }

    void show_debug_window(yorcvs::Vec2<float>& render_dimensions)
    {
        render_hitboxes(*parentWindow, render_dimensions, hitbox_color[0], hitbox_color[1], hitbox_color[2],
            hitbox_color[3]);

        if (!playerMoveSystem->entityList->entitiesID.empty()) {
            const size_t ID = playerMoveSystem->entityList->entitiesID[0];
            ImGui::Begin("Player");
            show_entity_stats(ID, "Player : ");
            ImGui::End();
        }
    }
    void show_entity_stats(size_t ID, [[maybe_unused]] std::string pre_name = "Entity : ")
    {
        if (appECS->has_components<identificationComponent>(ID)) {
            pre_name += appECS->get_component<identificationComponent>(ID).name + " (" + std::to_string(ID) + ")";
        }
        ImGui::Text("%s", pre_name.c_str());
        if (appECS->has_components<spriteComponent>(ID)) {
            static constexpr float size_multiplier = 4.0f;
            const spriteComponent& comp = appECS->get_component<spriteComponent>(ID);

            int texture_size_x {};
            int texture_size_y {};

            SDL_QueryTexture(parentWindow->assetm->load_from_file(comp.texture_path).get(), nullptr, nullptr,
                &texture_size_x, &texture_size_y);

            const yorcvs::Vec2<float> top_corner = {
                static_cast<float>(comp.src_rect.x) / static_cast<float>(texture_size_x),
                static_cast<float>(comp.src_rect.y) / static_cast<float>(texture_size_y)
            };
            const yorcvs::Vec2<float> bottom_corner = {
                static_cast<float>(comp.src_rect.x + comp.src_rect.w) / static_cast<float>(texture_size_x),
                static_cast<float>(comp.src_rect.y + comp.src_rect.h) / static_cast<float>(texture_size_y)
            };
            ImGui::Image(parentWindow->assetm->load_from_file(comp.texture_path).get(),
                { size_multiplier * comp.size.x, size_multiplier * comp.size.y }, { top_corner.x, top_corner.y },
                { bottom_corner.x, bottom_corner.y });
        }
        if (appECS->has_components<positionComponent>(ID)) {
            ImGui::Text("Position: (%f,%f)", appECS->get_component<positionComponent>(ID).position.x,
                appECS->get_component<positionComponent>(ID).position.y);
        }
        if (appECS->has_components<velocityComponent>(ID)) {
            ImGui::Text("Velocity: (%f,%f)", appECS->get_component<velocityComponent>(ID).vel.x,
                appECS->get_component<velocityComponent>(ID).vel.y);
        }
        if (appECS->has_components<healthComponent>(ID)) {
            auto& playerHealthC = appECS->get_component<healthComponent>(ID);
            ImGui::Text("Health: (%f/%f)", playerHealthC.HP, playerHealthC.max_HP);
        }
        if (appECS->has_components<staminaComponent>(ID)) {
            auto& playerStaminaC = appECS->get_component<staminaComponent>(ID);
            ImGui::Text("Stamina: (%f/%f)", playerStaminaC.stamina, playerStaminaC.max_stamina);
        }

        if (appECS->has_components<offensiveStatsComponent>(ID)) {
            auto& offStatsC = appECS->get_component<offensiveStatsComponent>(ID);
            ImGui::Text("Strength : (%f)", offStatsC.strength);
            ImGui::Text("Agility : (%f)", offStatsC.agility);
            ImGui::Text("Dexterity : (%f)", offStatsC.dexterity);
            ImGui::Text("Piercing : (%f)", offStatsC.piercing);
            ImGui::Text("Intellect : (%f)", offStatsC.intellect);
        }
        if (appECS->has_components<defensiveStatsComponent>(ID)) {
            auto& defstats = appECS->get_component<defensiveStatsComponent>(ID);
            ImGui::Text("Defense : (%f)", defstats.defense);
            ImGui::Text("Block : (%f)", defstats.block);
            ImGui::Text("Dodge : (%f)", defstats.dodge);
            ImGui::Text("Spirit : (%f)", defstats.spirit);
        }
    }
    void show_entity_interaction_window(size_t sender, size_t target)
    {
        ImGui::Text("%s", std::to_string(target).c_str());
        show_entity_stats(target);

        if (ImGui::Button("go to") && appECS->has_components<positionComponent>(target) && appECS->has_components<positionComponent>(target)) {
            appECS->get_component<positionComponent>(sender) = appECS->get_component<positionComponent>(target);
        }
        if (ImGui::Button("teleport here") && appECS->has_components<positionComponent>(target) && appECS->has_components<positionComponent>(target)) {
            appECS->get_component<positionComponent>(target) = appECS->get_component<positionComponent>(sender);
        }
    }
    void show_console_window()
    {
        ImGui::ShowDemoWindow();
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
        if (ImGui::InputText("##", &console_text, input_text_flags, &DebugInfo::TextEditCallbackStub, (void*)this) && !console_text.empty()) {
            console_logs.push_back(console_text);
            HistoryPos = -1;
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
        ImGui::Begin("Debug");
        if (!(ImGui::CollapsingHeader("Entities") && ImGui::BeginTable("table1", collumn_count, flags1))) {
            ImGui::End();
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
                show_entity_interaction_window(get_first_player_id(), i);
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::Text("%zu", i);

            ImGui::TableSetColumnIndex(1);
            if (appECS->has_components<identificationComponent>(i)) {
                ImGui::Text("%s", appECS->get_component<identificationComponent>(i).name.c_str());
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
            if (appECS->has_components<positionComponent>(i)) {
                const auto& position = appECS->get_component<positionComponent>(i).position;
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

        ImGui::End();
    }

    void reset()
    {
        // clear graphs
        for (auto& [parameter_name, queue] : update_time_history) {
            queue.clear();
        }
        // clear statistics
        for (auto& tup : update_time_statistics) {
            std::get<update_time_sample_tuple_element::samples>(tup) = 0.0f;
            std::get<update_time_sample_tuple_element::max>(tup) = 0.0f;
            std::get<update_time_sample_tuple_element::min>(tup) = std::numeric_limits<float>::max();
            std::get<update_time_sample_tuple_element::avg>(tup) = 0.0f;
        }
    }

    void add_log(const std::string& message)
    {
        console_logs.push_back(message);
    }
    void clear_logs()
    {
        console_logs.clear();
        console_previous_commands.clear();
        HistoryPos = -1;
    }
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        auto* console = static_cast<DebugInfo*>(data->UserData);
        return console->TextEditCallback(data);
    }

    int TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        // AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag) {
        case ImGuiInputTextFlags_CallbackHistory: {
            // Example of HISTORY
            const int prev_history_pos = HistoryPos;
            if (data->EventKey == ImGuiKey_UpArrow) {
                if (HistoryPos == -1) {
                    HistoryPos = static_cast<int>(console_previous_commands.size()) - 1;
                } else if (HistoryPos > 0) {
                    HistoryPos--;
                }
            } else if (data->EventKey == ImGuiKey_DownArrow) {
                if (HistoryPos != -1) {
                    if (++HistoryPos >= static_cast<int>(console_previous_commands.size())) {
                        HistoryPos = -1;
                    }
                }
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != HistoryPos) {
                const char* history_str = (HistoryPos >= 0) ? console_previous_commands[HistoryPos].c_str() : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
        }
        return 1;
    }

    size_t get_first_player_id()
    {
        if (!playerMoveSystem->entityList->entitiesID.empty()) {
            return playerMoveSystem->entityList->entitiesID[0];
        }
        const size_t invalidID = appECS->create_entity_ID();
        appECS->destroy_entity(invalidID);
        return invalidID;
    }
    std::vector<size_t> callbacks;
    yorcvs::sdl2_window* parentWindow {};

    yorcvs::ECS* appECS {};
    yorcvs::Map* map {};
    sol::state* lua_state {};

    // performance
    static constexpr size_t update_time_maximum_samples = 25;
    std::array<std::tuple<std::string, std::deque<float>>, update_time_item::update_time_tracked> update_time_history {
        { { "collision", {} },
            { "health", {} },
            { "stamina", {} },
            { "velocity", {} },
            { "animation", {} },
            { "behaviour", {} },
            { "overall", {} } }
    };
    std::array<std::tuple<float, float, float, float>, update_time_item::update_time_tracked>
        update_time_statistics {}; // samples , max , min , avg
    // console
    std::string console_text;
    std::vector<std::string> console_logs;
    std::vector<std::string> console_previous_commands;
    PlayerMovementControl* playerMoveSystem {};

    CollisionSystem* colSystem {};
    HealthSystem* healthSys {};

    // controls
    bool showDebugWindow = false;
    bool showConsole = false;
    float time_accumulator = 0;
    int HistoryPos = 0;

    static constexpr yorcvs::Vec2<float> health_full_bar_dimension = { 32.0f, 4.0f };
    const std::vector<uint8_t> health_bar_full_color = { 255, 0, 0, 255 };
    const std::vector<uint8_t> health_bar_empty_color = { 100, 0, 0, 255 };
    static constexpr float health_bar_base_width = 32.0f;
    static constexpr float health_bar_x_offset = 16.0f;
    const std::vector<uint8_t> hitbox_color = { 255, 0, 0, 100 };

    static constexpr size_t textR = 255;
    static constexpr size_t textG = 255;
    static constexpr size_t textB = 255;
    static constexpr size_t textA = 255;
    static constexpr size_t text_char_size = 100;
    static constexpr size_t text_line_length = 10000;

    const std::vector<uint8_t> stamina_bar_full_color = { 0, 255, 0, 100 };
    const std::vector<uint8_t> stamina_bar_empty_color = { 0, 100, 0, 100 };

    static constexpr float ui_controls_update_time = 250.0f;
    static constexpr float zoom_power = 0.1f;
};

}
