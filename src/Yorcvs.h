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
#include "Yorcvs.h"
#include "common/ecs.h"
#include "common/types.h"
#include "common/utilities.h"
#include "systems.h"

#include "tmxlite/Object.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/Property.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/Tileset.hpp"
#include "windowSDL2.h"
#include <cstdlib>
#include <exception>
#include <future>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
namespace json = nlohmann;
#include "tmxlite/Layer.hpp"
#include <filesystem>
#include <fstream>
#include <tmxlite/Map.hpp>

namespace yorcvs
{
class DebugInfo
{
  public:
    DebugInfo() = default;

    DebugInfo(yorcvs::Window<yorcvs::SDL2> *parentW, yorcvs::ECS *pECS, PlayerMovementControl *pms,
              CollisionSystem *cols)
        : parentWindow(parentW), appECS(pECS), playerMoveSystem(pms), colSystem(cols)
    {
        attach(parentW, pECS, pms, cols);
        // TODO: remove this test
    }

    ~DebugInfo() = default;

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

    void render(float elapsed, const yorcvs::Vec2<float> &render_dimensions)
    {

        if (parentWindow->is_key_pressed({SDL_SCANCODE_E}))
        {
            update(elapsed);
            parentWindow->draw_text(frameTime, FTRect);
            parentWindow->draw_text(maxframeTimeTX, maxFTRect);
            parentWindow->draw_text(ecsEntities, entitiesRect);
            parentWindow->draw_text(playerPosition, pPositionRect);
            parentWindow->draw_text(playerHealth, playerHealthRect);
            colSystem->render_hitboxes(*parentWindow, render_dimensions, 255, 0, 0, 255);
        }
    }

    void attach(yorcvs::Window<yorcvs::SDL2> *parentW, yorcvs::ECS *pECS, PlayerMovementControl *pms,
                CollisionSystem *cols)
    {
        parentWindow = parentW;
        appECS = pECS;
        playerMoveSystem = pms;
        colSystem = cols;
        frameTime = parentWindow->create_text("assets/font.ttf", "Frame Time : ", 255, 255, 255, 255, 100, 10000);
        maxframeTimeTX =
            parentWindow->create_text("assets/font.ttf", "Max Frame Time : ", 255, 255, 255, 255, 100, 10000);
        ecsEntities =
            parentWindow->create_text("assets/font.ttf", "Active Entities : ", 255, 255, 255, 255, 100, 10000);
        playerPosition =
            parentWindow->create_text("assets/font.ttf", "NO PLAYER FOUND ", 255, 255, 255, 255, 100, 10000);
        playerHealth = parentWindow->create_text("assets/font.ttf", "Health : -/- ", 255, 255, 255, 255, 100, 10000);
    }

    void reset()
    {
        maxFrameTime = 0.0f;
    }
    yorcvs::Window<yorcvs::SDL2> *parentWindow{};
    yorcvs::ECS *appECS{};

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

    PlayerMovementControl *playerMoveSystem{};

    CollisionSystem *colSystem{};
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
    Map(const std::string &path, yorcvs::ECS *world, yorcvs::Window<yorcvs::SDL2> &r)
        : ecs(world), init_ecs(*world), parentWindow(&r), collisionS(world), velocityS(world), pcS(world, &r),
          animS(world), healthS(world)
    {

        load(world, &r, path);
        entities.emplace_back(world);
        load_character_from_path(entities[0], "assets/player.json");
    }

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
            yorcvs::log("Cannot load finite  maps", yorcvs::MSGSEVERITY::ERROR);
        }
        tilesSize = {static_cast<float>(map.getTileSize().x), static_cast<float>(map.getTileSize().y)};
        const auto &layers = map.getLayers();

        for (const auto &layer : layers) // parse layers
        {
            const auto &properties = layer->getProperties();
            bool tiles_ysorted = false;
            switch (layer->getType())
            {
            case tmx::Layer::Type::Tile:

                for (const auto &property : properties)
                {
                    if (property.getName() == "Ysorted")
                    {
                        tiles_ysorted = true;
                    }
                }
                if (tiles_ysorted)
                {
                    parse_tile_layer_ysorted(map, layer->getLayerAs<tmx::TileLayer>());
                }
                else
                {
                    parse_tile_layer(map, layer->getLayerAs<tmx::TileLayer>());
                }

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
    void parse_tile_layer_ysorted(tmx::Map &map, tmx::TileLayer &tileLayer) const
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

                    /// add object
                    size_t entity = ecs->create_entity_ID();
                    ecs->add_component<positionComponent>(
                        entity,
                        {chunk_position * tilesSize +
                         tilesSize * yorcvs::Vec2<float>{static_cast<float>(chunk_x), static_cast<float>(chunk_y)}});
                    ecs->add_component<spriteComponent>(
                        entity,
                        {{0, 0},
                         {static_cast<float>(tile_set->getTileSize().x), static_cast<float>(tile_set->getTileSize().y)},
                         get_src_rect_from_uid(map, chunk.tiles[tileIndex].ID),
                         parentWindow->create_texture(tile_set->getImagePath())});
                }
            }
        }
    }

    /**
     * THESE FUNCTION ALL BEHAVE THE SAME
     *  RETURN TRUE IF THE PROPERTY EXISTS
     *  RETURN FALSE IF IT'S UNKNOWN
     */
    bool object_handle_property_bool(size_t entity, const tmx::Property &property, const tmx::Object &object)
    {
        // Note: handles hitbox to object
        if (property.getName() == "collision" && property.getBoolValue())
        {
            ecs->add_component<hitboxComponent>(entity, {{0, 0, object.getAABB().width, object.getAABB().height}});

            // TILED HAS A WEIRD BEHAVIOUR THAT IF AN TILE IS INSERTED AS A OBJECT IT'S Y POSITION IS DIFFERENT
            // FROM AN RECTANGLE OBJECT AND DOESN'T LOOK LIKE IN THE EDITOR
            if (!ecs->has_components<spriteComponent>(entity))
            {
                ecs->get_component<hitboxComponent>(entity).hitbox.y += object.getAABB().height;
            }
            return true;
        }
        // NOTE: handles player spawn area
        if (property.getName() == "playerSpawn" && property.getBoolValue())
        {
            spawn_coord = {object.getPosition().x, object.getPosition().y};
            return true;
        }
        return false;
    }
    bool object_handle_property_color(size_t entity, const tmx::Property &property);
    bool object_handle_property_float(size_t entity, const tmx::Property &property);
    bool object_handle_property_file(size_t entity, const tmx::Property &property);
    bool object_handle_property_int(size_t entity, const tmx::Property &property) const
    {
        // NOTE HANDLES HP
        if (property.getName() == "HP")
        {
            if (!ecs->has_components<healthComponent>(entity))
            {
                ecs->add_component<healthComponent>(entity, {});
            }
            ecs->get_component<healthComponent>(entity).HP = property.getFloatValue();
            return true;
        }
        if (property.getName() == "HP_max")
        {
            if (!ecs->has_components<healthComponent>(entity))
            {
                ecs->add_component<healthComponent>(entity, {});
            }
            ecs->get_component<healthComponent>(entity).maxHP = property.getFloatValue();
            return true;
        }
        if (property.getName() == "HP_regen")
        {
            if (!ecs->has_components<healthComponent>(entity))
            {
                ecs->add_component<healthComponent>(entity, {});
            }
            ecs->get_component<healthComponent>(entity).health_regen = property.getFloatValue();
            return true;
        }
    }
    bool object_handle_property_object(size_t entity, const tmx::Property &property);
    bool object_handle_property_string(size_t entity, const tmx::Property &property);

    void parse_object_layer(tmx::Map &map, tmx::ObjectGroup &objectLayer)
    {
        const auto &objects = objectLayer.getObjects();
        for (const auto &object : objects)
        {
            // create entity
            size_t entity = ecs->create_entity_ID();
            ecs->add_component<positionComponent>(
                entity, {{object.getPosition().x, object.getPosition().y - object.getAABB().height}});
            if (object.getTileID() != 0 && object.visible())
            {
                const auto *tileSet = get_tileset_containing(map, object.getTileID());
                // add sprite component

                ecs->add_component<spriteComponent>(entity, {{0, 0},
                                                             {object.getAABB().width, object.getAABB().height},
                                                             get_src_rect_from_uid(map, object.getTileID()),
                                                             parentWindow->create_texture(tileSet->getImagePath())});
            }
            /*
            Object properties
            * collision - object has collision
            * playerSpawn - objects' coordinates are where the player can spawn
            * HP - health
            * HP_max - maximum hp
            * HP_Regen - health regeneration
            */

            for (const auto &property : object.getProperties())
            {
                bool known = false;
                switch (property.getType())
                {
                case tmx::Property::Type::Int:
                    known = object_handle_property_int(entity, property);
                    break;
                case tmx::Property::Type::Boolean:
                    known = object_handle_property_bool(entity, property, object);
                    break;

                default:
                    break;
                }
                if (!known)
                {
                    yorcvs::log("UNKNOWN PROPERTY " + property.getName() + " of object " +
                                    std::to_string(object.getUID()),
                                yorcvs::MSGSEVERITY::WARNING);
                }
            }
        }
    }
    yorcvs::Vec2<float> get_spawn_position()
    {
        return spawn_coord;
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
        while (y_index + tile_set->getColumnCount() <= UID - tile_set->getFirstGID())
        {
            y_index += tile_set->getColumnCount();
            srcRect.y += tile_set->getTileSize().y;
        }
        srcRect.w = tile_set->getTileSize().x;
        srcRect.h = tile_set->getTileSize().y;
        return srcRect;
    }

    static tmx::Tileset const *get_tileset_containing(tmx::Map &map, const size_t tile_UID)
    {
        tmx::Tileset const *tile_set = nullptr;
        for (const auto &tileset : map.getTilesets())
        {
            if (tileset.hasTile(tile_UID))
            {
                tile_set = &tileset;
            }
        }
        if (tile_set == nullptr)
        {
            yorcvs::log("no tileset contains tile ID : " + std::to_string(tile_UID));
        }
        return tile_set;
    }

    void update(float dt, const yorcvs::Vec2<float> &render_dimensions)
    {
        collisionS.update();
        velocityS.update();
        animS.update(dt);
        healthS.update(dt);
        pcS.updateAnimations();
        pcS.updateControls(render_dimensions);
    }

    void render(const yorcvs::Vec2<float> &render_dimensions, yorcvs::Window<SDL2> &r, float elapsed)
    {
        render_tiles(r, render_dimensions);
    }

    void load_character_from_path(yorcvs::Entity &entity, const std::string &path)
    {
        std::ifstream playerIN(path);
        std::string playerData{(std::istreambuf_iterator<char>(playerIN)), (std::istreambuf_iterator<char>())};
        auto player = json::json::parse(playerData);

        ecs->add_component<hitboxComponent>(
            entity.id, {{player["hitbox"]["x"], player["hitbox"]["y"], player["hitbox"]["w"], player["hitbox"]["h"]}});
        ecs->add_component<positionComponent>(entity.id, {get_spawn_position()});
        ecs->add_component<velocityComponent>(entity.id, {{0.0f, 0.0f}, {false, false}});
        ecs->add_component<playerMovementControlledComponent>(entity.id, {});
        ecs->add_component<spriteComponent>(entity.id,
                                            {{player["sprite"]["offset"]["x"], player["sprite"]["offset"]["y"]},
                                             {player["sprite"]["size"]["x"], player["sprite"]["size"]["y"]},
                                             {player["sprite"]["srcRect"]["x"], player["sprite"]["srcRect"]["y"],
                                              player["sprite"]["srcRect"]["w"], player["sprite"]["srcRect"]["h"]},
                                             parentWindow->create_texture(player["sprite"]["spriteName"])});
        ecs->add_component<healthComponent>(entity.id, {5, 10, 0.1f, false});
        ecs->add_component<animationComponent>(entity.id, {});
        for (const auto &animation : player["sprite"]["animations"])
        {

            bool animation_fail = animS.add_animation(entity.id, animation["name"], animation["speed"]);
            for (const auto &frame : animation["frames"])
            {
                bool frame_fail = animS.add_animation_frame(entity.id, animation["name"],
                                                            {frame["x"], frame["y"], frame["w"], frame["h"]});
            }
        }
        animS.set_animation(entity, "idleL");
    }

  public:
    yorcvs::ECS *ecs{};

  private:
    struct ecs_Initializer
    {
        ecs_Initializer(yorcvs::ECS &world)
        {
            // register components
            world.register_component<hitboxComponent, positionComponent, velocityComponent, healthComponent>();
            world.register_component<playerMovementControlledComponent>();
            world.register_component<spriteComponent, animationComponent>();
        }
    };
    // class to initialize the ecs before systems are constructed
    ecs_Initializer init_ecs;

  public:
    yorcvs::Window<yorcvs::graphics> *parentWindow{};
    PlayerMovementControl pcS;
    CollisionSystem collisionS;



    yorcvs::Vec2<float> tilesSize;
    std::vector<yorcvs::Tile> tiles;
  private:
    
    std::string tilesetPath;
    yorcvs::Vec2<float> spawn_coord;
    VelocitySystem velocityS;
    AnimationSystem animS;
    HealthSystem healthS;
    std::vector<yorcvs::Entity> entities;
};

// TODO: MAKE SOME SYSTEMS MAP-DEPENDENT AND REMOVE THIS

/**
 * @brief Main game class
 *
 */
class Application
{
  public:
    Application()
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
            else
            {
                if (!config["window"]["width"].is_discarded() && !config["window"]["height"].is_discarded())
                {
                    r.set_size(config["window"]["width"], config["window"]["height"]);
                }
                if (!config["engine_settings"].is_discarded())
                {
                    if (!config["engine_settings"]["render_width"].is_discarded())
                    {
                        render_dimensions.x = config["engine_settings"]["render_width"];
                    }
                    if (!config["engine_settings"]["render_height"].is_discarded())
                    {
                        render_dimensions.y = config["engine_settings"]["render_height"];
                    }
                }
            }
        }
        else
        {
            yorcvs::log("Config file not found, loading default settings...");
        }
       
        dbInfo.attach(&r, map.ecs, &map.pcS, &map.collisionS);
        counter.start();
    }
    Application(const Application &other) = delete;
    Application(Application &&other) = delete;
    Application &operator=(const Application &other) = delete;
    Application &operator=(Application &&other) = delete;

    void update(float dt)
    {

        map.update(dt, render_dimensions);
    }
    void render_map_tiles(yorcvs::Map& map)
    {

        yorcvs::Vec2<float> rs = r.get_render_scale();
        r.set_render_scale(r.get_size() / render_dimensions);
        for (const auto &tile : map.tiles)
        {
            r.draw_sprite(tile.texture_path, {tile.coords.x, tile.coords.y, map.tilesSize.x, map.tilesSize.y},
                               tile.srcRect);
        }
        r.set_render_scale(rs);
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

            update(msPF);
            lag -= msPF;
        }

        r.clear();
        map.render(render_dimensions, r, elapsed);
        render_map_tiles(map);
        sprS.renderSprites(render_dimensions);
        dbInfo.render(elapsed, render_dimensions);
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
    yorcvs::Timer counter;

    static constexpr float msPF = 16.6f;
    float lag = 0.0f;
    yorcvs::Vec2<float> render_dimensions = {240.0f, 120.0f}; // how much to render

    yorcvs::ECS world{};
    yorcvs::Map map{"assets/map.tmx", &world, r};
    SpriteSystem sprS{map.ecs,&r}; 
    // debug stuff
    DebugInfo dbInfo;
};
} // namespace yorcvs
