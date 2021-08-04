/**
 * @file systemsSDL2.h
 * @author your name (you@domain.com)
 * @brief Systems dependent on sdl2
 * @version 0.1
 * @date 2021-08-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "components.h"
#include "ecs/ecs.h"
#include "window/windowSDL2.h"
#include <SDL.h>

class PlayerMovementControl
{
    public:
    PlayerMovementControl(yorcvs::ECS* parent,yorcvs::Window<yorcvs::SDL2>* parent_window)
    {
        window = parent_window;
        world = parent;
        if(!world->is_component_registered<playerMovementControlledComponent>())
        {
            world->register_component<playerMovementControlledComponent>();
        }
        world->register_system<PlayerMovementControl>(*this);
        world->add_criteria_for_iteration<PlayerMovementControl,playerMovementControlledComponent,velocityComponent>();
    }   

    void update()
    {
        bool w_pressed = window->is_key_pressed({SDL_SCANCODE_W});
        bool a_pressed = window->is_key_pressed({SDL_SCANCODE_A});
        bool s_pressed = window->is_key_pressed({SDL_SCANCODE_S});
        bool d_pressed = window->is_key_pressed({SDL_SCANCODE_D});
        for(const auto& ID : entityList->entitiesID)
        {

          world->get_component<velocityComponent>(ID).vel += yorcvs::Vec2<float>(static_cast<float>(d_pressed) + static_cast<float>(a_pressed)*-1.0f, 
                                                               static_cast<float>(w_pressed)*-1.0f + static_cast<float>(s_pressed));
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* world;
    yorcvs::Window<yorcvs::SDL2>* window;
};
