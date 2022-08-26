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

#include "common/ecs.h"
#include "common/types.h"
#include "engine/luaEngine.h"
#include "engine/map.h"
#include "game/components.h"

#include "engine/window/windowsdl2.h"
#include "ui/debuginfo.h"
#include "ui/entityinteraction.h"
#include "ui/performancewindow.h"
namespace yorcvs {
/**
 * @brief Main game class
 *
 */
class application {
public:
    application()
        : debug_info_widgets(this, &app_window, &map, &player_control, &map.collision_sys, &map.health_sys, &map.combat_sys, &lua_state)
        , entity_inter_widget(app_window, app_window, world, map.collision_sys, map.combat_sys, player_control)
    {
        lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
        yorcvs::lua::bind_runtime(lua_state, &world);

        yorcvs::lua::register_system_to_lua(lua_state, "health_system", map.health_sys);
        yorcvs::lua::register_system_to_lua(lua_state, "collision_system", map.collision_sys);
        yorcvs::lua::register_system_to_lua(lua_state, "animation_system", map.animation_sys, "set_animation", &animation_system::set_animation);
        yorcvs::lua::register_system_to_lua(lua_state, "combat_system", map.combat_sys, "attack",
            &combat_system::attack);
        lua_state["test_map"] = &map;
        // loading two maps one on top of each other
        // test_map:load_content("assets/map.tmx")
        lua_state.safe_script(R"(        
            test_map:load_content("assets/map.tmx")
            local pl = test_map:load_character_from_path(world:create_entity(),"assets/entities/test_player_2/test_player_2.json")
            world:add_playerMovementControl(pl)
            )");
        [[maybe_unused]] const auto callback_id = app_window.add_callback_on_event(yorcvs::Events::Type::WINDOW_QUIT, [&app_active = active](const yorcvs::event&) { app_active = false; });
        counter.start();
    }
    application(const application& other) = delete;
    application(application&& other) = delete;
    application& operator=(const application& other) = delete;
    application& operator=(application&& other) = delete;

    void render_map_chunk(yorcvs::map& p_map, const std::tuple<intmax_t, intmax_t>& chunk)
    {
        if (p_map.tiles_chunks.find(chunk) != p_map.tiles_chunks.end()) {
            const auto& tiles = p_map.tiles_chunks.at(chunk);
            for (const auto& tile : tiles) {
                app_window.draw_texture(tile.texture_path, { tile.coords.x, tile.coords.y, p_map.tilesSize.x, p_map.tilesSize.y },
                    tile.srcRect);
            }
        }
    }
    void render_map_tiles(yorcvs::map& p_map)
    {
        yorcvs::vec2<float> render_scale = app_window.get_render_scale();
        app_window.set_render_scale(app_window.get_window_size() / render_dimensions);
        // get player position
        if (player_control.entityList->empty()) {
            app_window.set_render_scale(render_scale); // set renderscale back
            return;
        }
        const size_t entity_ID = (*player_control.entityList)[0];
        const yorcvs::vec2<float> player_position = world.get_component<position_component>(entity_ID).position;
        const std::tuple<intmax_t, intmax_t> player_position_chunk = std::make_tuple(
            static_cast<intmax_t>(std::floor(player_position.x / (32.0f * 16.0f))), static_cast<intmax_t>(std::floor(player_position.y / (32.0f * 16.0f))));
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
            map.health_sys.update(msPF);
            tracked_parameters[yorcvs::ui::performance_window::update_time_item::health] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            behaviour_sys.update(msPF);
            tracked_parameters[yorcvs::ui::performance_window::update_time_item::behaviour] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            map.collision_sys.update(msPF);
            tracked_parameters[yorcvs::ui::performance_window::update_time_item::collision] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            map.velocity_sys.update(msPF);
            tracked_parameters[yorcvs::ui::performance_window::update_time_item::velocity] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            map.animation_sys.update(msPF);
            tracked_parameters[yorcvs::ui::performance_window::update_time_item::animation] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            update_timer.start();
            map.sprint_sys.update(msPF);
            tracked_parameters[yorcvs::ui::performance_window::update_time_item::stamina] = update_timer.get_ticks<float, std::chrono::nanoseconds>();

            lag -= msPF;
            tracked_parameters[yorcvs::ui::performance_window::update_time_item::overall] = update_loop_timer.get_ticks<float, std::chrono::nanoseconds>();
            performance_widget.record_update_time<yorcvs::ui::performance_window::update_time_item::health,
                yorcvs::ui::performance_window::update_time_item::behaviour,
                yorcvs::ui::performance_window::update_time_item::collision,
                yorcvs::ui::performance_window::update_time_item::velocity,
                yorcvs::ui::performance_window::update_time_item::animation,
                yorcvs::ui::performance_window::update_time_item::stamina,
                yorcvs::ui::performance_window::update_time_item::overall>(tracked_parameters);
        }
        app_window.clear();
        render_map_tiles(map);
        sprite_sys.renderSprites(render_dimensions);
        debug_info_widgets.render(render_dimensions);
        entity_inter_widget.render(render_dimensions);
        if (debug_info_widgets.is_debug_window_open()) {
            performance_widget.render();
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

    ~application() = default;

private:
    static constexpr yorcvs::vec2<float> default_render_dimensions = { 240.0f, 120.0f };
    static constexpr float msPF = 41.6f;
    static constexpr intmax_t default_render_distance = 1;

    yorcvs::sdl2_window app_window;
    yorcvs::timer counter;
    yorcvs::timer update_timer;
    yorcvs::timer update_loop_timer;

    float lag = 0.0f;
    yorcvs::vec2<float> render_dimensions = default_render_dimensions; // how much to render
    intmax_t render_distance = default_render_distance;
    yorcvs::ECS world {};
    sol::state lua_state;
    yorcvs::map map { &world };
    sprite_system sprite_sys { map.ecs, &app_window };
    player_movement_control player_control { map.ecs, &app_window };
    behaviour_system behaviour_sys { map.ecs, &lua_state };

    yorcvs::ui::performance_window performance_widget;
    std::array<float, yorcvs::ui::performance_window::update_time_item::update_time_tracked> tracked_parameters;

    debug_info debug_info_widgets;
    entity_interaction_widget<yorcvs::eventhandler_sdl2, yorcvs::sdl2_window> entity_inter_widget;
    bool active = true;
};
} // namespace yorcvs
