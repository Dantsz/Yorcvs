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
#include "common/types.h"
#include "common/utilities.h"
#include "systems.h"
#include "tmxlite/Layer.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/Tileset.hpp"
#include "windowSDL2.h"
#include <cstdlib>
#include <future>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
namespace json = nlohmann;
#include <filesystem>
#include <fstream>
#include <tmxlite/Map.hpp>

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
        // TODO: remove this test
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

struct Tile
{
    yorcvs::Vec2<float> coords;
    yorcvs::Rect<size_t> srcRect;
    std::string texture_path;
};

class Map
{
  public:
    void load(yorcvs::ECS *parent, yorcvs::Window<yorcvs::graphics> *window, const std::string &path)
    {
        ecs = parent;
        parentWindow = window;
        yorcvs::log("Loading map: " + path);
        tmx::Map map{};
        if (!map.load(path))
        {
            yorcvs::log("Map loading failed", yorcvs::MSGSEVERITY::ERROR);
        }
        const auto &tilesets = map.getTilesets();
        yorcvs::log("Map contains " + std::to_string(tilesets.size()) + " tile sets: ");
        for (const auto &tileset : tilesets)
        {
            yorcvs::log(tileset.getImagePath());
        }
        if (!map.isInfinite())
        {
            yorcvs::log("Cannot load non-infinte maps", yorcvs::MSGSEVERITY::ERROR);
        }
        tilesSize = {static_cast<float>(map.getTileSize().x), static_cast<float>(map.getTileSize().y)};
        const auto &layers = map.getLayers();
        for (const auto &layer : layers) // parse layers
        {
            switch (layer->getType())
            {
            case tmx::Layer::Type::Tile:
                parse_tile_layer(map, layer->getLayerAs<tmx::TileLayer>());
                break;
            case tmx::Layer::Type::Object:
                parse_object_layer(map, layer->getLayerAs<tmx::ObjectGroup>());
                break;
            case tmx::Layer::Type::Image:

                break;
            case tmx::Layer::Type::Group:

                break;
            }
        }
    }

    void parse_tile_layer(tmx::Map &map, tmx::TileLayer &tileLayer)
    {
        const auto &chunks = tileLayer.getChunks();

        for (const auto &chunk : chunks) // parse chunks
        {
            yorcvs::Vec2<float> chunk_position = {static_cast<float>(chunk.position.x),
                                                  static_cast<float>(chunk.position.y)};
            for (auto chunk_y = 0; chunk_y < chunk.size.y; chunk_y++)
            {
                for (auto chunk_x = 0; chunk_x < chunk.size.x; chunk_x++)
                {
                    // parse tiles
                    const size_t tileIndex = chunk_y * chunk.size.x + chunk_x;
                    if (chunk.tiles[tileIndex].ID == 0)
                    {
                        continue;
                    }
                    // chunk.tiles[tileIndex].ID;
                    // find tileset
                    tmx::Tileset const *tile_set = nullptr;
                    for (const auto &tileset : map.getTilesets())
                    {
                        if (tileset.hasTile(chunk.tiles[tileIndex].ID))
                        {
                            tile_set = &tileset;
                        }
                    }

                    // put the tile in the vector
                    yorcvs::Tile tile{};
                    tile.texture_path = tile_set->getImagePath();
                    tile.coords =
                        chunk_position * tilesSize +
                        tilesSize * yorcvs::Vec2<float>{static_cast<float>(chunk_x), static_cast<float>(chunk_y)};
                    tile.srcRect = get_src_rect_from_uid(map, chunk.tiles[tileIndex].ID);

                    tiles.push_back(tile);
                }
            }
        }
    }

    void parse_object_layer(tmx::Map &map, tmx::ObjectGroup &objectLayer)
    {
        const auto &objects = objectLayer.getObjects();
        for (const auto &object : objects)
        {
            // create entity
            size_t entity = ecs->create_entity_ID();
              ecs->add_component<positionComponent>(entity,{{object.getPosition().x,object.getPosition().y}});
            if (object.getTileID() != 0)
            {
                const auto *tileSet = get_tileset_containing(map,object.getTileID());
                // add sprite component
              
                ecs->add_component<spriteComponent>(entity, {{0, 0},
                                                             {object.getAABB().width, object.getAABB().height},
                                                             get_src_rect_from_uid(map, object.getTileID()),
                                                             parentWindow->create_texture(tileSet->getImagePath())});
            }
        }
    }

    void render_tiles(yorcvs::Window<yorcvs::graphics> &window, const yorcvs::Vec2<float> &render_dimensions)
    {

        yorcvs::Vec2<float> rs = window.get_render_scale();
        window.set_render_scale(window.get_size() / render_dimensions);
        for (const auto &tile : tiles)
        {
            window.draw_sprite(tile.texture_path, {tile.coords.x, tile.coords.y, tilesSize.x, tilesSize.y},
                               tile.srcRect);
        }
        window.set_render_scale(rs);
    }

    // NOTE: this can be a free function
    static yorcvs::Rect<size_t> get_src_rect_from_uid(const tmx::Map &map, const size_t UID)
    {
        tmx::Tileset const *tile_set = nullptr;
        for (const auto &tileset : map.getTilesets())
        {
            if (tileset.hasTile(UID))
            {
                tile_set = &tileset;
            }
        }
        if (tile_set == nullptr)
        {
            yorcvs::log(std::string("failed to find a tileset matching the uid: ") + std::to_string(UID),
                        yorcvs::MSGSEVERITY::ERROR);
            return {0, 0, 0, 0};
        }
        yorcvs::Rect<size_t> srcRect{};
        srcRect.x = ((UID - tile_set->getFirstGID()) % tile_set->getColumnCount()) * tile_set->getTileSize().x;
        srcRect.y = 0;
        size_t y_index = 0;
        while (y_index + tile_set->getColumnCount() < UID)
        {
            y_index += tile_set->getColumnCount();
            srcRect.y += tile_set->getTileSize().y;
        }
        srcRect.w = tile_set->getTileSize().x;
        srcRect.h = tile_set->getTileSize().y;
        return srcRect;
    }

    static tmx::Tileset const* get_tileset_containing(tmx::Map& map,const size_t tile_UID)
    {
        tmx::Tileset const *tile_set = nullptr;
        for (const auto &tileset : map.getTilesets())
        {
            if (tileset.hasTile(tile_UID))
            {
                tile_set = &tileset;
            }
        }
        if(tile_set != nullptr)
        {
          return tile_set;
        }
    }

  private:
    std::string tilesetPath;
    yorcvs::Vec2<float> tilesSize;
    std::vector<yorcvs::Tile> tiles;

    yorcvs::ECS *ecs{};
    yorcvs::Window<yorcvs::graphics> *parentWindow{};
};

/**
 * @brief Main game class
 *
 */
class Application
{
  public:
    Application()
        : r(), collisionS(&world), velocityS(&world), pcS(&world, &r), sprS(&world, &r), animS(&world),
          healthS(&world), dbInfo{&r, &world, &pcS}
    {
        // Load config
        if (std::filesystem::exists(configname))
        {
            yorcvs::log("Loading config file...");
            std::ifstream config_in(configname);
            // NOTE: should read whole file in string
            std::string confstr{(std::istreambuf_iterator<char>(config_in)), (std::istreambuf_iterator<char>())};
            auto config = json::json::parse(confstr);
            if (config.is_discarded())
            {
                yorcvs::log("Inavlid config file");
            }
            else if (!config["window"]["width"].is_discarded() && !config["window"]["height"].is_discarded())
            {
                r.set_size(config["window"]["width"], config["window"]["height"]);
            }
        }
        else
        {
            yorcvs::log("Config file not found, loading default settings...");
        }

        map.load(&world, &r, "assets/map.tmx");

        entities.emplace_back(&world);
        std::ifstream playerIN("assets/player.json");
        std::string playerData{(std::istreambuf_iterator<char>(playerIN)), (std::istreambuf_iterator<char>())};
        auto player = json::json::parse(playerData);

        world.add_component<hitboxComponent>(entities[0].id, {{player["hitbox"]["x"], player["hitbox"]["y"],
                                                               player["hitbox"]["w"], player["hitbox"]["h"]}});
        world.add_component<positionComponent>(entities[0].id, {{0, 0}});
        world.add_component<velocityComponent>(entities[0].id, {{0.0f, 0.0f}, {false, false}});
        world.add_component<playerMovementControlledComponent>(entities[0].id, {});
        world.add_component<spriteComponent>(entities[0].id,
                                             {{player["sprite"]["offset"]["x"], player["sprite"]["offset"]["y"]},
                                              {player["sprite"]["size"]["x"], player["sprite"]["size"]["y"]},
                                              {player["sprite"]["srcRect"]["x"], player["sprite"]["srcRect"]["y"],
                                               player["sprite"]["srcRect"]["w"], player["sprite"]["srcRect"]["h"]},
                                              r.create_texture(player["sprite"]["spriteName"])});
        world.add_component<animationComponent>(entities[0].id, {0, 8, 0.0f, 100.0f});
        world.add_component<healthComponent>(entities[0].id, {5, 10, 0.1f, false});

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
        map.render_tiles(r, render_dimensions);
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
    static constexpr const char *configname = "yorcvsconfig.json";

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
    yorcvs::Map map{};
    // debug stuff
};
} // namespace yorcvs
