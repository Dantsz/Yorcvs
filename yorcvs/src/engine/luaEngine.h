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
    sol::usertype<yorcvs::Vec2<float>> vec2f = lua_state.new_usertype<yorcvs::Vec2<float>>(
        "vec2f", "x", &yorcvs::Vec2<float>::x, "y", &yorcvs::Vec2<float>::y);
    sol::usertype<yorcvs::Vec2<bool>> vec2b = lua_state.new_usertype<yorcvs::Vec2<bool>>("vec2b", "x", &yorcvs::Vec2<bool>::x, "y", &yorcvs::Vec2<bool>::y);
    sol::usertype<yorcvs::Rect<float>> rect2f = lua_state.new_usertype<yorcvs::Rect<float>>(
        "rect2f", "x", &yorcvs::Rect<float>::x, "y", &yorcvs::Rect<float>::y, "w", &yorcvs::Rect<float>::w, "h",
        &yorcvs::Rect<float>::h);
}
inline void bind_map_functions(sol::state& lua_state)
{
    sol::usertype<yorcvs::Map> map_type = lua_state.new_usertype<yorcvs::Map>("Map");
    // lua_state["create_map"] = []() {};
    lua_state["Map"]["load_content"] = [](yorcvs::Map& map, const std::string& path) // loads alll tiles and object from the path to the map
    {
        map.load(map.ecs, path);
    };
    lua_state["Map"]["load_entity_from_string"] = [](yorcvs::Map& map, const size_t entityID, const std::string& path) { // creates a new entity and assigns components from the file
        map.load_character_from_path(entityID, path);
        return entityID;
    };
    lua_state["Map"]["load_entity_from_path"] = [](yorcvs::Map& map, const size_t entityID, const std::string& path) { // creates a new entity and assigns components from the file
        map.load_character_from_path(entityID, path);
        return entityID;
    };
    lua_state["Map"]["save_entity"] = [](yorcvs::Map& map, const size_t ID) {
        return map.save_character(ID);
    };
}
inline void bind_system_entity_list(sol::state& lua_state)
{
    sol::usertype<EntitySystemList> entsl = lua_state.new_usertype<EntitySystemList>("EntitySystemList");
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
    register_component_to_lua<healthComponent>(lua_state, "healthComponent",
        "HP", &healthComponent::HP);
    register_component_to_lua<healthStatsComponent>(lua_state, "healthStatsComponent",
        "max_HP", &healthStatsComponent::max_HP,
        "health_regen", &healthStatsComponent::health_regen);
    register_component_to_lua<identificationComponent>(lua_state, "identificationComponent",
        "name", &identificationComponent::name);
    register_component_to_lua<hitboxComponent>(lua_state,
        "hitboxComponent",
        "hitbox", &hitboxComponent::hitbox);
    register_component_to_lua<positionComponent>(lua_state, "positionComponent",
        "position", &positionComponent::position);
    register_component_to_lua<velocityComponent>(lua_state, "velocityComponent",
        "vel", &velocityComponent::vel,
        "facing", &velocityComponent::facing);
    register_component_to_lua<staminaComponent>(lua_state, "staminaComponent",
        "stamina", &staminaComponent::stamina);
    register_component_to_lua<staminaStatsComponent>(lua_state, "staminaStatsComponent",
        "max_stamina", &staminaStatsComponent::max_stamina,
        "stamina_regen", &staminaStatsComponent::stamina_regen);
    register_component_to_lua<spriteComponent>(lua_state, "spriteComponent",
        "offset", &spriteComponent::offset,
        "size", &spriteComponent::size,
        "src_rect", &spriteComponent::src_rect,
        "texture_path", &spriteComponent::texture_path);
    register_component_to_lua<animationComponent>(
        lua_state, "animationComponent",
        "animations", &animationComponent::animation_name_to_start_frame_index,
        "cur_frame", &animationComponent::current_frame, "frames", &animationComponent::frames);
    register_component_to_lua<defensiveStatsComponent>(
        lua_state, "defensiveStatsComponent",
        "defense", &defensiveStatsComponent::defense,
        "block", &defensiveStatsComponent::block,
        "dodge", &defensiveStatsComponent::dodge,
        "spirit", &defensiveStatsComponent::spirit);
    register_component_to_lua<offensiveStatsComponent>(
        lua_state, "offensiveStatsComponent",
        "strength", &offensiveStatsComponent::strength,
        "agility", &offensiveStatsComponent::agility,
        "dexterity", &offensiveStatsComponent::dexterity,
        "piercing", &offensiveStatsComponent::piercing,
        "intellect", &offensiveStatsComponent::intellect);
    register_component_to_lua<playerMovementControlledComponent>(lua_state, "playerMovementControl");
    register_component_to_lua<behaviourComponent>(lua_state, "behaviourComponent", &behaviourComponent::code_path, "code_path");
    return true;
}

} // namespace yorcvs::lua
namespace sol {
template <>
struct is_container<yorcvs::EntitySystemList> : std::true_type {
};
}
