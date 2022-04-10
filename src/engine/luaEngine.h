#pragma once
#include "../common/ecs.h"
#include "../game/components.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
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
    std::vector<std::string>& component_names = lua_state["impl"]["component_names"];
    yorcvs::ECS* ecs = lua_state["world"];
    sol::usertype<T> new_type = lua_state.new_usertype<T>(name, args...);
    lua_state["ECS"]["create_" + name] = [](){return T();} ;
    lua_state["ECS"]["add_" + name] = &yorcvs::ECS::add_default_component<T>;
    lua_state["ECS"]["get_" + name] = &yorcvs::ECS::get_component<T>;
    lua_state["ECS"]["has_" + name] = &yorcvs::ECS::has_components<T>;
    lua_state["ECS"]["remove_" + name] = &yorcvs::ECS::remove_component<T>;
    lua_state["ECS"]["component_ID" + name] = &yorcvs::ECS::get_component_ID<T>;
    const size_t index = ecs->get_component_ID<T>();
    if(component_names.size() < index)
    {
        component_names.resize(index + 1);
    }
    component_names.insert(component_names.begin() + index,name);
}   
/**
 * @brief Gives lua acces to basic engine types like vector and rectangle
 * 
 * @param lua_state 
 */
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
/**
 * @brief Gives the lua state accest to the running ECS and components
 * 
 * @param lua_state 
 * @param ecs 
 * 
 * 
 */
inline bool bind_runtime(sol::state &lua_state, yorcvs::ECS *ecs)
{
    lua_state["impl"] = lua_state.create_table_with("component_names",std::vector<std::string>{});
   
    bind_basic_types(lua_state);
    sol::usertype<yorcvs::ECS> lua_ECS = lua_state.new_usertype<yorcvs::ECS>("ECS");
    lua_state["world"] = ecs;
    lua_ECS["create_entity"] = &yorcvs::ECS::create_entity_ID;
    lua_ECS["is_valid_entity"] = &yorcvs::ECS::is_valid_entity;
    lua_ECS["destroy_entity"] = &yorcvs::ECS::destroy_entity;
    lua_ECS["get_active_entities"] = &yorcvs::ECS::get_active_entities_number;
    lua_ECS["get_entity_list_size"] = &yorcvs::ECS::get_entity_list_size;
    lua_ECS["get_entity_signature"] = &yorcvs::ECS::get_entity_signature;
    lua_state["ECS"]["component_name"] = [&](yorcvs::ECS* ecs,size_t ID)
    {
        std::vector<std::string>& names = lua_state["impl"]["component_names"];
        if(names.size() > ID)
        {
            return names[ID];
        }   
        return std::string{"null"};
    };
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