/**
 * @file Yorcvs.h
 * @author Dantsz
 * @brief
 * @version 0.1
 * @date 2021-07-31
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once

#include "Yorcvs.h"
#include "common/ecs.h"
#include "common/types.h"
#include "common/utilities.h"
#include "engine/luaEngine.h"
#include "engine/map.h"
#include "game/componentSerialization.h"
#include "game/components.h"
#include "game/systems.h"

#include "engine/window/windowsdl2.h"
#include <cstdlib>
#include <exception>
#include <future>

#include <string>
#include <vector>

#include <cmath>
#include <filesystem>
#include <fstream>

#include "sol/sol.hpp"

#include "imgui.h"
#include "imgui_sdl.h"
#include "misc/cpp/imgui_stdlib.h"

namespace yorcvs
{
class DebugInfo
{
  public:
    DebugInfo() = default;

    DebugInfo(yorcvs::sdl2_window *parentW, yorcvs::Map *map, PlayerMovementControl *pms, CollisionSystem *cols,
              HealthSystem *healthS, sol::state *lua)
        : parentWindow(parentW), appECS(map->ecs), map(map), lua_state(lua), playerMoveSystem(pms), colSystem(cols)
    {
        attach(parentW, map, pms, cols, healthS, lua);
    }
    ~DebugInfo() = default;
    DebugInfo(const DebugInfo &other) = delete;
    DebugInfo(DebugInfo &&other) = delete;
    DebugInfo operator=(const DebugInfo &other) = delete;
    DebugInfo operator=(DebugInfo &&other) = delete;

    void update(const float elapsed, yorcvs::Vec2<float> &render_dimensions)
    {
        time_accumulator += elapsed;
        if (time_accumulator >= ui_controls_update_time)
        {
            if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_LCTRL))
            {
                if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_E))
                {
                    showDebugWindow = !showDebugWindow;
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(Events::Key::YORCVS_KEY_TILDE))
                {
                    playerMoveSystem->controls_enable = !playerMoveSystem->controls_enable;
                    showConsole = !showConsole;
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_I))
                {
                    render_dimensions -= render_dimensions * zoom_power;
                    time_accumulator = 0;
                }

                if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_K))
                {
                    render_dimensions += render_dimensions * zoom_power;
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(yorcvs::Events::Key::YORCVS_KEY_C))
                {
                    yorcvs::log("Saving player...");
                    std::ofstream out("assets/testPlayer.json");
                    out << map->save_character(playerMoveSystem->entityList->entitiesID[0]);
                    yorcvs::log("Done.");
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(yorcvs::Events::YORCVS_KEY_R))
                {
                    reset();
                }
            }
        }
        frame_time = elapsed;
        avg_frame_time *= frame_time_samples;
        frame_time_samples += 1.0f;
        avg_frame_time += elapsed;
        avg_frame_time /= frame_time_samples;
        if (!playerMoveSystem->entityList->entitiesID.empty())
        {
            (*lua_state)["playerID"] = playerMoveSystem->entityList->entitiesID[0];
        }
        if (showDebugWindow)
        {
            if (elapsed > maxFrameTime)
            {
                maxFrameTime = elapsed;
            }
        }
    }

    void render_hitboxes(yorcvs::sdl2_window &window, const yorcvs::Vec2<float> &render_dimensions, const size_t r,
                         const size_t g, const size_t b, const size_t a)
    {
        yorcvs::Vec2<float> old_rs = window.get_render_scale();
        window.set_render_scale(window.get_window_size() / render_dimensions);

        yorcvs::Rect<float> rect{};
        for (const auto &ID : colSystem->entityList->entitiesID)
        {
            rect.x = appECS->get_component<positionComponent>(ID).position.x +
                     appECS->get_component<hitboxComponent>(ID).hitbox.x;
            rect.y = appECS->get_component<positionComponent>(ID).position.y +
                     appECS->get_component<hitboxComponent>(ID).hitbox.y;
            rect.w = appECS->get_component<hitboxComponent>(ID).hitbox.w;
            rect.h = appECS->get_component<hitboxComponent>(ID).hitbox.h;
            window.draw_rect(rect, r, g, b, a);
            if (appECS->has_components<healthComponent>(ID))
            {
                /// draw health bar

                yorcvs::Rect<float> healthBarRect{};
                if (appECS->has_components<spriteComponent>(
                        ID)) // if the entity has a sprite component , render the health above it, not above the hitbox
                {
                    healthBarRect.y = rect.y - appECS->get_component<spriteComponent>(ID).size.y / 2;
                }
                else
                {
                    healthBarRect.y = rect.y - rect.h;
                }

                healthBarRect.x = rect.x - health_bar_x_offset + rect.w / 2;

                healthBarRect.w = health_full_bar_dimension.x;
                healthBarRect.h = health_full_bar_dimension.y;
                if (appECS->has_components<staminaComponent>(ID))
                {
                    healthBarRect.y -= health_full_bar_dimension.y * 2;
                }
                window.draw_rect(healthBarRect, health_bar_empty_color[0], health_bar_empty_color[1],
                                 health_bar_empty_color[2], health_bar_empty_color[3]);
                healthBarRect.w = (appECS->get_component<healthComponent>(ID).HP /
                                   appECS->get_component<healthComponent>(ID).max_HP) *
                                  health_bar_base_width;
                window.draw_rect(healthBarRect, health_bar_full_color[0], health_bar_full_color[1],
                                 health_bar_full_color[2], health_bar_full_color[3]);
            }
            if (appECS->has_components<staminaComponent>(ID))
            {
                yorcvs::Rect<float> staminaBarRect{};
                if (appECS->has_components<spriteComponent>(
                        ID)) // if the entity has a sprite component , render the health above it, not above the hitbox
                {
                    staminaBarRect.y = rect.y - appECS->get_component<spriteComponent>(ID).size.y / 2;
                }
                else
                {
                    staminaBarRect.y = rect.y - rect.h;
                }

                staminaBarRect.x = rect.x - health_bar_x_offset + rect.w / 2;

                staminaBarRect.w = health_full_bar_dimension.x;
                staminaBarRect.h = health_full_bar_dimension.y;
                window.draw_rect(staminaBarRect, stamina_bar_empty_color[0], stamina_bar_empty_color[1],
                                 stamina_bar_empty_color[2], stamina_bar_empty_color[3]);
                staminaBarRect.w = (appECS->get_component<staminaComponent>(ID).stamina /
                                    appECS->get_component<staminaComponent>(ID).max_stamina) *
                                   health_bar_base_width;
                window.draw_rect(staminaBarRect, stamina_bar_full_color[0], stamina_bar_full_color[1],
                                 stamina_bar_full_color[2], stamina_bar_full_color[3]);
            }
        }
        window.set_render_scale(old_rs);
    }

    void render(yorcvs::Vec2<float> &render_dimensions)
    {

        if (showDebugWindow)
        {
            show_debug_window(render_dimensions);
        }
        if (showConsole)
        {
            show_console_window();
            show_entities_table();
        }
    }

    void attach(yorcvs::sdl2_window *parentW, yorcvs::Map *map, PlayerMovementControl *pms, CollisionSystem *cols,
                HealthSystem *healthS, sol::state *lua)
    {
        lua_state = lua;
        attach_lua();
        parentWindow = parentW;
        this->map = map;
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

  private:
    void show_debug_window(yorcvs::Vec2<float> &render_dimensions)
    {
        render_hitboxes(*parentWindow, render_dimensions, hitbox_color[0], hitbox_color[1], hitbox_color[2],
                        hitbox_color[3]);
        ImGui::Begin("DebugWindow");
        ImGui::Text("frameTime: %f", frame_time);
        ImGui::Text("maxFramTime: %f", maxFrameTime);
        ImGui::Text("avgFrameTime: %f", avg_frame_time);
        ImGui::Text("ecsEntities: %zu", appECS->get_active_entities_number());
        if (!playerMoveSystem->entityList->entitiesID.empty())
        {
            const size_t ID = playerMoveSystem->entityList->entitiesID[0];
            if (appECS->has_components<positionComponent>(ID))
            {
                ImGui::Text("playerPosition: (%f,%f)", appECS->get_component<positionComponent>(ID).position.x,
                            appECS->get_component<positionComponent>(ID).position.y);
            }
            if (appECS->has_components<velocityComponent>(ID))
            {
                ImGui::Text("playerVelocity: (%f,%f)", appECS->get_component<velocityComponent>(ID).vel.x,
                            appECS->get_component<velocityComponent>(ID).vel.y);
            }
        }
        ImGui::End();

        if (!playerMoveSystem->entityList->entitiesID.empty())
        {
            const size_t ID = playerMoveSystem->entityList->entitiesID[0];
            std::string playerName = "Player : ";
            if (appECS->has_components<identificationComponent>(ID))
            {
                playerName += appECS->get_component<identificationComponent>(ID).name + " (" + std::to_string(ID) + ")";
            }
            ImGui::Begin(playerName.c_str());
            if (appECS->has_components<healthComponent>(ID))
            {
                healthComponent &playerHealthC = appECS->get_component<healthComponent>(ID);
                ImGui::Text("playerHealth: (%f/%f)", playerHealthC.HP, playerHealthC.max_HP);
            }
            if (appECS->has_components<staminaComponent>(ID))
            {
                staminaComponent &playerStaminaC = appECS->get_component<staminaComponent>(ID);
                ImGui::Text("stamina: (%f/%f)", playerStaminaC.stamina, playerStaminaC.max_stamina);
            }

            if (appECS->has_components<offensiveStatsComponent>(ID))
            {
                offensiveStatsComponent &offStatsC = appECS->get_component<offensiveStatsComponent>(ID);
                ImGui::Text("Strength : (%f)", offStatsC.strength);
                ImGui::Text("Agility : (%f)", offStatsC.agility);
                ImGui::Text("Dexterity : (%f)", offStatsC.dexterity);
                ImGui::Text("Piercing : (%f)", offStatsC.piercing);
                ImGui::Text("Intellect : (%f)", offStatsC.intellect);
            }
            if (appECS->has_components<defensiveStatsComponent>(ID))
            {
                defensiveStatsComponent &defstats = appECS->get_component<defensiveStatsComponent>(ID);
                ImGui::Text("Defense : (%f)", defstats.defense);
                ImGui::Text("Block : (%f)", defstats.block);
                ImGui::Text("Dodge : (%f)", defstats.dodge);
                ImGui::Text("Spirit : (%f)", defstats.spirit);
            }

            ImGui::End();
        }
    }
    void show_console_window()
    {
        ImGui::ShowDemoWindow();
        ImGui::Begin("Console");
        ImGuiInputTextFlags input_text_flags =
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -3 * footer_height_to_reserve), false,
                          ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto &item : console_logs)
        {
            ImGui::TextUnformatted(item.c_str());
        }
        ImGui::EndChild();
        ImGui::Separator();
        bool reclaim_focus = false;
        if (ImGui::InputText("##", &console_text, input_text_flags, &DebugInfo::TextEditCallbackStub, (void *)this) &&
            !console_text.empty())
        {
            console_logs.push_back(console_text);
            HistoryPos = -1;
            console_previous_commands.push_back(console_text);
            auto rez = lua_state->safe_script(console_text,
                                              [](lua_State *, sol::protected_function_result pfr) { return pfr; });
            if (!rez.valid())
            {
                sol::error err = rez;
                std::string text = err.what();
                console_logs.push_back(std::move(text));
            }
            console_text = "";
            reclaim_focus = true;
        }
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
        {
            ImGui::SetKeyboardFocusHere(-1);
        }
        if (ImGui::SmallButton("Clear"))
        {
            clear_logs();
        }
        ImGui::End();
    }
    void show_entities_table()
    {
        const ImGuiTableFlags flags1 = ImGuiTableFlags_BordersV | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Resizable;
        ImGui::Begin("Debug");
        if (ImGui::CollapsingHeader("Entities") && ImGui::BeginTable("table1", 4, flags1))
        {

            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Signature");
            ImGui::TableSetupColumn("Position");
            ImGui::TableHeadersRow();
            for (size_t i = 0; i < appECS->get_entity_list_size(); i++)
            {
                if (!appECS->is_valid_entity(i))
                {
                    continue;
                }
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_None;
                ImGui::Selectable("", false, selectable_flags);
                ImGui::SameLine();
                ImGui::Text("%zu", i);

                ImGui::TableSetColumnIndex(1);
                if (appECS->has_components<identificationComponent>(i))
                {
                    ImGui::Text("%s", appECS->get_component<identificationComponent>(i).name.c_str());
                }
                else
                {
                    ImGui::Text("%s", "Unknown");
                }

                ImGui::TableSetColumnIndex(2);
                std::string signature{};
                signature.resize(appECS->get_entity_signature(i).size());
                for (size_t j = 0; j < appECS->get_entity_signature(i).size(); j++)
                {
                    signature[j] = appECS->get_entity_signature(i)[j] ? '1' : '0';
                }
                ImGui::Text("%s", signature.c_str());
                ImGui::TableSetColumnIndex(3);
                if (appECS->has_components<positionComponent>(i))
                {
                    const auto &position = appECS->get_component<positionComponent>(i).position;
                    ImGui::Text("%f/%f", position.x, position.y);
                }
                else
                {
                    ImGui::Text("(-/-)");
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void reset()
    {
        maxFrameTime = 0.0f;
    }

    void add_log(const std::string &message)
    {
        console_logs.push_back(message);
    }
    void clear_logs()
    {
        console_logs.clear();
        console_previous_commands.clear();
        HistoryPos = -1;
    }
    static int TextEditCallbackStub(ImGuiInputTextCallbackData *data)
    {
        DebugInfo *console = static_cast<DebugInfo *>(data->UserData);
        return console->TextEditCallback(data);
    }

    int TextEditCallback(ImGuiInputTextCallbackData *data)
    {
        // AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackHistory: {
            // Example of HISTORY
            const int prev_history_pos = HistoryPos;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (HistoryPos == -1)
                {
                    HistoryPos = static_cast<int>(console_previous_commands.size()) - 1;
                }
                else if (HistoryPos > 0)
                {
                    HistoryPos--;
                }
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (HistoryPos != -1)
                {
                    if (++HistoryPos >= static_cast<int>(console_previous_commands.size()))
                    {
                        HistoryPos = -1;
                    }
                }
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != HistoryPos)
            {
                const char *history_str = (HistoryPos >= 0) ? console_previous_commands[HistoryPos].c_str() : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
        }
        return 1;
    }

    std::vector<size_t> callbacks;
    yorcvs::sdl2_window *parentWindow{};
    yorcvs::ECS *appECS{};
    yorcvs::Map *map{};
    sol::state *lua_state{};
    // debug window
    float frame_time = 0.0f;
    float maxFrameTime = 0.0f;
    float frame_time_samples = 0.0f;
    float avg_frame_time = 0.0f;
    // console
    std::string console_text;
    std::vector<std::string> console_logs;
    std::vector<std::string> console_previous_commands;
    PlayerMovementControl *playerMoveSystem{};

    CollisionSystem *colSystem{};
    HealthSystem *healthSys{};

    // controls
    bool showDebugWindow = false;
    bool showConsole = false;
    float time_accumulator = 0;
    int HistoryPos = 0;

    static constexpr yorcvs::Vec2<float> health_full_bar_dimension = {32.0f, 4.0f};
    const std::vector<uint8_t> health_bar_full_color = {255, 0, 0, 255};
    const std::vector<uint8_t> health_bar_empty_color = {100, 0, 0, 255};
    static constexpr float health_bar_base_width = 32.0f;
    static constexpr float health_bar_x_offset = 16.0f;
    const std::vector<uint8_t> hitbox_color = {255, 0, 0, 100};

    static constexpr size_t textR = 255;
    static constexpr size_t textG = 255;
    static constexpr size_t textB = 255;
    static constexpr size_t textA = 255;
    static constexpr size_t text_char_size = 100;
    static constexpr size_t text_line_length = 10000;

    const std::vector<uint8_t> stamina_bar_full_color = {0, 255, 0, 100};
    const std::vector<uint8_t> stamina_bar_empty_color = {0, 100, 0, 100};

    static constexpr float ui_controls_update_time = 250.0f;
    static constexpr float zoom_power = 0.1f;
};

// TODO: MAKE SOME SYSTEMS MAP-DEPENDENT AND REMOVE THIS

/**
 * @brief Main game class
 *
 */
class Application
{
  public:
    Application()
    {
        lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
        yorcvs::lua::bind_runtime(lua_state, &world);

        yorcvs::lua::register_system_to_lua(lua_state, "Health_system", map.healthS);
        yorcvs::lua::register_system_to_lua(lua_state, "Collision_system", map.collisionS);
        yorcvs::lua::register_system_to_lua(lua_state, "Animation_system", map.animS);
        yorcvs::lua::register_system_to_lua(lua_state, "Combat_system", map.combat_system, "attack",
                                            &CombatSystem::attack);
        lua_state["test_map"] = &map;
        // loading two maps one on top of each other
        lua_state.safe_script(R"(
            test_map:load_content("assets/map.tmx")
            local pl = test_map:load_entity_from_path(world:create_entity(),"assets/player.json")
            world:add_playerMovementControl(pl)
            )");
        r.add_callback([&app_active = active](const yorcvs::event &e) {
            if (e.get_type() == yorcvs::Events::Type::WINDOW_QUIT)
            {
                app_active = false;
            }
        });

        dbInfo.attach(&r, &map, &pcS, &map.collisionS, &map.healthS, &lua_state);
        counter.start();
    }
    Application(const Application &other) = delete;
    Application(Application &&other) = delete;
    Application &operator=(const Application &other) = delete;
    Application &operator=(Application &&other) = delete;

    void render_map_chunk(yorcvs::Map &p_map, const std::tuple<intmax_t, intmax_t> &chunk)
    {
        if (p_map.tiles_chunks.find(chunk) != p_map.tiles_chunks.end())
        {
            const auto &tiles = p_map.tiles_chunks.at(chunk);
            for (const auto &tile : tiles)
            {
                r.draw_texture(tile.texture_path, {tile.coords.x, tile.coords.y, p_map.tilesSize.x, p_map.tilesSize.y},
                               tile.srcRect);
            }
        }
    }
    void render_map_tiles(yorcvs::Map &p_map)
    {
        yorcvs::Vec2<float> render_scale = r.get_render_scale();
        r.set_render_scale(r.get_window_size() / render_dimensions);
        // get player position
        const size_t entity_ID = pcS.entityList->entitiesID[0];
        const yorcvs::Vec2<float> player_position = world.get_component<positionComponent>(entity_ID).position;
        const std::tuple<intmax_t, intmax_t> player_position_chunk = std::tuple<intmax_t, intmax_t>(
            std::floor(player_position.x / (32.0f * 16.0f)), std::floor(player_position.y / (32.0f * 16.0f)));
        // render chunks

        std::tuple<intmax_t, intmax_t> chunk_to_be_rendered{};
        for (intmax_t x = render_distance * -1; x <= render_distance; x++)
        {
            for (intmax_t y = -1 * render_distance; y <= render_distance; y++)
            {
                chunk_to_be_rendered = std::make_tuple<intmax_t, intmax_t>(std::get<0>(player_position_chunk) + x,
                                                                           std::get<1>(player_position_chunk) + y);
                render_map_chunk(p_map, chunk_to_be_rendered);
            }
        }

        r.set_render_scale(render_scale);
    }
    void run()
    {
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        const float elapsed = std::min(100.0f, counter.get_ticks<float, std::chrono::nanoseconds>() / 1000000.0f);
        counter.stop();
        counter.start();

        lag += elapsed;

        r.handle_events();
        while (lag >= msPF)
        {
            dbInfo.update(msPF, render_dimensions);
            pcS.updateControls(render_dimensions, msPF);
            bhvS.update(msPF);
            map.update(msPF);
            lag -= msPF;
        }
        r.clear();
        render_map_tiles(map);
        sprS.renderSprites(render_dimensions);
        dbInfo.render(render_dimensions);
        ImGui::Render();
        ImGuiSDL::Render(ImGui::GetDrawData());
        r.present();
    }

    [[nodiscard]] bool is_active() const
    {
        return active;
    }

    ~Application() = default;

  private:
    static constexpr yorcvs::Vec2<float> default_render_dimensions = {240.0f, 120.0f};
    static constexpr float msPF = 41.6f;
    static constexpr intmax_t default_render_distance = 1;

    yorcvs::sdl2_window r;
    yorcvs::Timer counter;

    float lag = 0.0f;
    yorcvs::Vec2<float> render_dimensions = default_render_dimensions; // how much to render
    intmax_t render_distance = default_render_distance;
    yorcvs::ECS world{};
    sol::state lua_state;
    yorcvs::Map map{&world};
    SpriteSystem sprS{map.ecs, &r};
    PlayerMovementControl pcS{map.ecs, &r};
    BehaviourSystem bhvS{map.ecs, &lua_state};

    DebugInfo dbInfo;

    bool active = true;
};
} // namespace yorcvs