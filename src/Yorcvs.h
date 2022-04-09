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
#include "engine/luaEngine.h"
#include "game/componentSerialization.h"
#include "game/components.h"
#include "game/systems.h"

#include "tmxlite/Object.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/Property.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/Tileset.hpp"
#include "engine/windowSDL2.h"
#include <cstdlib>
#include <exception>
#include <future>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
namespace json = nlohmann;
#include "tmxlite/Layer.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <tmxlite/Map.hpp>
#include "sol/sol.hpp"

#include "imgui.h"
#include "imgui_sdl.h"
#include "misc/cpp/imgui_stdlib.h"
// TODO: move this to utlities
namespace std
{
template <> struct hash<std::tuple<intmax_t, intmax_t>>
{
    size_t operator()(const std::tuple<intmax_t, intmax_t> &p) const
    {
        intmax_t x = std::get<0>(p);
        intmax_t y = std::get<1>(p);
        return yorcvs::spiral::unwrap(x, y);
    }
};
} // namespace std
namespace yorcvs
{

struct Tile
{
    yorcvs::Vec2<float> coords;
    yorcvs::Rect<size_t> srcRect;
    std::string texture_path;
};

/**
 * @brief Loads tmx map data into the ecs
 * 
 */
class Map
{
  public:
    Map(yorcvs::ECS *world)
        : ecs(world), init_ecs(*world), collisionS(world), healthS(world), sprintS(world), velocityS(world),
          animS(world)
    {
    }
    /**
     * @brief Construct a new Map object
     * 
     * @param path path to map file
     * @param world the ecs in which data shoudl be added
     */
    Map(const std::string &path, yorcvs::ECS *world) : Map(world)
    {
        load(world, path);
    }
    ~Map()
    {
        clear();
    }
    /**
     * @brief Loads data from path into the ecs
     * 
     * @param parent 
     * @param path 
     */
    void load(yorcvs::ECS *parent, const std::string &path)
    {
        ecs = parent;
        map_file_path = path;
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
    /**
     * @brief Updates the systems registered by the map
     * 
     * @param dt delta time
     */
    void update(const float dt)
    {
        collisionS.update(dt);
        velocityS.update(dt);
        animS.update(dt);
        healthS.update(dt);
        sprintS.update(dt);
    }
    /**
     * @brief Removes all entitites and tiles loaded by this map
     * 
     */
    void clear()
    {
        for(const auto& entity : entities)
        {
            if(ecs->is_valid_entity(entity))
            {
                ecs->destroy_entity(entity);
            }
        }
        entities.clear();
        ysorted_tiles.clear();
        tiles_chunks.clear();
    }
    /**
     * @brief Loads json entity data into the entity
     * 
     * @param entity_id id of the entity
     * @param path path to the json file
     */
    void load_character_from_path(const size_t entity_id, const std::string &path)
    {
        std::filesystem::path file = path;
        const std::string directory_path = file.remove_filename().generic_string();

        std::ifstream entityIN(path);
        std::string entityDATA{(std::istreambuf_iterator<char>(entityIN)), (std::istreambuf_iterator<char>())};
        auto entityJSON = json::json::parse(entityDATA);
        if (entityJSON.contains("identification"))
        {
            if (!ecs->has_components<identificationComponent>(entity_id))
            {
                ecs->add_component<identificationComponent>(entity_id, {});
            }

            yorcvs::components::deserialize(ecs->get_component<identificationComponent>(entity_id),
                                            entityJSON["identification"]);
        }

        if (entityJSON.contains("hitbox"))
        {
            if (!ecs->has_components<hitboxComponent>(entity_id))
            {
                ecs->add_component<hitboxComponent>(entity_id, {});
                yorcvs::components::deserialize(ecs->get_component<hitboxComponent>(entity_id), entityJSON["hitbox"]);
            }
        }

        // NOT IMPLEMENTED SERIALIZATION Yet
        if (!ecs->has_components<positionComponent>(entity_id))
        {
            ecs->add_component<positionComponent>(entity_id, {});
            // TODO: move this elsewhere
            ecs->get_component<positionComponent>(entity_id) = {get_spawn_position()};
        }
        if (!ecs->has_components<velocityComponent>(entity_id))
        {
            ecs->add_component<velocityComponent>(entity_id, {});
        }
        ecs->get_component<velocityComponent>(entity_id) = {{0.0f, 0.0f}, {false, false}};

        if (entityJSON.contains("health"))
        {
            if (!ecs->has_components<healthComponent>(entity_id))
            {
                ecs->add_component<healthComponent>(entity_id, {});
            }

            yorcvs::components::deserialize(ecs->get_component<healthComponent>(entity_id), entityJSON["health"]);
        }
        if (entityJSON.contains("stamina"))
        {
            if (!ecs->has_components<staminaComponent>(entity_id))
            {
                ecs->add_component<staminaComponent>(entity_id, {});
            }
            yorcvs::components::deserialize(ecs->get_component<staminaComponent>(entity_id), entityJSON["stamina"]);
        }
        if (entityJSON.contains("sprite"))
        {
            if (!ecs->has_components<spriteComponent>(entity_id))
            {
                ecs->add_component<spriteComponent>(entity_id, {});
            }
            const std::string sprite_path = directory_path + std::string(entityJSON["sprite"]["spriteName"]);
            entityJSON["sprite"]["spriteName"] = sprite_path;
            yorcvs::components::deserialize(ecs->get_component<spriteComponent>(entity_id), entityJSON["sprite"]);
            if (entityJSON["sprite"].contains("animations"))
            {
                if (!ecs->has_components<animationComponent>(entity_id))
                {
                    ecs->add_component<animationComponent>(entity_id, {});
                }
                yorcvs::components::deserialize(ecs->get_component<animationComponent>(entity_id),
                                                entityJSON["sprite"]["animations"]);

                AnimationSystem::set_animation(ecs, entity_id, "idleL");
            }
        }
    }
    void load_character_from_path(yorcvs::Entity &entity, const std::string &path)
    {
        load_character_from_path(entity.id, path);
    }
    /**
     * @brief Serializez an entities components and returns the string reprezentation
     * 
     * @param entity 
     * @return std::string 
     */
    std::string save_character_to_path(const size_t entity) const
    {
        json::json j;
        if (ecs->has_components<identificationComponent>(entity))
        {
            j["name"] = yorcvs::components::serialize(ecs->get_component<identificationComponent>(entity));
        }
        if (ecs->has_components<healthComponent>(entity))
        {
            j["health"] = yorcvs::components::serialize(ecs->get_component<healthComponent>(entity));
        }
        if (ecs->has_components<staminaComponent>(entity))
        {
            j["stamina"] = yorcvs::components::serialize(ecs->get_component<staminaComponent>(entity));
        }
        if (ecs->has_components<hitboxComponent>(entity))
        {
            j["hitbox"] = yorcvs::components::serialize(ecs->get_component<hitboxComponent>(entity));
        }
        if (ecs->has_components<spriteComponent>(entity))
        {
            j["sprite"] = yorcvs::components::serialize(ecs->get_component<spriteComponent>(entity));
            j["sprite"]["animations"] = yorcvs::components::serialize(ecs->get_component<animationComponent>(entity));
        }
        return j.dump(4);
    }

  private:
    void parse_tile_layer(tmx::Map &map, tmx::TileLayer &tileLayer)
    {
        const auto &chunks = tileLayer.getChunks();
        for (const auto &chunk : chunks) // parse chunks
        {
            // std::cout << "Loading chunk: " << chunk.position.x/chunk.size.x << " " << chunk.position.y/chunk.size.y
            // << '\n';
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
                    tiles_chunks[std::make_tuple<intmax_t, intmax_t>(chunk.position.x / chunk.size.x,
                                                                     chunk.position.y / chunk.size.y)]
                        .push_back(tile);
                }
            }
        }
    }
    void parse_tile_layer_ysorted(tmx::Map &map, tmx::TileLayer &tileLayer)
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
                    ysorted_tiles.emplace_back(ecs);
                    const size_t entity = ysorted_tiles[ysorted_tiles.size() - 1].id;
                    ecs->add_component<positionComponent>(
                        entity,
                        {chunk_position * tilesSize +
                         tilesSize * yorcvs::Vec2<float>{static_cast<float>(chunk_x), static_cast<float>(chunk_y)}});
                    ecs->add_component<spriteComponent>(entity, {{0, 0},
                                                                 {static_cast<float>(tile_set->getTileSize().x),
                                                                  static_cast<float>(tile_set->getTileSize().y)},
                                                                 get_src_rect_from_uid(map, chunk.tiles[tileIndex].ID),
                                                                 tile_set->getImagePath()});
                }
            }
        }
    }

    /**
     * THESE FUNCTION ALL BEHAVE THE SAME
     *  RETURN TRUE IF THE PROPERTY EXISTS
     *  RETURN FALSE IF IT'S UNKNOWN
     */
    bool object_handle_property_bool(const size_t entity, const tmx::Property &property, const tmx::Object &object)
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
    [[nodiscard]] bool object_handle_property_float(const size_t entity, const tmx::Property &property) const
    {
        if (property.getName() == "behaviourDT")
        {
            if (!ecs->has_components<behaviourComponent>(entity))
            {
                ecs->add_component<behaviourComponent>(entity, {});
            }
            ecs->get_component<behaviourComponent>(entity).dt = property.getFloatValue();
            ecs->get_component<behaviourComponent>(entity).accumulated = 0.0f;
            return true;
        }
        return false;
    }
    bool object_handle_property_file(const size_t entity, const tmx::Property &property)
    {
        const std::string &filePath = property.getFileValue();
        std::filesystem::path map_file = map_file_path;
        const std::string directory_path = map_file.remove_filename().generic_string();

        if (property.getName() == "entityPath")
        {
            load_character_from_path(entity, directory_path + filePath);
            return true;
        }
        if (property.getName() == "behaviour")
        {
            ecs->add_component<behaviourComponent>(entity, {});
            return true;
        }
        return false;
    }
    [[nodiscard]] bool object_handle_property_int(const size_t entity, const tmx::Property &property) const
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
            ecs->get_component<healthComponent>(entity).max_HP = property.getFloatValue();
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
        return false;
    }
    bool object_handle_property_object(size_t entity, const tmx::Property &property);
    bool object_handle_property_string(size_t entity, const tmx::Property &property);

    void parse_object_layer(tmx::Map &map, tmx::ObjectGroup &objectLayer)
    {
        const auto &objects = objectLayer.getObjects();
        for (const auto &object : objects)
        {
            // create entity
            entities.push_back(ecs->create_entity_ID());
            const size_t entity = entities[entities.size() - 1];
            ecs->add_component<positionComponent>(
                entity, {{object.getPosition().x, object.getPosition().y - object.getAABB().height}});
            if (object.getTileID() != 0 && object.visible())
            {
                const auto *tileSet = get_tileset_containing(map, object.getTileID());
                // add sprite component

                ecs->add_component<spriteComponent>(entity, {{0, 0},
                                                             {object.getAABB().width, object.getAABB().height},
                                                             get_src_rect_from_uid(map, object.getTileID()),
                                                             tileSet->getImagePath()});
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
                case tmx::Property::Type::File:
                    known = object_handle_property_file(entity, property);
                    break;
                case tmx::Property::Type::Float:
                    known = object_handle_property_float(entity, property);
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
    yorcvs::Vec2<float> get_spawn_position() const
    {
        return spawn_coord;
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

  public:
    yorcvs::ECS *ecs{};


    // TODO: MAKE THIS UNNECESSARY
    struct ecs_Initializer
    {
        explicit ecs_Initializer(yorcvs::ECS &world)
        {
            // register components
            world.register_component<hitboxComponent, positionComponent, velocityComponent, healthComponent,
                                     staminaComponent>();
            world.register_component<playerMovementControlledComponent, behaviourComponent>();
            world.register_component<spriteComponent, animationComponent>();
        }
    };
    // class to initialize the ecs before systems are constructed
    ecs_Initializer init_ecs;

  
    CollisionSystem collisionS;
    yorcvs::Vec2<float> tilesSize;

    std::vector<size_t> entities;//not a vector of Entities because the map is not responsible for their lifetimes( they can be destroyed by other stuff)
    HealthSystem healthS;
    StaminaSystem sprintS;

    std::string map_file_path;
    std::unordered_map<std::tuple<intmax_t, intmax_t>, std::vector<yorcvs::Tile>> tiles_chunks;

 
    yorcvs::Vec2<float> spawn_coord;
    VelocitySystem velocityS;
    AnimationSystem animS;

    std::vector<yorcvs::Entity> ysorted_tiles;
};
class DebugInfo
{
  public:
    DebugInfo() = default;

    DebugInfo(yorcvs::Window<yorcvs::graphics> *parentW, yorcvs::Map *map, PlayerMovementControl *pms,
              CollisionSystem *cols, HealthSystem *healthS,sol::state* lua)
        : parentWindow(parentW), appECS(map->ecs), map(map), lua_state(lua), playerMoveSystem(pms),colSystem(cols)
    {
        attach(parentW, map, pms, cols, healthS,lua);
    }
    ~DebugInfo() = default;
    DebugInfo(const DebugInfo &other) = delete;
    DebugInfo(DebugInfo &&other) = delete;
    DebugInfo operator=(const DebugInfo &other) = delete;
    DebugInfo operator=(DebugInfo &&other) = delete;

    void update(const float ft)
    {
        // if (parentWindow->is_key_pressed(yorcvs::YORCVS_KEY_R))  
        // {
        //     reset();
        // }
        // if (parentWindow->is_key_pressed(yorcvs::YORCVS_KEY_C))
        // {
        //     std::cout << "Saving player... \n";
        //     std::ofstream out("assets/testPlayer.json");
        //     out << map->save_character_to_path(playerMoveSystem->entityList->entitiesID[0]);
        //     std::cout << "Done.\n";
        // }
        frame_time = ft;
        avg_frame_time *= frame_time_samples;
        frame_time_samples += 1.0f;
        avg_frame_time += ft;
        avg_frame_time /= frame_time_samples;
        if(!playerMoveSystem->entityList->entitiesID.empty())
        {
            (*lua_state)["playerID"] = playerMoveSystem->entityList->entitiesID[0];
        }
        if (showDebugWindow)
        {
            if (ft > maxFrameTime)
            {
                maxFrameTime = ft;
            }
        }
        if (showConsole)
        {
           
        }
    }

    template <typename render_backend>
    void render_hitboxes(yorcvs::Window<render_backend> &window, const yorcvs::Vec2<float> &render_dimensions,
                         const size_t r, const size_t g, const size_t b, const size_t a)
    {
        yorcvs::Vec2<float> old_rs = window.get_render_scale();
        window.set_render_scale(window.get_size() / render_dimensions);

        yorcvs::Rect<float> rect{};
        for (const auto &ID : colSystem->entityList->entitiesID)
        {
            rect.x = appECS->get_component<positionComponent>(ID).position.x +
                     appECS->get_component<hitboxComponent>(ID).hitbox.x;
            rect.y = appECS->get_component<positionComponent>(ID).position.y +
                     appECS->get_component<hitboxComponent>(ID).hitbox.y;
            rect.w = appECS->get_component<hitboxComponent>(ID).hitbox.w;
            rect.h = appECS->get_component<hitboxComponent>(ID).hitbox.h;
            window.draw_rect(rect, r, g, b, a);
            if (appECS->has_components<healthComponent>(ID))
            {
                /// draw health bar

                yorcvs::Rect<float> healthBarRect{};
                if (appECS->has_components<spriteComponent>(
                        ID)) // if the entity has a sprite component , render the health above it, not above the hitbox
                {
                    healthBarRect.y = rect.y - appECS->get_component<spriteComponent>(ID).size.y / 2;
                }
                else
                {
                    healthBarRect.y = rect.y - rect.h;
                }

                healthBarRect.x = rect.x - 16.0f + rect.w / 2;

                healthBarRect.w = health_full_bar_dimension.x;
                healthBarRect.h = health_full_bar_dimension.y;
                if (appECS->has_components<staminaComponent>(ID))
                {
                    healthBarRect.y -= health_full_bar_dimension.y * 2;
                }
                window.draw_rect(healthBarRect, health_bar_empty_color[0], health_bar_empty_color[1],
                                 health_bar_empty_color[2], health_bar_empty_color[3]);
                healthBarRect.w =
                    (appECS->get_component<healthComponent>(ID).HP / appECS->get_component<healthComponent>(ID).max_HP) *
                    32.0f;
                window.draw_rect(healthBarRect, health_bar_full_color[0], health_bar_full_color[1],
                                 health_bar_full_color[2], health_bar_full_color[3]);
            }
            if (appECS->has_components<staminaComponent>(ID))
            {
                yorcvs::Rect<float> staminaBarRect{};
                if (appECS->has_components<spriteComponent>(
                        ID)) // if the entity has a sprite component , render the health above it, not above the hitbox
                {
                    staminaBarRect.y = rect.y - appECS->get_component<spriteComponent>(ID).size.y / 2;
                }
                else
                {
                    staminaBarRect.y = rect.y - rect.h;
                }

                staminaBarRect.x = rect.x - 16.0f + rect.w / 2;

                staminaBarRect.w = health_full_bar_dimension.x;
                staminaBarRect.h = health_full_bar_dimension.y;
                window.draw_rect(staminaBarRect, stamina_bar_empty_color[0], stamina_bar_empty_color[1],
                                 stamina_bar_empty_color[2], stamina_bar_empty_color[3]);
                staminaBarRect.w = (appECS->get_component<staminaComponent>(ID).stamina /
                                    appECS->get_component<staminaComponent>(ID).max_stamina) *
                                   32.0f;
                window.draw_rect(staminaBarRect, stamina_bar_full_color[0], stamina_bar_full_color[1],
                                 stamina_bar_full_color[2], stamina_bar_full_color[3]);
            }
        }
        window.set_render_scale(old_rs);
    }

    void render(const float elapsed, yorcvs::Vec2<float> &render_dimensions)
    {
        time_accumulator += elapsed;
        if (time_accumulator >= ui_controls_update_time)
        {
            if (parentWindow->is_key_pressed(yorcvs::YORCVS_KEY_LCTRL))
            {
                if (parentWindow->is_key_pressed(yorcvs::YORCVS_KEY_E))
                {
                    showDebugWindow = !showDebugWindow;
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(YORCVS_KEY_TILDE))
                {
                    playerMoveSystem->controls_enable = !playerMoveSystem->controls_enable;
                    showConsole = !showConsole;
                    time_accumulator = 0;
                }
                if (parentWindow->is_key_pressed(yorcvs::YORCVS_KEY_I))
                {
                    render_dimensions -= render_dimensions * zoom_power;
                    time_accumulator = 0;
                }

                if (parentWindow->is_key_pressed(yorcvs::YORCVS_KEY_K))
                {
                    render_dimensions += render_dimensions * zoom_power;
                    time_accumulator = 0;
                }
            }
        }
        update(elapsed);
        if (showDebugWindow)
        {     
            render_hitboxes(*parentWindow, render_dimensions, hitbox_color[0], hitbox_color[1], hitbox_color[2],
                            hitbox_color[3]);
            ImGui::Begin("DebugWindow");
            ImGui::Text("frameTime: %f",frame_time);
            ImGui::Text("maxFramTime: %f",maxFrameTime);
            ImGui::Text("avgFrameTime: %f",avg_frame_time);
            ImGui::Text("ecsEntities: %zu",appECS->get_active_entities_number());
            if (!playerMoveSystem->entityList->entitiesID.empty())
            {
                const size_t ID = playerMoveSystem->entityList->entitiesID[0];
                healthComponent &playerHealthC = appECS->get_component<healthComponent>(ID);
                if (appECS->has_components<positionComponent>(ID))
                {
                 ImGui::Text("playerPosition: (%f,%f)",appECS->get_component<positionComponent>(ID).position.x,appECS->get_component<positionComponent>(ID).position.y);
                }
                if (appECS->has_components<healthComponent>(ID))
                {
                    ImGui::Text("playerHealth: (%f/%f)",playerHealthC.HP,playerHealthC.max_HP);
                }
            }
            ImGui::End();
        }
        if (showConsole)
        {
            ImGui::ShowDemoWindow();
            ImGui::Begin("Console");
            ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue |  ImGuiInputTextFlags_CallbackHistory;

            const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
            for(const auto& item : console_logs)
            {
                ImGui::TextUnformatted( item.c_str());
            }
            ImGui::EndChild();
            ImGui::Separator();
            bool reclaim_focus = false;
            if(ImGui::InputText("##",&console_text,input_text_flags,&DebugInfo::TextEditCallbackStub, (void*)this) && !console_text.empty())
            {  
                console_logs.push_back(console_text);
                HistoryPos = -1;
                console_previous_commands.push_back(console_text);
                auto rez = lua_state->safe_script(console_text,[](lua_State*, sol::protected_function_result pfr) {return pfr;});
                if(!rez.valid())
                {   sol::error err = rez;
                    std::string text = err.what();
                    console_logs.emplace_back(std::move(text));
                }
                else
                {
                    //console_logs.emplace_back(rez.get<std::string>());
                   
                }
                console_text = "";
                reclaim_focus = true;
            }
            ImGui::SetItemDefaultFocus();
            if (reclaim_focus)
                ImGui::SetKeyboardFocusHere(-1);
            ImGui::End();
        }
    }

    void attach(yorcvs::Window<yorcvs::graphics> *parentW, yorcvs::Map *map, PlayerMovementControl *pms,
                CollisionSystem *cols, HealthSystem *healthS,sol::state* lua)
    {
        lua_state = lua;
        attach_lua();
        parentWindow = parentW;
        this->map = map;
        appECS = map->ecs;
        playerMoveSystem = pms;
        colSystem = cols;
        healthSys = healthS;

     
    }
    void attach_lua()
    {
        lua_state->set_function("internal_log",&DebugInfo::add_log,this);
        lua_state->script("function log(message) internal_log(tostring(message)) end");
        (*lua_state)["print"] = (*lua_state)["log"];
    }

    void reset()
    {
        maxFrameTime = 0.0f;
    }

    void add_log(const std::string& message)
    {
        console_logs.push_back(message);
    }
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        DebugInfo* console = static_cast<DebugInfo*>(data->UserData);
        return console->TextEditCallback(data);
    }

    int  TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = console_previous_commands.size() - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= console_previous_commands.size())
                            HistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    const char* history_str = (HistoryPos >= 0) ? console_previous_commands[HistoryPos].c_str() : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
    }    
    std::vector<size_t> callbacks;
    std::string console_input;

    yorcvs::Window<yorcvs::graphics> *parentWindow{};
    yorcvs::ECS *appECS{};
    yorcvs::Map *map{};
    sol::state *lua_state{};
    //debug window
    float frame_time = 0.0f;
    float maxFrameTime = 0.0f;
    float frame_time_samples = 0.0f;
    float avg_frame_time = 0.0f;
    //console
    std::string console_text;
    std::vector<std::string> console_logs;
    std::vector<std::string> console_previous_commands;
    PlayerMovementControl *playerMoveSystem{};

    CollisionSystem *colSystem{};
    HealthSystem *healthSys{};

    // controls
    bool showDebugWindow = false;
    bool showConsole = false;
    float time_accumulator = 0;
    int HistoryPos = 0;
    
    static constexpr yorcvs::Vec2<float> health_full_bar_dimension = {32.0f, 4.0f};
    const std::vector<uint8_t> health_bar_full_color = {255, 0, 0, 255};
    const std::vector<uint8_t> health_bar_empty_color = {100, 0, 0, 255};

    const std::vector<uint8_t> hitbox_color = {255, 0, 0, 100};


    static constexpr size_t textR = 255;
    static constexpr size_t textG = 255;
    static constexpr size_t textB = 255;
    static constexpr size_t textA = 255;
    static constexpr size_t text_char_size = 100;
    static constexpr size_t text_line_length = 10000;

    const std::vector<uint8_t> stamina_bar_full_color = {0, 255, 0, 100};
    const std::vector<uint8_t> stamina_bar_empty_color = {0, 100, 0, 100};

    static constexpr float ui_controls_update_time = 250.0f;
    static constexpr float zoom_power = 0.1f;
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
        lua_state.open_libraries(sol::lib::base, sol::lib::package,sol::lib::math);
        yorcvs::lua::bind_runtime(lua_state, &world);
        //loading two maps one on top of each other
        map.load(&world,"assets/map.tmx");
        map.load(&world,"assets/map2.tmx");
        size_t player_id = world.create_entity_ID();
        map.load_character_from_path(player_id, "assets/player.json");
        world.add_default_component<playerMovementControlledComponent>(player_id);

        dbInfo.attach(&r, &map, &pcS, &map.collisionS, &map.healthS, &lua_state);
        counter.start();
    }
    Application(const Application &other) = delete;
    Application(Application &&other) = delete;
    Application &operator=(const Application &other) = delete;
    Application &operator=(Application &&other) = delete;

    void update(float dt)
    {
        map.update(dt);
    }
    void render_map_chunk(yorcvs::Map &p_map, const std::tuple<intmax_t, intmax_t> &chunk)
    {
        if (p_map.tiles_chunks.find(chunk) != p_map.tiles_chunks.end())
        {
            const auto &tiles = p_map.tiles_chunks.at(chunk);
            for (const auto &tile : tiles)
            {
                r.draw_sprite(tile.texture_path, {tile.coords.x, tile.coords.y, p_map.tilesSize.x, p_map.tilesSize.y},
                              tile.srcRect);
            }
        }
    }
    void render_map_tiles(yorcvs::Map &p_map)
    {
        yorcvs::Vec2<float> render_scale = r.get_render_scale();
        r.set_render_scale(r.get_size() / render_dimensions);
        // get player position
        const size_t entity_ID = pcS.entityList->entitiesID[0];
        const yorcvs::Vec2<float> player_position = world.get_component<positionComponent>(entity_ID).position;
        const std::tuple<intmax_t, intmax_t> player_position_chunk = std::tuple<intmax_t, intmax_t>(
            std::floor(player_position.x / (32.0f * 16.0f)), std::floor(player_position.y / (32.0f * 16.0f)));
        // render chunks

        std::tuple<intmax_t, intmax_t> chunk_to_be_rendered{};
        for (intmax_t x = render_distance * -1; x <= render_distance; x++)
        {
            for (intmax_t y = -1 * render_distance; y <= render_distance; y++)
            {
                chunk_to_be_rendered = std::make_tuple<intmax_t, intmax_t>(std::get<0>(player_position_chunk) + x,
                                                                           std::get<1>(player_position_chunk) + y);
                render_map_chunk(p_map, chunk_to_be_rendered);
            }
        }

        r.set_render_scale(render_scale);
    }
    void run()
    {
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        const float elapsed = std::min(100.0f, counter.get_ticks<float, std::chrono::nanoseconds>() / 1000000.0f);
        counter.stop();
        counter.start();

        lag += elapsed;

        r.handle_events();
        while (lag >= msPF)
        {
            update(msPF);
            bhvS.update(msPF);
            pcS.updateControls(render_dimensions, msPF);
            lag -= msPF;
        }

        
        r.clear();
        
        render_map_tiles(map);
        sprS.renderSprites(render_dimensions);
        dbInfo.render(elapsed, render_dimensions);
              
       
        ImGui::Render();
        ImGuiSDL::Render(ImGui::GetDrawData());
        r.present();

    }

    [[nodiscard]] bool is_active() const
    {
        return r.is_active();
    }

    ~Application()
    {
        r.cleanup();
    }

  private:
    static constexpr yorcvs::Vec2<float> default_render_dimensions = {240.0f, 120.0f};
    static constexpr float msPF = 41.6f;
    static constexpr intmax_t default_render_distance = 1;

    yorcvs::Window<yorcvs::graphics> r;
    yorcvs::Timer counter;

    float lag = 0.0f;
    yorcvs::Vec2<float> render_dimensions = default_render_dimensions; // how much to render
    intmax_t render_distance = default_render_distance;
    yorcvs::ECS world{};
    sol::state lua_state;
    yorcvs::Map map{&world};
    SpriteSystem sprS{map.ecs, &r};
    PlayerMovementControl pcS{map.ecs, &r};
    BehaviourSystem bhvS{map.ecs,&lua_state};

    DebugInfo dbInfo;
};
} // namespace yorcvs