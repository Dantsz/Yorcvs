/**
 * @file Yorcvs.h
 * @author Dantsz
 * @brief Simple entity component system
 * @version 0.1
 * @date 2021-07-31
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include "Yorcvs.h"
#include "common/ecs.h"
#include "common/timer.h"
#include "systems.h"
#include "windowSDL2.h"
#include <cstdlib>
#include <future>
#include <thread>

namespace yorcvs
{
class DebugInfo
{
  public:
    DebugInfo(yorcvs::Window<yorcvs::SDL2> *parentW, yorcvs::ECS *pECS, PlayerMovementControl *pms)
    {
        parentWindow = parentW;
        appECS = pECS;
        playerMoveSystem = pms;
        frameTime = parentWindow->create_text("assets/font.ttf", "Frame Time : ", 255, 255, 255, 255, 100, 10000);
        maxframeTimeTX = parentWindow->create_text("assets/font.ttf", "Max Frame Time : ", 255, 255, 255, 255, 100, 10000);
        ecsEntities = parentWindow->create_text("assets/font.ttf", "Active Entities : ", 255, 255, 255, 255, 100, 10000);
        playerPosition = parentWindow->create_text("assets/font.ttf", "NO PLAYER FOUND ", 255, 255, 255, 255, 100, 10000);
    }

    void update(float ft)
    {
        if(parentWindow->is_key_pressed({SDL_SCANCODE_R}))
        {
           reset();
        }
        
        parentWindow->set_text_message(frameTime, "Frame Time : " + std::to_string(ft));

        if(ft > maxFrameTime)
        {
            maxFrameTime = ft;
            parentWindow->set_text_message(maxframeTimeTX,"Max Frame Time: " + std::to_string(maxFrameTime));
        }

        parentWindow->set_text_message(ecsEntities,
                                       "Active Entities : " + std::to_string(appECS->get_active_entities_number()));
        // set player position text

        if (playerMoveSystem->entityList->entitiesID.empty())
        {
             parentWindow->set_text_message(
                playerPosition,"NO PLAYER FOUND");
        }
        else
        {
            parentWindow->set_text_message(
                playerPosition,
                "Player position : X = " +
                    std::to_string(appECS->get_component<positionComponent>(playerMoveSystem->entityList->entitiesID[0]).position.x) +
                    " Y = " + std::to_string(appECS->get_component<positionComponent>(playerMoveSystem->entityList->entitiesID[0]).position.y));
        }
    }

    void render(float elapsed) 
    {
        
        if(parentWindow->is_key_pressed({SDL_SCANCODE_E}))
        {
            update(elapsed);
            parentWindow->draw_text(frameTime, FTRect);
            parentWindow->draw_text(maxframeTimeTX,maxFTRect);
            parentWindow->draw_text(ecsEntities, entitiesRect);
            parentWindow->draw_text(playerPosition, pPositionRect);
        }
    }

    void reset()
    {
        maxFrameTime = 0.0f;
    }
    yorcvs::Window<yorcvs::SDL2> *parentWindow;
    yorcvs::ECS *appECS;
   
    yorcvs::Text<yorcvs::SDL2> frameTime;
    yorcvs::Rect<float> FTRect = {0, 0, 150, 25};
    
    float maxFrameTime = 0.0f;
    yorcvs::Text<yorcvs::SDL2> maxframeTimeTX;
    yorcvs::Rect<float> maxFTRect = {0, 25, 150, 25};

    yorcvs::Text<yorcvs::SDL2> ecsEntities;
    yorcvs::Rect<float> entitiesRect = {0, 50, 150, 25};

    yorcvs::Text<yorcvs::graphics> playerPosition;
    yorcvs::Rect<float> pPositionRect = {0, 75, 300, 25};
    PlayerMovementControl *playerMoveSystem;

};

/**
 * @brief Main game class
 *
 */
class Application
{
  public:
    Application()
        : collisionS(&world), velocityS(&world), pcS(&world, &r), sprS(&world, &r), animS(&world), healthS(&world)
    {
        r.init("TEst", 960, 480);
        entities.emplace_back(&world);
        world.add_component<hitboxComponent>(entities[0].id, {{14, 6, 6, 20}});
        world.add_component<positionComponent>(entities[0].id, {{0, 0}});
        world.add_component<velocityComponent>(entities[0].id, {{0.0f, 0.0f}, {false, false}});
        world.add_component<playerMovementControlledComponent>(entities[0].id, {});
        world.add_component<spriteComponent>(
            entities[0].id,
            {{0.0f, 0.0f}, {32.0f, 32.0f}, {0, 64, 32, 32}, r.create_texture("assets/test_player_sheet.png")});
        world.add_component<animationComponent>(entities[0].id, {0, 8, 0.0f, 500.0f});

        entities.emplace_back(&world);
        world.add_component<hitboxComponent>(entities[1].id, {{0, 0, 30, 30}});
        world.add_component<positionComponent>(entities[1].id, {{30, 30}});
        world.add_component<spriteComponent>(
            entities[1].id, {{0.0f, 0.0f}, {30.0f, 30.0f}, {0, 0, 200, 200}, r.create_texture("assets/lettuce.png")});

        entities.emplace_back(&world);
        world.add_component<hitboxComponent>(entities[2].id, {{0, 0, 30, 30}});
        world.add_component<positionComponent>(entities[2].id, {{60, 30}});
        world.add_component<spriteComponent>(
            entities[2].id, {{0.0f, 0.0f}, {30.0f, 30.0f}, {0, 0, 200, 200}, r.create_texture("assets/lettuce.png")});

        entities.emplace_back(&world);
        world.add_component<hitboxComponent>(entities[3].id, {{0, 0, 30, 30}});
        world.add_component<positionComponent>(entities[3].id, {{30, 60}});
        world.add_component<spriteComponent>(
            entities[3].id, {{0.0f, 0.0f}, {30.0f, 30.0f}, {0, 0, 200, 200}, r.create_texture("assets/lettuce.png")});
        counter.start();
    }
    Application(const Application &other) = delete;
    Application(Application &&other) = delete;
    Application &operator=(const Application &other) = delete;
    Application &operator=(Application &&other) = delete;
    
    void update(float dt)
    {
        
        collisionS.update();
        velocityS.update();
        animS.update(dt);
        healthS.update(dt);
        pcS.updateAnimations();
    }

    void updateMT(float dt)
    {
        

        collisionS.update();

        auto velAsync = std::async(&VelocitySystem::update, velocityS);

        auto velAnims = std::async(&AnimationSystem::update, animS, dt);
        healthS.update(dt);
        pcS.updateAnimations();

        velAsync.get();
        velAnims.get();
        counter.start();
    }

    void run()
    {
        float elapsed = counter.get_ticks<float, std::chrono::nanoseconds>();
        elapsed /= 1000000.0f;
        
        counter.stop();
        counter.start();
        lag += elapsed;
     
        r.handle_events();
       
       
        while (lag >= msPF)
        {   
            pcS.updateControls(render_dimensions);
            update(msPF);
            lag -= msPF;
        }

        r.clear();

        sprS.renderSprites(render_dimensions);
        dbInfo.render(elapsed);
        r.present();
    }

    [[nodiscard]] bool is_active() const
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
    DebugInfo dbInfo{&r, &world, &pcS};
    yorcvs::Timer counter;

    static constexpr float msPF = 16.6f;
    float lag = 0.0f;
    yorcvs::Vec2<float> render_dimensions = {120.0f,60.0f};// how much to render
   
    std::vector<yorcvs::Entity> entities;
    // debug stuff
};
} // namespace yorcvs
