local chicken_speed = 0.033;
local velx = math.random() - 0.5
local vely = math.random() - 0.5
world:get_velocityComponent(entityID).vel.x = velx * chicken_speed
world:get_velocityComponent(entityID).vel.y = vely * chicken_speed
if (world:get_velocityComponent(entityID).vel.x > 0) then
    world:get_animationComponent(entityID).cur_animation = "walkingL"
elseif (world:get_velocityComponent(entityID).vel.x < 0) then
    world:get_animationComponent(entityID).cur_animation = "walkingR"
elseif(world:get_velocityComponent(entityID).facing.x == true) then
    world:get_animationComponent(entityID).cur_animation = "idleL"
else
    world:get_animationComponent(entityID).cur_animation = "idleR"
end
