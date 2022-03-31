#pragma once
#include "sol/forward.hpp"
#include "sol/sol.hpp"
#include "../common/ecs.h"
#include "sol/state.hpp"
#include "../game/components.h"
struct Doge {
	int tailwag = 50;

	Doge() {
	}

	Doge(int wags)
		: tailwag(wags) {
	}

	~Doge() {
		std::cout << "Dog at " << this << " is being destroyed..." << std::endl;
	}
};

namespace yorcvs::lua
{   
     /**
     * @brief Exposes type to lua and creates a methods for the ECS
     * 
     * @tparam T 
     * @param lua_state 
     * @param name 
     */
    template<typename T,typename... Args>
    inline void register_component_to_lua(sol::state & lua_state,const std::string& name,Args &&...args)
    {
        sol::usertype<T> new_type = lua_state.new_usertype<T>(name,args...);
        lua_state["ECS"]["get_" + name] = &yorcvs::ECS::get_component<T>;
        lua_state["ECS"]["remove_"+name] = &yorcvs::ECS::remove_component<T>;
    }
    inline bool bind_runtime(sol::state & lua_state, yorcvs::ECS* ecs)
    {
       
        sol::usertype<yorcvs::ECS> lua_ECS = lua_state.new_usertype<yorcvs::ECS>("ECS");
        lua_state["world"] = ecs;
        lua_ECS["createEntity"] = &yorcvs::ECS::create_entity_ID;
        lua_ECS["isValidEntity"] = &yorcvs::ECS::is_valid_entity;
        lua_ECS["destroyEntity"] = &yorcvs::ECS::destroy_entity;
        register_component_to_lua<healthComponent>(lua_state,"healthComponent","HP",&healthComponent::HP,"maxHP",&healthComponent::maxHP,"health_regen",&healthComponent::health_regen);
        
        
        return true;
    }

}