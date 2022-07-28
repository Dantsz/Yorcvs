animation_system:set_animation(entityID,"idleR")
if(world:get_healthComponent(entityID).HP < world:get_healthComponent(entityID).max_HP/2) then
    world:remove_healthComponent(entityID)
    world:remove_behaviourComponent(entityID)
    animation_system:set_animation(entityID,"idle_broken")
end