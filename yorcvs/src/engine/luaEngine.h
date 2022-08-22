#pragma once
#include "../common/ecs.h"
#include "../game/components.h"
#include "map.h"
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#include "sol/forward.hpp"
#include "sol/sol.hpp"
#include "sol/state.hpp"

namespace yorcvs::lua {
/**
 * @brief Exposes type to lua and creates a methods for the ECS
 *
 * @tparam T
 * @param lua_state
 * @param name
 * @param args pairs made of a string which is the name sued by lua  and the parameter of the component
 */
template <typename T, typename... Args>
inline void register_component_to_lua(sol::state& lua_state, const std::string& name, Args&&... args)
{
    yorcvs::ECS* ecs = lua_state["world"];
    const auto index = ecs->get_component_ID<T>();
    if (!index.has_value()) {
        yorcvs::log("Cannot register the component " + name + " as it's not registered by the ECS", yorcvs::MSGSEVERITY::ERROR);
    }
    std::vector<std::string>& component_names = lua_state["impl"]["component_names"];

    sol::usertype<T> new_type = lua_state.new_usertype<T>(name, std::forward<Args>(args)...);
    lua_state["ECS"]["create_" + name] = []() { return T(); };
    lua_state["ECS"]["add_" + name] = &yorcvs::ECS::add_default_component<T>;
    lua_state["ECS"]["get_" + name] = &yorcvs::ECS::get_component<T>;
    lua_state["ECS"]["has_" + name] = &yorcvs::ECS::has_components<T>;
    lua_state["ECS"]["remove_" + name] = &yorcvs::ECS::remove_component<T>;
    lua_state["ECS"]["component_ID" + name] = &yorcvs::ECS::get_component_ID<T>;

    if (component_names.size() < index.value()) {
        component_names.resize(index.value() + 1, "null");
    }
    component_names.insert(component_names.begin() + index.value(), name);
}
/**
 * @brief Exposes a system to lua
 *
 */
template <systemT T, typename... Args>
inline void register_system_to_lua(sol::state& lua_state, const std::string& name, T& system, Args&&... args)
{
    const std::string system_type_name = name + "_t"; // name of the system metatable
    sol::usertype<T> system_type = lua_state.new_usertype<T>(
        system_type_name,
        "entities", &T::entityList,
        std::forward<Args>(args)...); // register the system type

    lua_state[name] = system; // set the instance of system used
}
/**
 * @brief Gives lua acces to basic engine types like vector and rectangle
 *
 * @param lua_state
 */
inline void bind_basic_types(sol::state& lua_state)
{
    sol::usertype<yorcvs::vec2<float>> vec2f = lua_state.new_usertype<yorcvs::vec2<float>>(
        "vec2f", "x", &yorcvs::vec2<float>::x, "y", &yorcvs::vec2<float>::y);
    sol::usertype<yorcvs::vec2<bool>> vec2b = lua_state.new_usertype<yorcvs::vec2<bool>>("vec2b", "x", &yorcvs::vec2<bool>::x, "y", &yorcvs::vec2<bool>::y);
    sol::usertype<yorcvs::rect<float>> rect2f = lua_state.new_usertype<yorcvs::rect<float>>(
        "rect2f", "x", &yorcvs::rect<float>::x, "y", &yorcvs::rect<float>::y, "w", &yorcvs::rect<float>::w, "h",
        &yorcvs::rect<float>::h);
}
inline void bind_map_functions(sol::state& lua_state)
{
    sol::usertype<yorcvs::map> map_type = lua_state.new_usertype<yorcvs::map>("Map");
    // lua_state["create_map"] = []() {};
    lua_state["Map"]["load_content"] = [](yorcvs::map& map, const std::string& path) // loads alll tiles and object from the path to the map
    {
        map.load(map.ecs, path);
    };
    lua_state["Map"]["load_entity_from_string"] = [](yorcvs::map& map, const size_t entityID, const std::string& path) { // creates a new entity and assigns components from the file
        map.load_entity_from_path(entityID, path);
        return entityID;
    };
    lua_state["Map"]["load_entity_from_path"] = [](yorcvs::map& map, const size_t entityID, const std::string& path) { // creates a new entity and assigns components from the file
        map.load_entity_from_path(entityID, path);
        return entityID;
    };
    lua_state["Map"]["save_entity"] = [](yorcvs::map& map, const size_t ID) {
        return map.save_entity(ID);
    };
}
inline void bind_system_entity_list(sol::state& lua_state)
{
    sol::usertype<entity_system_list> entsl = lua_state.new_usertype<entity_system_list>("EntitySystemList");
}
/**
 * @brief Gives the lua state accest to the running ECS and components
 *
 * @param lua_state
 * @param ecs
 *
 *
 */
inline bool bind_runtime(sol::state& lua_state, yorcvs::ECS* ecs)
{
    lua_state["impl"] = lua_state.create_table_with("component_names", std::vector<std::string> {});
    lua_state["run_script"] = [&](const std::string& path) { lua_state.safe_script_file(path); };
    bind_basic_types(lua_state);
    bind_map_functions(lua_state);
    sol::usertype<yorcvs::ECS> lua_ECS = lua_state.new_usertype<yorcvs::ECS>("ECS");
    lua_state["world"] = ecs;
    lua_ECS["create_entity"] = &yorcvs::ECS::create_entity_ID;
    lua_ECS["is_valid_entity"] = &yorcvs::ECS::is_valid_entity;
    lua_ECS["destroy_entity"] = &yorcvs::ECS::destroy_entity;
    lua_ECS["get_active_entities"] = &yorcvs::ECS::get_active_entities_number;
    lua_ECS["get_entity_list_size"] = &yorcvs::ECS::get_entity_list_size;
    lua_ECS["get_entity_signature"] = &yorcvs::ECS::get_entity_signature;
    lua_ECS["copy_components_to_from_entity"] = &yorcvs::ECS::copy_components_to_from_entity;
    // returns the components name based on it's ID
    lua_state["ECS"]["component_name"] = [&](yorcvs::ECS*, size_t ID) {
        std::vector<std::string>& names = lua_state["impl"]["component_names"];
        if (names.size() > ID) {
            return names[ID];
        }
        return std::string { "null" };
    };
    // returns the number of registered components by the ECS that can be used in lua scripts
    lua_state["ECS"]["components_registered"] = [&](yorcvs::ECS*) {
        std::vector<std::string>& names = lua_state["impl"]["component_names"];
        return names.size();
    };
    bind_system_entity_list(lua_state);
    register_component_to_lua<health_component>(lua_state, "healthComponent",
        "HP", &health_component::HP);
    register_component_to_lua<health_stats_component>(lua_state, "healthStatsComponent",
        "max_HP", &health_stats_component::max_HP,
        "health_regen", &health_stats_component::health_regen);
    register_component_to_lua<identification_component>(lua_state, "identificationComponent",
        "name", &identification_component::name);
    register_component_to_lua<hitbox_component>(lua_state,
        "hitboxComponent",
        "hitbox", &hitbox_component::hitbox);
    register_component_to_lua<position_component>(lua_state, "positionComponent",
        "position", &position_component::position);
    register_component_to_lua<velocity_component>(lua_state, "velocityComponent",
        "vel", &velocity_component::vel,
        "facing", &velocity_component::facing);
    register_component_to_lua<stamina_component>(lua_state, "staminaComponent",
        "stamina", &stamina_component::stamina);
    register_component_to_lua<stamina_stats_component>(lua_state, "staminaStatsComponent",
        "max_stamina", &stamina_stats_component::max_stamina,
        "stamina_regen", &stamina_stats_component::stamina_regen);
    register_component_to_lua<sprite_component>(lua_state, "spriteComponent",
        "offset", &sprite_component::offset,
        "size", &sprite_component::size,
        "src_rect", &sprite_component::src_rect,
        "texture_path", &sprite_component::texture_path);
    register_component_to_lua<animation_component>(
        lua_state, "animationComponent",
        "animations", &animation_component::animation_name_to_start_frame_index,
        "cur_frame", &animation_component::current_frame, "frames", &animation_component::frames);
    register_component_to_lua<defensive_stats_component>(
        lua_state, "defensiveStatsComponent",
        "defense", &defensive_stats_component::defense,
        "block", &defensive_stats_component::block,
        "dodge", &defensive_stats_component::dodge,
        "spirit", &defensive_stats_component::spirit);
    register_component_to_lua<offensive_stats_component>(
        lua_state, "offensiveStatsComponent",
        "strength", &offensive_stats_component::strength,
        "agility", &offensive_stats_component::agility,
        "dexterity", &offensive_stats_component::dexterity,
        "piercing", &offensive_stats_component::piercing,
        "intellect", &offensive_stats_component::intellect);
    register_component_to_lua<player_movement_controlled_component>(lua_state, "playerMovementControl");
    register_component_to_lua<behaviour_component>(lua_state, "behaviourComponent", &behaviour_component::code_path, "code_path");
    return true;
}

} // namespace yorcvs::lua
namespace sol {
template <>
struct is_container<yorcvs::entity_system_list> : std::true_type {
};
}
