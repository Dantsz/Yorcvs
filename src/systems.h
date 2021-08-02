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
        world->registerComponent<hitboxComponent>();
        world->registerComponent<positionComponent>();
 
        world->registerSystem<CollisionSystem>(*this);
        
        world->addCriteriaForIteration<CollisionSystem, positionComponent, hitboxComponent>();
    }
    void update(float dt, yorcvs::Window<yorcvs::SDL2>* testWindow) 
    {
        for(const auto& ID : entityList->entitiesID)
        {
            yorcvs::Vec2<float> pos = world->getComponent<positionComponent>(ID).position;
            pos += {world->getComponent<hitboxComponent>(ID).hitbox.x,world->getComponent<hitboxComponent>(ID).hitbox.y};
            yorcvs::Vec2<float> size = {world->getComponent<hitboxComponent>(ID).hitbox.w,world->getComponent<hitboxComponent>(ID).hitbox.h};
            testWindow->drawSprite("assets/lettuce.png",pos,size,{0,0,200,200});
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* world;
    
};
