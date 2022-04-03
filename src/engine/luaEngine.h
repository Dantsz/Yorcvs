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
    lua_state["ECS"]["add_" + name] = &yorcvs::ECS::add_default_component<T>;
    lua_state["ECS"]["get_" + name] = &yorcvs::ECS::get_component<T>;
    lua_state["ECS"]["remove_" + name] = &yorcvs::ECS::remove_component<T>;
}
inline void bind_basic_types(sol::state &lua_state)
{
    sol::usertype<yorcvs::Vec2<float>> vec2f = lua_state.new_usertype<yorcvs::Vec2<float>>(
        "vec2f", "x", &yorcvs::Vec2<float>::x, "y", &yorcvs::Vec2<float>::y);
    sol::usertype<yorcvs::Vec2<bool>> vec2b = lua_state.new_usertype<yorcvs::Vec2<bool>>(
        "vec2b", "x", &yorcvs::Vec2<bool>::x, "y", &yorcvs::Vec2<bool>::y);
    sol::usertype<yorcvs::Rect<float>> rect2f = lua_state.new_usertype<yorcvs::Rect<float>>(
        "rect2f", "x", &yorcvs::Rect<float>::x, "y", &yorcvs::Rect<float>::y, "w", &yorcvs::Rect<float>::w, "h",
        &yorcvs::Rect<float>::h);
    sol::usertype<animationComponent::Animation> Animation = lua_state.new_usertype<animationComponent::Animation>("Animation","frames",&animationComponent::Animation::frames,
    "speed",&animationComponent::Animation::speed);
}
inline bool bind_runtime(sol::state &lua_state, yorcvs::ECS *ecs)
{
    bind_basic_types(lua_state);
    sol::usertype<yorcvs::ECS> lua_ECS = lua_state.new_usertype<yorcvs::ECS>("ECS");
    lua_state["world"] = ecs;
    lua_ECS["createEntity"] = &yorcvs::ECS::create_entity_ID;
    lua_ECS["isValidEntity"] = &yorcvs::ECS::is_valid_entity;
    lua_ECS["destroyEntity"] = &yorcvs::ECS::destroy_entity;
    register_component_to_lua<healthComponent>(lua_state, "healthComponent", "HP", &healthComponent::HP, "max_HP",
                                               &healthComponent::max_HP, "health_regen",
                                               &healthComponent::health_regen);
    register_component_to_lua<identificationComponent>(lua_state, "identificationComponent", "name",
                                                       &identificationComponent::name);
    register_component_to_lua<hitboxComponent>(lua_state, "hitboxComponent", "hitbox", &hitboxComponent::hitbox);
    register_component_to_lua<positionComponent>(lua_state, "positionComponent", "position",
                                                 &positionComponent::position);
    register_component_to_lua<velocityComponent>(lua_state, "velocityComponent", "vel",&velocityComponent::vel,"facing",&velocityComponent::facing);
    register_component_to_lua<staminaComponent>(lua_state, "staminaComponent", "stamina", &staminaComponent::stamina,
                                                "max_stamina", &staminaComponent::max_stamina, "stamina_regen",
                                                &staminaComponent::stamina_regen);
    register_component_to_lua<spriteComponent>(lua_state, "spriteComponent", "offset", &spriteComponent::offset, "size",
                                               &spriteComponent::size, "src_rect", &spriteComponent::src_rect,
                                               "texture_path", &spriteComponent::texture_path);
    register_component_to_lua<animationComponent>(lua_state, "animationComponent","animations", &animationComponent::animations,
                                                 "cur_animation", &animationComponent::cur_animation,"cur_frame",&animationComponent::cur_frame,"cur_elapsed",&animationComponent::cur_elapsed);

    return true;
}

} // namespace yorcvs::lua