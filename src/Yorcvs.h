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
#include "common/utilities.h"
#include "common/types.h"
#include "systems.h"
#include "windowSDL2.h"
#include <cstdlib>
#include <future>
#include <thread>
#include <nlohmann/json.hpp>

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
        maxframeTimeTX =
            parentWindow->create_text("assets/font.ttf", "Max Frame Time : ", 255, 255, 255, 255, 100, 10000);
        ecsEntities =
            parentWindow->create_text("assets/font.ttf", "Active Entities : ", 255, 255, 255, 255, 100, 10000);
        playerPosition =
            parentWindow->create_text("assets/font.ttf", "NO PLAYER FOUND ", 255, 255, 255, 255, 100, 10000);
        playerHealth = parentWindow->create_text("assets/font.ttf", "Health : -/- ", 255, 255, 255, 255, 100, 10000);
        //TODO: remove this test
        nlohmann::json j2 = {
        {"pi", 3.141},
        {"happy", true},
        {"name", "Niels"},
        {"nothing", nullptr},
        {"answer", {
            {"everything", 42}
        }},
        {"list", {1, 0, 2}},
        {"object", {
            {"currency", "USD"},
            {"value", 42.99}
        }}
        };
        std::cout<< j2.dump(2) << '\n';
    }

    void update(float ft)
    {
        if (parentWindow->is_key_pressed({SDL_SCANCODE_R}))
        {
            reset();
        }

        parentWindow->set_text_message(frameTime, "Frame Time : " + std::to_string(ft));

        if (ft > maxFrameTime)
        {
            maxFrameTime = ft;
            parentWindow->set_text_message(maxframeTimeTX, "Max Frame Time: " + std::to_string(maxFrameTime));
        }

        parentWindow->set_text_message(ecsEntities,
                                       "Active Entities : " + std::to_string(appECS->get_active_entities_number()));
        // set player position text

        if (playerMoveSystem->entityList->entitiesID.empty())
        {
            parentWindow->set_text_message(playerPosition, "NO PLAYER FOUND");
            parentWindow->set_text_message(playerHealth, "Health: -/-");
        }
        else
        {
            size_t ID = playerMoveSystem->entityList->entitiesID[0];
            parentWindow->set_text_message(
                playerPosition,
                "Player position : X = " + std::to_string(appECS->get_component<positionComponent>(ID).position.x) +
                    " Y = " + std::to_string(appECS->get_component<positionComponent>(ID).position.y));

            if (appECS->has_components<healthComponent>(ID))
            {
                healthComponent &playerHealthC = appECS->get_component<healthComponent>(ID);
                parentWindow->set_text_message(playerHealth, "Health: " + std::to_string(playerHealthC.HP) + " / " +
                                                                 std::to_string(playerHealthC.maxHP));
            }
        }
    }

    void render(float elapsed)
    {

        if (parentWindow->is_key_pressed({SDL_SCANCODE_E}))
        {
            update(elapsed);
            parentWindow->draw_text(frameTime, FTRect);
            parentWindow->draw_text(maxframeTimeTX, maxFTRect);
            parentWindow->draw_text(ecsEntities, entitiesRect);
            parentWindow->draw_text(playerPosition, pPositionRect);
            parentWindow->draw_text(playerHealth, playerHealthRect);
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

    yorcvs::Text<yorcvs::graphics> playerHealth;
    yorcvs::Rect<float> playerHealthRect = {0, 100, 200, 25};

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
        : r("Yorcvs", 960, 480), collisionS(&world), velocityS(&world), pcS(&world, &r), sprS(&world, &r),
          animS(&world), healthS(&world), dbInfo{&r, &world, &pcS}
    {

        entities.emplace_back(&world);
        world.add_component<hitboxComponent>(entities[0].id, {{28, 12, 12, 40}});
        world.add_component<positionComponent>(entities[0].id, {{0, 0}});
        world.add_component<velocityComponent>(entities[0].id, {{0.0f, 0.0f}, {false, false}});
        world.add_component<playerMovementControlledComponent>(entities[0].id, {});
        world.add_component<spriteComponent>(
            entities[0].id,
            {{0.0f, 0.0f}, {64.0f, 64.0f}, {0, 128, 64, 64}, r.create_texture("assets/test_player_sheet.png")});
        world.add_component<animationComponent>(entities[0].id, {0, 8, 0.0f, 100.0f});
        world.add_component<healthComponent>(entities[0].id,{5,10,0.1f,false});

        entities.emplace_back(&world);
        world.add_component<hitboxComponent>(entities[1].id, {{0, 0, 60, 60}});
        world.add_component<positionComponent>(entities[1].id, {{60, 60}});
        world.add_component<spriteComponent>(
            entities[1].id, {{0.0f, 0.0f}, {60.0f, 60.0f}, {0, 0, 200, 200}, r.create_texture("assets/lettuce.png")});

        entities.emplace_back(&world);
        world.add_component<hitboxComponent>(entities[2].id, {{0, 0, 60, 60}});
        world.add_component<positionComponent>(entities[2].id, {{120, 60}});
        world.add_component<spriteComponent>(
            entities[2].id, {{0.0f, 0.0f}, {60.0f, 60.0f}, {0, 0, 200, 200}, r.create_texture("assets/lettuce.png")});

        entities.emplace_back(&world);
        world.add_component<hitboxComponent>(entities[3].id, {{0, 0, 60, 60}});
        world.add_component<positionComponent>(entities[3].id, {{60, 120}});
        world.add_component<spriteComponent>(
            entities[3].id, {{0.0f, 0.0f}, {60.0f, 60.0f}, {0, 0, 200, 200}, r.create_texture("assets/lettuce.png")});
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
    DebugInfo dbInfo;
    yorcvs::Timer counter;

    static constexpr float msPF = 16.6f;
    float lag = 0.0f;
    yorcvs::Vec2<float> render_dimensions = {480.0f, 240.0f}; // how much to render

    std::vector<yorcvs::Entity> entities;
    // debug stuff
};
} // namespace yorcvs
