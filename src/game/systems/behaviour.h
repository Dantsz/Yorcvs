#pragma once
#include "../../common/ecs.h"
#include "../../common/assetmanager.h"
#include "../components.h"
#include "sol/sol.hpp"
#include "sol/types.hpp"
#include <fstream>
/**
 * @brief Handles behaviour of non-player entities.
 *
 */
class BehaviourSystem
{
  public:
    BehaviourSystem(yorcvs::ECS *parent, sol::state *lua) : world(parent), lua_state(lua)
    {
        world->register_system<BehaviourSystem>(*this); // registers itself
        world->add_criteria_for_iteration<BehaviourSystem, behaviourComponent, velocityComponent>();
        scripts = std::make_unique<yorcvs::AssetManager<std::string>>(
            [&](const std::string &path) {
                std::string *program = new std::string();
                std::ifstream in{path};
                program->assign((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
                return program;
            },
            [&](std::string *str) { delete str; }); //creates an asset manager that manages lua programs from path
    }

    void run_behaviour(const size_t ID)
    {
        (*lua_state)["entityID"] = ID;
        const std::string &script_path = world->get_component<behaviourComponent>(ID).code_path;

        lua_state->safe_script(*scripts->load_from_file(script_path));

        world->get_component<behaviourComponent>(ID).accumulated = 0.0f;
    }
    void update(const float dt)
    {
        for (const auto ID : entityList->entitiesID)
        {
            world->get_component<behaviourComponent>(ID).accumulated += dt;

            if (world->get_component<behaviourComponent>(ID).accumulated >
                world->get_component<behaviourComponent>(ID).dt)
            {
                run_behaviour(ID);
            }
        }
    }
    std::shared_ptr<yorcvs::EntitySystemList> entityList = nullptr;
    yorcvs::ECS *world = nullptr;
    std::unique_ptr<yorcvs::AssetManager<std::string>> scripts;
    sol::state *lua_state;
    static constexpr float velocity_trigger_treshold = 0.0f;
};
