/**
 * @file Yorcvs.h
 * @author Dantsz
 * @brief 
 * @version 0.1
 * @date 2021-07-31
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include "common/timer.h"
#include "window/windowSDL2.h"
#include <cstdlib>
#include "Yorcvs.h"
#include "common/ecs.h"
#include "systems.h"
#include "systemsSDL2.h"
#include <thread>
#include <future>

namespace yorcvs
{
    class DebugInfo
    {
        public:
        DebugInfo(yorcvs::Window<yorcvs::SDL2>* parentW, yorcvs::ECS* pECS)
        {
            parentWindow = parentW;
            appECS = pECS;
            frameTime = parentWindow->create_text("assets/font.ttf","Frame Time : ",255,255,255,255,100,1000);
        }

        void update(float ft)
        {
           
            parentWindow->set_text_message(frameTime,"Frame Time : " + std::to_string(ft));
        }
        
        void render() const
        {
            parentWindow->draw_text(frameTime,FTRect);
        }
        yorcvs::Window<yorcvs::SDL2>* parentWindow;
        yorcvs::ECS* appECS;
        yorcvs::Text<yorcvs::SDL2> frameTime;
        yorcvs::Rect<float> FTRect = {0,0,150,25};
    };
    /**
     * @brief Main game class
     * 
     */
    class Application
    {
        public:
        Application(): collisionS(&world), velocityS(&world),pcS(&world,&r),sprS(&world,&r),animS(&world),healthS(&world)
        {
           r.init("TEst", 960, 500);
           entities.emplace_back(&world);
                world.add_component<hitboxComponent>(entities[0].id,{{75,75,20,45}});
                world.add_component<positionComponent>(entities[0].id,{{100,100}});
                world.add_component<velocityComponent>(entities[0].id,{{0.0f,0.0f},{false,false}});
                world.add_component<playerMovementControlledComponent>(entities[0].id,{});
                world.add_component<spriteComponent>(entities[0].id,{{0.0f,0.0f},{160.0f,160.0f},{0,64,32,32},r.create_texture("assets/test_player_sheet.png")});
                world.add_component<animationComponent>(entities[0].id,{0,8,0.0f,10.0f});

           entities.emplace_back(&world);
                world.add_component<hitboxComponent>(entities[1].id,{{0,0,160,160}});
                world.add_component<positionComponent>(entities[1].id,{{500,100}});
                world.add_component<spriteComponent>(entities[1].id,{{0.0f,0.0f},{160.0f,160.0f},{0,0,200,200},r.create_texture("assets/lettuce.png")});


           entities.emplace_back(&world);
                world.add_component<hitboxComponent>(entities[2].id,{{0,0,160,160}});
                world.add_component<positionComponent>(entities[2].id,{{660,100}});
                world.add_component<spriteComponent>(entities[2].id,{{0.0f,0.0f},{160.0f,160.0f},{0,0,200,200},r.create_texture("assets/lettuce.png")});

            entities.emplace_back(&world);
                world.add_component<hitboxComponent>(entities[3].id,{{0,0,160,160}});
                world.add_component<positionComponent>(entities[3].id,{{500,260}});
                world.add_component<spriteComponent>(entities[3].id,{{0.0f,0.0f},{160.0f,160.0f},{0,0,200,200},r.create_texture("assets/lettuce.png")});
            counter.start();
        }
        Application(const Application& other) = delete;
        Application(Application&& other) = delete;
        Application& operator=(const Application& other) = delete;
        Application& operator=(Application&& other) = delete;

        void run()
        {
            float elapsed = counter.get_ticks<float,std::chrono::nanoseconds>();
            elapsed /= 1000000.0f;
            dbInfo.update(elapsed);
            counter.start();
            lag += elapsed;
            
            r.handle_events();
            
            while(lag >= msPF)
            {
                pcS.updateControls();
                collisionS.update(lag);
                velocityS.update(lag);
                animS.update(lag);
                healthS.update(lag);
                pcS.updateAnimations();
                lag -= msPF;
            }


        
            
            r.clear();

            sprS.renderSprites();
            dbInfo.render();
            r.present();

        }



        void runMT()
        {
            float elapsed = counter.get_ticks<float,std::chrono::nanoseconds>();
            elapsed /= 1000000.0f;
            dbInfo.update(elapsed);
            counter.start();
            lag += elapsed;
        
            r.handle_events();
            
            while(lag >= msPF)
            {
            pcS.updateControls();

            collisionS.update(lag);

            auto velAsync = std::async(&VelocitySystem::update,velocityS,lag);
            
            auto velAnims = std::async(&AnimationSystem::update,animS,lag);
            healthS.update(lag);
            pcS.updateAnimations();
            lag -= msPF;
            velAsync.get();
            velAnims.get();
            }


        
            
            r.clear();
            
            sprS.renderSprites();
            dbInfo.render();
            r.present();


        }

        [[nodiscard]]bool is_active() const
        {
            return r.isActive;
        }

        ~Application()
        {
            r.cleanup();
        }
        private:
        yorcvs::Window<yorcvs::SDL2> r;
        yorcvs::ECS world{};
        CollisionSystem collisionS;
        VelocitySystem velocityS;
        PlayerMovementControl pcS;
        SpriteSystem sprS;
        AnimationSystem animS;
        HealthSystem healthS;

        yorcvs::Timer counter;

        static constexpr float msPF = 16.6f;
        float lag = 0.0f;

        std::vector<yorcvs::Entity> entities;
        //debug stuff
        DebugInfo dbInfo{&r,&world};
    };
}
