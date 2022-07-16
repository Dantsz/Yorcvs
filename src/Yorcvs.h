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

#include "imgui.h"
#include "imgui_sdl.h"
#include "misc/cpp/imgui_stdlib.h"

#include "common/ecs.h"
#include "common/types.h"
#include "common/utilities.h"
#include "engine/luaEngine.h"
#include "engine/map.h"
#include "game/componentSerialization.h"
#include "game/components.h"
#include "game/systems.h"

#include "engine/window/windowsdl2.h"
#include "nlohmann/json.hpp"

#include "sol/sol.hpp"
#include "ui/debuginfo.h"
#include "ui/performancewindow.h"
namespace yorcvs {
/**
 * @brief Main game class
 *
 */
class Application {
public:
    Application()
        : debug_info_widgets(this, &app_window, &map, &player_control, &map.collision_system, &map.health_system, &map.combat_system, &lua_state)
    {
        lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
        yorcvs::lua::bind_runtime(lua_state, &world);

        yorcvs::lua::register_system_to_lua(lua_state, "Health_system", map.health_system);
        yorcvs::lua::register_system_to_lua(lua_state, "Collision_system", map.collision_system);
        yorcvs::lua::register_system_to_lua(lua_state, "Animation_system", map.animation_system);
        yorcvs::lua::register_system_to_lua(lua_state, "Combat_system", map.combat_system, "attack",
            &CombatSystem::attack);
        lua_state["test_map"] = &map;
        // loading two maps one on top of each other
        lua_state.safe_script(R"(
            test_map:load_content("assets/map.tmx")
            local pl = test_map:load_entity_from_path(world:create_entity(),"assets/entities/test_player_1/test_player_1.json")
            world:add_playerMovementControl(pl)
            )");
        app_window.add_callback([&app_active = active](const yorcvs::event& e) {
            if (e.get_type() == yorcvs::Events::Type::WINDOW_QUIT) {
                app_active = false;
            }
        });
        counter.start();
    }
    Application(const Application& other) = delete;
    Application(Application&& other) = delete;
    Application& operator=(const Application& other) = delete;
    Application& operator=(Application&& other) = delete;

    void render_map_chunk(yorcvs::Map& p_map, const std::tuple<intmax_t, intmax_t>& chunk)
    {
        if (p_map.tiles_chunks.find(chunk) != p_map.tiles_chunks.end()) {
            const auto& tiles = p_map.tiles_chunks.at(chunk);
            for (const auto& tile : tiles) {
                app_window.draw_texture(tile.texture_path, { tile.coords.x, tile.coords.y, p_map.tilesSize.x, p_map.tilesSize.y },
                    tile.srcRect);
            }
        }
    }
    void render_map_tiles(yorcvs::Map& p_map)
    {
        yorcvs::Vec2<float> render_scale = app_window.get_render_scale();
        app_window.set_render_scale(app_window.get_window_size() / render_dimensions);
        // get player position
        if (player_control.entityList->entitiesID.empty()) {
            app_window.set_render_scale(render_scale); // set renderscale back
            return;
        }
        const size_t entity_ID = player_control.entityList->entitiesID[0];
        const yorcvs::Vec2<float> player_position = world.get_component<positionComponent>(entity_ID).position;
        const std::tuple<intmax_t, intmax_t> player_position_chunk = std::tuple<intmax_t, intmax_t>(
            std::floor(player_position.x / (32.0f * 16.0f)), std::floor(player_position.y / (32.0f * 16.0f)));
        // render chunks

        std::tuple<intmax_t, intmax_t> chunk_to_be_rendered {};
        for (intmax_t x = render_distance * -1; x <= render_distance; x++) {
            for (intmax_t y = -1 * render_distance; y <= render_distance; y++) {
                chunk_to_be_rendered = std::make_tuple<intmax_t, intmax_t>(std::get<0>(player_position_chunk) + x,
                    std::get<1>(player_position_chunk) + y);
                render_map_chunk(p_map, chunk_to_be_rendered);
            }
        }

        app_window.set_render_scale(render_scale);
    }
    void run()
    {
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        const float elapsed = std::min(100.0f, counter.get_ticks<float, std::chrono::nanoseconds>() / 1000000.0f);
        counter.stop();
        counter.start();

        lag += elapsed;
        app_window.handle_events();

        while (lag >= msPF) {
            update_loop_timer.start();
            debug_info_widgets.update(msPF, render_dimensions);
            player_control.updateControls(render_dimensions, msPF);

            update_timer.start();
            map.health_system.update(msPF);
            tracked_parameters[yorcvs::ui::Performance_Window::update_time_item::health] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            behaviour_system.update(msPF);
            tracked_parameters[yorcvs::ui::Performance_Window::update_time_item::behaviour] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            map.collision_system.update(msPF);
            tracked_parameters[yorcvs::ui::Performance_Window::update_time_item::collision] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            map.velocity_system.update(msPF);
            tracked_parameters[yorcvs::ui::Performance_Window::update_time_item::velocity] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            map.animation_system.update(msPF);
            tracked_parameters[yorcvs::ui::Performance_Window::update_time_item::animation] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            map.sprint_system.update(msPF);
            tracked_parameters[yorcvs::ui::Performance_Window::update_time_item::stamina] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            lag -= msPF;
            tracked_parameters[yorcvs::ui::Performance_Window::update_time_item::overall] = update_loop_timer.get_ticks<float, std::chrono::nanoseconds>();
            performance_window.record_update_time<yorcvs::ui::Performance_Window::update_time_item::health,
                yorcvs::ui::Performance_Window::update_time_item::behaviour,
                yorcvs::ui::Performance_Window::update_time_item::collision,
                yorcvs::ui::Performance_Window::update_time_item::velocity,
                yorcvs::ui::Performance_Window::update_time_item::animation,
                yorcvs::ui::Performance_Window::update_time_item::stamina,
                yorcvs::ui::Performance_Window::update_time_item::overall>(tracked_parameters);
        }
        app_window.clear();
        render_map_tiles(map);
        sprite_system.renderSprites(render_dimensions);
        debug_info_widgets.render(render_dimensions);
        if (debug_info_widgets.is_debug_window_open()) {
            performance_window.render();
        }
        ImGui::Render();
        ImGuiSDL::Render(ImGui::GetDrawData());
        app_window.present();
        debug_info_widgets.end_frame();
    }

    [[nodiscard]] bool is_active() const
    {
        return active;
    }

    ~Application() = default;

private:
    static constexpr yorcvs::Vec2<float> default_render_dimensions = { 240.0f, 120.0f };
    static constexpr float msPF = 41.6f;
    static constexpr intmax_t default_render_distance = 1;

    yorcvs::sdl2_window app_window;
    yorcvs::Timer counter;
    yorcvs::Timer update_timer;
    yorcvs::Timer update_loop_timer;

    float lag = 0.0f;
    yorcvs::Vec2<float> render_dimensions = default_render_dimensions; // how much to render
    intmax_t render_distance = default_render_distance;
    yorcvs::ECS world {};
    sol::state lua_state;
    yorcvs::Map map { &world };
    SpriteSystem sprite_system { map.ecs, &app_window };
    PlayerMovementControl player_control { map.ecs, &app_window };
    BehaviourSystem behaviour_system { map.ecs, &lua_state };

    yorcvs::ui::Performance_Window performance_window;
    std::array<float, yorcvs::ui::Performance_Window::update_time_item::update_time_tracked> tracked_parameters;

    DebugInfo debug_info_widgets;

    bool active = true;
};
} // namespace yorcvs
