#pragma once
#include "sol/sol.hpp"
#include "../common/ecs.h"
namespace yorcvs
{
    class LuaEngine 
    {
        public:
        LuaEngine() = delete;
        LuaEngine(yorcvs::ECS* ecs) : world(ecs)
        {

        };
        LuaEngine(const LuaEngine&) = delete;
        LuaEngine& operator=(const LuaEngine&) = delete;
        LuaEngine(LuaEngine&&) = delete;
        LuaEngine& operator=(LuaEngine&&) = delete;

                


        private:
        yorcvs::ECS* world;
        sol::state lua_state;
    };
}