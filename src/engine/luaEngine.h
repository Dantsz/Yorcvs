#pragma once
#include "sol/forward.hpp"
#include "sol/sol.hpp"
#include "../common/ecs.h"
#include "sol/state.hpp"
namespace yorcvs::lua
{   
    inline bool bind_runtime(sol::state & lua_state, yorcvs::ECS* ecs)
    {
       
        sol::usertype<yorcvs::ECS> lua_ECS = lua_state.new_usertype<yorcvs::ECS>("ECS");
        lua_state["world"] = ecs;
        lua_ECS["createEntity"] = &yorcvs::ECS::create_entity_ID;
        lua_ECS["isValidEntity"] = &yorcvs::ECS::is_valid_entity;
        lua_ECS["destroyEntity"] = &yorcvs::ECS::destroy_entity;
        return true;
    }
}