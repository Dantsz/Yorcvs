#pragma once
#include "../common/ecs.h"
#include "../game/components.h"
#include "sol/forward.hpp"
#include "sol/sol.hpp"
#include "sol/state.hpp"

namespace yorcvs::lua
{
/**
 * @brief Exposes type to lua and creates a methods for the ECS
 *
 * @tparam T
 * @param lua_state
 * @param name
 */
template <typename T, typename... Args>
inline void register_component_to_lua(sol::state &lua_state, const std::string &name, Args &&... args)
{
    sol::usertype<T> new_type = lua_state.new_usertype<T>(name, args...);
    lua_state["ECS"]["get_" + name] = &yorcvs::ECS::get_component<T>;
    lua_state["ECS"]["remove_" + name] = &yorcvs::ECS::remove_component<T>;
}
inline void bind_basic_types(sol::state &lua_state)
{
    sol::usertype<yorcvs::Vec2<float>> vec2f = lua_state.new_usertype<yorcvs::Vec2<float>>(
        "vec2f", "x", &yorcvs::Vec2<float>::x, "y", &yorcvs::Vec2<float>::y);
    sol::usertype<yorcvs::Rect<float>> rect2f =
        lua_state.new_usertype<yorcvs::Rect<float>>("rect2f", "x", &yorcvs::Rect<float>::x, "y", &yorcvs::Rect<float>::y, "w",
                                           &yorcvs::Rect<float>::w, "h", &yorcvs::Rect<float>::h);
}
inline bool bind_runtime(sol::state &lua_state, yorcvs::ECS *ecs)
{
    bind_basic_types(lua_state);
    sol::usertype<yorcvs::ECS> lua_ECS = lua_state.new_usertype<yorcvs::ECS>("ECS");
    lua_state["world"] = ecs;
    lua_ECS["createEntity"] = &yorcvs::ECS::create_entity_ID;
    lua_ECS["isValidEntity"] = &yorcvs::ECS::is_valid_entity;
    lua_ECS["destroyEntity"] = &yorcvs::ECS::destroy_entity;
    register_component_to_lua<healthComponent>(lua_state, "healthComponent", "HP", &healthComponent::HP, "maxHP",
                                               &healthComponent::maxHP, "health_regen", &healthComponent::health_regen);
    register_component_to_lua<identificationComponent>(lua_state,"identificationComponent", "name", &identificationComponent::name);
    register_component_to_lua<hitboxComponent>(lua_state,"hitboxComponent", "hitbox", &hitboxComponent::hitbox);
    register_component_to_lua<positionComponent>(lua_state,"positionComponent", "position", &positionComponent::position);
    return true;
}

} // namespace yorcvs::lua