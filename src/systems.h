#pragma once
#include "components.h"
#include "ecs/ecs.h"
#include "window/windowSDL2.h"
#include <array>
class CollisionSystem
{
  public:
    CollisionSystem(yorcvs::ECS *parent)
    {
        world = parent;
        // is this legal? yee
        if (!world->is_component_registered<hitboxComponent>())
        {
            world->register_component<hitboxComponent>();
        }
        if (!world->is_component_registered<positionComponent>())
        {
            world->register_component<positionComponent>();
        }

        world->register_system<CollisionSystem>(*this);

        world->add_criteria_for_iteration<CollisionSystem, positionComponent, hitboxComponent>();
    }

    void update(float elapsedTime) const // checks and resolves collisions
    {
        yorcvs::Rect<float> rectA{};
        yorcvs::Rect<float> rectB{};

        for (const auto &IDA : entityList->entitiesID)
        {
            if (world->has_components<velocityComponent>(IDA))
            {
                std::array<bool, 4> collisions{false, false, false, false};
                rectA.x = world->get_component<positionComponent>(IDA).position.x;
                rectA.y = world->get_component<positionComponent>(IDA).position.y;
                rectA.w = world->get_component<hitboxComponent>(IDA).hitbox.w;
                rectA.h = world->get_component<hitboxComponent>(IDA).hitbox.h;
                yorcvs::Vec2<float> &rectAvel = world->get_component<velocityComponent>(IDA).vel;
                for (const auto &IDB : entityList->entitiesID)
                {

                    rectB.x = world->get_component<positionComponent>(IDB).position.x;
                    rectB.y = world->get_component<positionComponent>(IDB).position.y;
                    rectB.w = world->get_component<hitboxComponent>(IDB).hitbox.w;
                    rectB.h = world->get_component<hitboxComponent>(IDB).hitbox.h;
                    if (IDA != IDB)
                    {
                        // left to right
                        if (rectA.x + rectA.w <= rectB.x && rectA.x + rectA.w + (rectAvel.x * elapsedTime) > rectB.x &&
                            rectA.y + rectA.h > rectB.y && rectA.y - rectB.y < rectB.h)
                        {
                            rectAvel.x = (rectB.x - rectA.x - rectA.w) / elapsedTime;
                            collisions[0] = true;
                        }
                        // right to left
                        if (rectA.x >= rectB.x + rectB.w && rectA.x + (rectAvel.x * elapsedTime) < rectB.x + rectB.w &&
                            rectA.y + rectA.h > rectB.y && rectA.y - rectB.y < rectB.h)
                        {
                            rectAvel.x = (rectB.x + rectB.w - rectA.x) / elapsedTime;
                            collisions[1] = true;
                        }
                        // up to down
                        if (rectA.y + rectA.h <= rectB.y && rectA.y + rectA.h + (rectAvel.y * elapsedTime) > rectB.y &&
                            rectA.x - rectB.x < rectB.w && rectA.x + rectA.w > rectB.x)
                        {
                            rectAvel.y = (rectB.y - rectA.y - rectA.h) / elapsedTime;
                            collisions[2] = true;
                        }
                        // down to up
                        if (rectA.y >= rectB.y + rectB.h && rectA.y + (rectAvel.y * elapsedTime) < rectB.y + rectB.h &&
                            rectA.x - rectB.x < rectB.w && rectA.x + rectA.w > rectB.x)
                        {
                            rectAvel.y = (rectB.y + rectB.h - rectA.y) / elapsedTime;
                            collisions[3] = true;
                        }


                        // top right corner
                        if (rectB.x < rectA.x + rectA.w + (rectAvel.x * elapsedTime) && rectB.h < rectA.y + rectA.h + (rectAvel.y * elapsedTime) 
                            && rectA.x < rectB.x && rectA.y < rectB.y)
                        {
                            rectAvel.x = (rectB.x - (rectA.x +rectA.w))/elapsedTime;
                            rectAvel.y = (rectB.y - (rectA.y+ rectA.h))/elapsedTime;
                            collisions[0] = true;
                            collisions[3] = true;
                        }
                        //top left corner
                        if(rectB.x + rectB.w > rectA.x + (rectAvel.x * elapsedTime) && rectA.y + rectA.h +  (rectAvel.y * elapsedTime) > rectB.h &&
                           rectA.x + (rectAvel.x * elapsedTime) > rectB.x && rectA.y < rectB.y )
                           { 

                            rectAvel.x = ((rectB.x + rectB.w) - rectA.x)/elapsedTime;
                            rectAvel.y = ((rectA.y + rectA.h) - rectB.y)/elapsedTime;   
                            collisions[1] = true;
                            collisions[3] = true;

                           }
                        //bottom right corner
                        if(rectB.x < rectA.x + rectA.w + (rectAvel.x * elapsedTime) &&  (rectB.y + rectB.h)  > rectA.y +  (rectAvel.y * elapsedTime) &&
                          rectA.x < rectB.x && rectA.y + rectA.h > rectB.h)
                          {
                            rectAvel.x = (rectB.x - (rectA.x +rectA.w))/elapsedTime;
                            rectAvel.y = ( (rectB.y + rectB.h) - rectA.y)/elapsedTime;   
                            collisions[0] = true;
                            collisions[2] = true;
                        
                          }
                        //bottom left corner
                        if(rectB.x + rectB.w > rectA.x + (rectAvel.x * elapsedTime) && (rectB.y + rectB.h)  > rectA.y +  (rectAvel.y * elapsedTime) &&
                        rectA.x > rectB.x  && rectB.y < rectA.y)
                        {
                            rectAvel.x = ((rectB.x + rectB.w) - rectA.x)/elapsedTime;
                            rectAvel.y = ( (rectB.y + rectB.h) - rectA.y)/elapsedTime;   
                            collisions[1] = true;
                            collisions[2] = true;
                        }


                    }
                    if (collisions[0] == true || collisions[1] == true || collisions[2]  == true || collisions[3]  == true)
                    {
                        std::cout << collisions[0] << ' ' << collisions[1] << ' ' << collisions[2] << ' '
                                  << collisions[3] << '\n';
                    }
                }
            }
        }
    }

    void render(yorcvs::Window<yorcvs::SDL2> *testWindow) const
    {
        for (const auto &ID : entityList->entitiesID)
        {
            yorcvs::Vec2<float> pos = world->get_component<positionComponent>(ID).position;
            pos += {world->get_component<hitboxComponent>(ID).hitbox.x,
                    world->get_component<hitboxComponent>(ID).hitbox.y};
            yorcvs::Vec2<float> size = {world->get_component<hitboxComponent>(ID).hitbox.w,
                                        world->get_component<hitboxComponent>(ID).hitbox.h};
            testWindow->draw_sprite("assets/lettuce.png", pos, size, {0, 0, 200, 200});
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};

class VelocitySystem
{
  public:
    VelocitySystem(yorcvs::ECS *parent)
    {
        world = parent;
        if (!world->is_component_registered<positionComponent>())
        {
            world->register_component<positionComponent>();
        }
        if (!world->is_component_registered<velocityComponent>())
        {
            world->register_component<velocityComponent>();
        }
        world->register_system<VelocitySystem>(*this);
        world->add_criteria_for_iteration<VelocitySystem, positionComponent, velocityComponent>();
    }
    void update(float dt) const
    {
        for (const auto &ID : entityList->entitiesID)
        {
            yorcvs::Vec2<float> posOF = world->get_component<velocityComponent>(ID).vel;
            posOF *= dt;
            world->get_component<positionComponent>(ID).position += posOF;
            world->get_component<velocityComponent>(ID) = {{0, 0}};
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};
