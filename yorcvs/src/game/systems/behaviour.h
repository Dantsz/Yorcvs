#pragma once
#include "../../common/assetmanager.h"
#include "../../common/ecs.h"
#include "../components.h"
#include "sol/sol.hpp"
#include "sol/types.hpp"
#include <fstream>
/**
 * @brief Handles behaviour of non-player entities.
 *
 */
class behaviour_system {
public:
    behaviour_system(yorcvs::ECS* parent, sol::state* lua)
        : world(parent)
        , lua_state(lua)
    {
        world->register_system<behaviour_system>(*this); // registers itself
        world->add_criteria_for_iteration<behaviour_system, behaviour_component, velocity_component>();
        scripts = std::make_unique<yorcvs::asset_manager<std::string>>(
            [&](const std::string& path) {
                auto program = std::make_shared<std::string>();
                std::ifstream in { path };
                program->assign((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
                return program;
            },
            [&](std::string* str) { delete str; }); // creates an asset manager that manages lua programs from path
    }

    void run_behaviour(const size_t ID) const
    {
        (*lua_state)["entityID"] = ID;
        const std::string& script_path = world->get_component<behaviour_component>(ID).code_path;
        world->get_component<behaviour_component>(ID).accumulated = 0.0f;
        lua_state->safe_script(*scripts->load_from_file(script_path));
    }
    void update(const float dt)
    {
        for (const auto ID : *entityList) {
            world->get_component<behaviour_component>(ID).accumulated += dt;

            if (world->get_component<behaviour_component>(ID).accumulated > world->get_component<behaviour_component>(ID).dt) {
                run_behaviour(ID);
            }
        }
    }
    std::shared_ptr<yorcvs::entity_system_list> entityList = nullptr;
    yorcvs::ECS* world = nullptr;
    std::unique_ptr<yorcvs::asset_manager<std::string>> scripts;
    sol::state* lua_state;
    static constexpr float velocity_trigger_treshold = 0.0f;
};
