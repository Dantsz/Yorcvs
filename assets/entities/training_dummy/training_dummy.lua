world:get_animationComponent(entityID).cur_animation = "idleR"

if(world:get_healthComponent(entityID).HP < world:get_healthComponent(entityID).max_HP/2) then
    world:remove_healthComponent(entityID)
    world:remove_behaviourComponent(entityID)
    world:get_animationComponent(entityID).cur_animation = "idle_broken"
end
