#pragma once
#include "components.h"
#include "ecs/ecs.h"
#include "window/windowSDL2.h"
class CollisionSystem
{
    public:
    CollisionSystem(yorcvs::ECS* parent)
    {
        world = parent;
        //is this legal?
        world->register_component<hitboxComponent>();
        world->register_component<positionComponent>();
 
        world->register_system<CollisionSystem>(*this);
        
        world->add_criteria_for_iteration<CollisionSystem, positionComponent, hitboxComponent>();
    }
    void update(float elapsedTime)
    {
        
    }

    void render(float dt, yorcvs::Window<yorcvs::SDL2>* testWindow) 
    {
        for(const auto& ID : entityList->entitiesID)
        {
            yorcvs::Vec2<float> pos = world->get_component<positionComponent>(ID).position;
            pos += {world->get_component<hitboxComponent>(ID).hitbox.x,world->get_component<hitboxComponent>(ID).hitbox.y};
            yorcvs::Vec2<float> size = {world->get_component<hitboxComponent>(ID).hitbox.w,world->get_component<hitboxComponent>(ID).hitbox.h};
            testWindow->draw_sprite("assets/lettuce.png",pos,size,{0,0,200,200});
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* world;
    
};
