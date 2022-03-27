#pragma once
#include "sol/sol.hpp"
#include "../common/ecs.h"
#include "sol/state.hpp"
namespace yorcvs::lua
{   
    inline bool bind_runtime(sol::state & lua_state, yorcvs::ECS* ecs)
    {
        return true;
    }
}