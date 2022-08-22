#pragma once
#include "../common/ecs.h"
#include "../common/utilities.h"
#include "../game/component_serialization.h"
#include "../game/systems.h"
#include "entity_loader.h"
#include "nlohmann/json.hpp"
#include "tmxlite/Layer.hpp"
#include "tmxlite/Map.hpp"
#include "tmxlite/Object.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/Property.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/Tileset.hpp"
#include <filesystem>
namespace json = nlohmann;
namespace yorcvs {
struct tile {
    yorcvs::vec2<float> coords;
    yorcvs::rect<size_t> srcRect;
    std::string texture_path;
};
/**
 * @brief Loads tmx map data into the ecs
 *
 */
class map : public entity_loader<identification_component, health_component, health_stats_component, stamina_component, stamina_stats_component, hitbox_component, sprite_component, animation_component, defensive_stats_component, offensive_stats_component> {
public:
    explicit map(yorcvs::ECS* world)
        : entity_loader(world, { "identification", "health", "health_stats", "stamina", "stamina_stats", "hitbox", "sprite", "animation", "defensive_stats", "offsensive_stats" })
        , ecs(world)
        , init_ecs(*world)
        , health_sys(world)
        , sprint_sys(world)
        , velocity_sys(world)
        , animation_sys(world)
        , combat_sys(world)
        , collision_sys(world)
    {
    }
    /**
     * @brief Construct a new Map object
     *
     * @param path path to map file
     * @param world the ecs in which data shoudl be added
     */
    map(const std::string& path, yorcvs::ECS* world)
        : map(world)
    {
        load(world, path);
    }
    map(const map& other) = delete;
    map(map&& other) = delete;
    ~map()
    {
        clear();
    }
    map& operator=(const map& other) = delete;
    map& operator=(map&& other) = delete;
    /**
     * @brief Loads data from path into the ecs
     *
     * @param parent
     * @param path
     */
    void load(yorcvs::ECS* parent, const std::string& path)
    {
        ecs = parent;
        map_file_path = path;
        yorcvs::log("Loading map: " + path);
        tmx::Map map {};
        if (!map.load(path)) {
            yorcvs::log("Map loading failed", yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        const auto& tilesets = map.getTilesets();
        yorcvs::log("Map contains " + std::to_string(tilesets.size()) + " tile sets: ");
        for (const auto& tileset : tilesets) {
            yorcvs::log(tileset.getImagePath());
        }
        if (!map.isInfinite()) {
            yorcvs::log("Cannot load finite  maps", yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        tilesSize = { static_cast<float>(map.getTileSize().x), static_cast<float>(map.getTileSize().y) };
        const auto& layers = map.getLayers();

        for (const auto& layer : layers) // parse layers
        {
            const auto& properties = layer->getProperties();
            bool tiles_ysorted = false;
            switch (layer->getType()) {
            case tmx::Layer::Type::Tile:
                for (const auto& property : properties) {
                    if (property.getName() == "Ysorted") {
                        tiles_ysorted = true;
                    }
                }
                if (tiles_ysorted) {
                    parse_tile_layer_ysorted(map, layer->getLayerAs<tmx::TileLayer>());
                } else {
                    parse_tile_layer(map, layer->getLayerAs<tmx::TileLayer>());
                }
                break;
            case tmx::Layer::Type::Object:
                parse_object_layer(map, layer->getLayerAs<tmx::ObjectGroup>());
                break;
            // case tmx::Layer::Type::Image:
            //     break;
            // case tmx::Layer::Type::Group:
            default:
                break;
            }
        }
    }

    /**
     * @brief Removes all entitites and tiles loaded by this map
     *
     */
    void clear() noexcept
    {
        for (const auto& entity : entities) {
            if (ecs->is_valid_entity(entity)) {
                ecs->destroy_entity(entity);
            }
        }
        entities.clear();
        ysorted_tiles.clear();
        tiles_chunks.clear();
    }

private:
    void parse_tile_layer(tmx::Map& map, tmx::TileLayer& tileLayer)
    {
        const auto& chunks = tileLayer.getChunks();
        for (const auto& chunk : chunks) // parse chunks
        {
            yorcvs::vec2<float> chunk_position = { static_cast<float>(chunk.position.x),
                static_cast<float>(chunk.position.y) };
            for (auto chunk_y = 0; chunk_y < chunk.size.y; chunk_y++) {
                for (auto chunk_x = 0; chunk_x < chunk.size.x; chunk_x++) {
                    // parse tiles
                    const size_t tileIndex = chunk_y * chunk.size.x + chunk_x;
                    if (chunk.tiles[tileIndex].ID == 0) {
                        continue;
                    }
                    // chunk.tiles[tileIndex].ID;
                    // find tileset
                    tmx::Tileset const* tile_set = nullptr;
                    for (const auto& tileset : map.getTilesets()) {
                        if (tileset.hasTile(chunk.tiles[tileIndex].ID)) {
                            tile_set = &tileset;
                        }
                    }
                    // put the tile in the vector
                    yorcvs::tile tile {};
                    if (tile_set == nullptr) {
                        yorcvs::log("No tileset in map " + map.getWorkingDirectory() + "  contains tile: " + std::to_string(chunk.tiles[tileIndex].ID), yorcvs::MSGSEVERITY::ERROR);
                    } else {
                        tile.texture_path = tile_set->getImagePath();
                    }
                    tile.coords = chunk_position * tilesSize + tilesSize * yorcvs::vec2<float> { static_cast<float>(chunk_x), static_cast<float>(chunk_y) };
                    tile.srcRect = get_src_rect_from_uid(map, chunk.tiles[tileIndex].ID);
                    tiles_chunks[std::make_tuple<intmax_t, intmax_t>(chunk.position.x / chunk.size.x,
                                     chunk.position.y / chunk.size.y)]
                        .push_back(tile);
                }
            }
        }
    }
    void parse_tile_layer_ysorted(tmx::Map& map, tmx::TileLayer& tileLayer)
    {
        const auto& chunks = tileLayer.getChunks();
        for (const auto& chunk : chunks) // parse chunks
        {
            yorcvs::vec2<float> chunk_position = { static_cast<float>(chunk.position.x),
                static_cast<float>(chunk.position.y) };
            for (auto chunk_y = 0; chunk_y < chunk.size.y; chunk_y++) {
                for (auto chunk_x = 0; chunk_x < chunk.size.x; chunk_x++) {
                    // parse tiles
                    const size_t tileIndex = chunk_y * chunk.size.x + chunk_x;
                    if (chunk.tiles[tileIndex].ID == 0) {
                        continue;
                    }
                    // chunk.tiles[tileIndex].ID;
                    // find tileset
                    tmx::Tileset const* tile_set = nullptr;
                    for (const auto& tileset : map.getTilesets()) {
                        if (tileset.hasTile(chunk.tiles[tileIndex].ID)) {
                            tile_set = &tileset;
                        }
                    }

                    // add object
                    ysorted_tiles.emplace_back(ecs);
                    const size_t entity = ysorted_tiles[ysorted_tiles.size() - 1].id;
                    ecs->add_component<position_component>(
                        entity,
                        { chunk_position * tilesSize + tilesSize * yorcvs::vec2<float> { static_cast<float>(chunk_x), static_cast<float>(chunk_y) } });
                    ecs->add_component<sprite_component>(entity, { { 0, 0 }, { static_cast<float>(tile_set->getTileSize().x), static_cast<float>(tile_set->getTileSize().y) }, get_src_rect_from_uid(map, chunk.tiles[tileIndex].ID), tile_set->getImagePath() });
                }
            }
        }
    }

    /**
     * THESE FUNCTION ALL BEHAVE THE SAME
     *  RETURN TRUE IF THE PROPERTY EXISTS
     *  RETURN FALSE IF IT'S UNKNOWN
     */
    bool object_handle_property_bool(const size_t entity, const tmx::Property& property, const tmx::Object& object)
    {
        // Note: handles hitbox to object
        if (property.getName() == "collision" && property.getBoolValue()) {
            ecs->add_component<hitbox_component>(entity, { { 0, 0, object.getAABB().width, object.getAABB().height } });
            // TILED HAS A WEIRD BEHAVIOUR THAT IF AN TILE IS INSERTED AS A OBJECT IT'S Y POSITION IS DIFFERENT
            // FROM AN RECTANGLE OBJECT AND DOESN'T LOOK LIKE IN THE EDITOR
            if (!ecs->has_components<sprite_component>(entity)) {
                ecs->get_component<hitbox_component>(entity).hitbox.y += object.getAABB().height;
            }
            return true;
        }
        // NOTE: handles player spawn area
        if (property.getName() == "playerSpawn" && property.getBoolValue()) {
            spawn_coord = { object.getPosition().x, object.getPosition().y };
            return true;
        }
        return false;
    }
    bool object_handle_property_color(size_t entity, const tmx::Property& property);
    [[nodiscard]] bool object_handle_property_float(const size_t entity, const tmx::Property& property) const
    {
        if (property.getName() == "behaviourDT") {
            if (!ecs->has_components<behaviour_component>(entity)) {
                ecs->add_component<behaviour_component>(entity, {});
            }
            ecs->get_component<behaviour_component>(entity).dt = property.getFloatValue();
            ecs->get_component<behaviour_component>(entity).accumulated = 0.0f;
            return true;
        }
        return false;
    }
    bool object_handle_property_file(const size_t entity, const tmx::Property& property)
    {
        const std::string& filePath = property.getFileValue();
        std::filesystem::path map_file = map_file_path;
        const std::string directory_path = map_file.remove_filename().generic_string();

        if (property.getName() == "entityPath") {
            load_entity_from_path(entity, directory_path + filePath);
            return true;
        }
        if (property.getName() == "behaviour") {
            ecs->add_component<behaviour_component>(entity, {});
            if (filePath.empty()) {
                yorcvs::log("Entity " + std::to_string(entity) + " has not been specified a valid behaviour, using default", yorcvs::MSGSEVERITY::ERROR);
                ecs->get_component<behaviour_component>(entity).code_path = directory_path + "scripts/behavior_chicken.lua";
                return true;
            }
            ecs->get_component<behaviour_component>(entity).code_path = directory_path + filePath;
            return true;
        }
        return false;
    }
    [[nodiscard]] bool object_handle_property_int([[maybe_unused]] const size_t entity, [[maybe_unused]] const tmx::Property& property) const
    {
        return false;
    }
    bool object_handle_property_object(size_t entity, const tmx::Property& property);
    bool object_handle_property_string(size_t entity, const tmx::Property& property);

    void parse_object_layer(tmx::Map& map, tmx::ObjectGroup& objectLayer)
    {
        const auto& objects = objectLayer.getObjects();
        for (const auto& object : objects) {
            // create entity
            entities.push_back(ecs->create_entity_ID());
            const size_t entity = entities[entities.size() - 1];
            ecs->add_component<position_component>(
                entity, { { object.getPosition().x, object.getPosition().y - object.getAABB().height } });
            if (object.getTileID() != 0 && object.visible()) {
                const auto* tileSet = get_tileset_containing(map, object.getTileID());
                // add sprite component

                ecs->add_component<sprite_component>(entity, { { 0, 0 }, { object.getAABB().width, object.getAABB().height }, get_src_rect_from_uid(map, object.getTileID()), tileSet->getImagePath() });
            }
            /*
            Object properties
            * collision - object has collision
            * playerSpawn - objects' coordinates are where the player can spawn
            * HP - health
            * HP_max - maximum hp
            * HP_Regen - health regeneration
            */
            for (const auto& property : object.getProperties()) {
                bool known = false;
                switch (property.getType()) {
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
                if (!known) {
                    yorcvs::log("UNKNOWN PROPERTY " + property.getName() + " of object " + std::to_string(object.getUID()),
                        yorcvs::MSGSEVERITY::WARNING);
                }
            }
        }
    }
    [[nodiscard]] yorcvs::vec2<float> get_spawn_position() const
    {
        return spawn_coord;
    }
    void OnCharacterDeserialized(size_t entity_id, const std::string& path) override
    {
        std::filesystem::path file = path;
        const std::string directory_path = file.remove_filename().generic_string();
        if (world->has_components<sprite_component>(entity_id)) {
            const std::string sprite_path = directory_path + world->get_component<sprite_component>(entity_id).texture_path;
            world->get_component<sprite_component>(entity_id).texture_path = sprite_path;
        }
        // These components should not be serialized as the position and velocity is relative to the map!!!
        if (!world->has_components<position_component>(entity_id)) {
            world->add_component<position_component>(entity_id, {});
            const auto spawn_position = get_spawn_position();
            world->get_component<position_component>(entity_id).position = spawn_position;
        }
        if (!world->has_components<velocity_component>(entity_id)) {
            world->add_component<velocity_component>(entity_id, {});
        }
        world->get_component<velocity_component>(entity_id) = { { 0.0f, 0.0f }, { false, false } };
    }
    // NOTE: this can be a free function
    static yorcvs::rect<size_t> get_src_rect_from_uid(const tmx::Map& map, const size_t UID)
    {
        tmx::Tileset const* tile_set = nullptr;
        for (const auto& tileset : map.getTilesets()) {
            if (tileset.hasTile(static_cast<uint32_t>(UID))) {
                tile_set = &tileset;
            }
        }
        if (tile_set == nullptr) {
            yorcvs::log(std::string("failed to find a tileset matching the uid: ") + std::to_string(UID),
                yorcvs::MSGSEVERITY::ERROR);
            return { 0, 0, 0, 0 };
        }
        yorcvs::rect<size_t> srcRect {};
        srcRect.x = ((UID - tile_set->getFirstGID()) % tile_set->getColumnCount()) * tile_set->getTileSize().x;
        srcRect.y = 0;
        size_t y_index = 0;
        while (y_index + tile_set->getColumnCount() <= UID - tile_set->getFirstGID()) {
            y_index += tile_set->getColumnCount();
            srcRect.y += tile_set->getTileSize().y;
        }
        srcRect.w = tile_set->getTileSize().x;
        srcRect.h = tile_set->getTileSize().y;
        return srcRect;
    }

    static tmx::Tileset const* get_tileset_containing(tmx::Map& map, const size_t tile_UID)
    {
        tmx::Tileset const* tile_set = nullptr;
        for (const auto& tileset : map.getTilesets()) {
            if (tileset.hasTile(static_cast<uint32_t>(tile_UID))) {
                tile_set = &tileset;
            }
        }
        if (tile_set == nullptr) {
            yorcvs::log("no tileset contains tile ID : " + std::to_string(tile_UID));
        }
        return tile_set;
    }

public:
    yorcvs::ECS* ecs {};

    // TODO: MAKE THIS UNNECESSARY
    struct ecs_Initializer {
        explicit ecs_Initializer(yorcvs::ECS& world)
        {
            // register components
            world.register_component<identification_component>();
            world.register_component<hitbox_component, position_component, velocity_component, health_component,
                stamina_component>();
            world.register_component<player_movement_controlled_component, behaviour_component>();
            world.register_component<sprite_component, animation_component>();
            world.register_component<health_stats_component, stamina_stats_component, offensive_stats_component, defensive_stats_component>();
        }
    };
    // class to initialize the ecs before systems are constructed
    ecs_Initializer init_ecs;

    yorcvs::vec2<float> tilesSize;

    std::vector<size_t> entities {}; // not a vector of Entities because the map is not responsible for their lifetimes( they can be destroyed by other stuff)
    health_system health_sys;
    stamina_system sprint_sys;

    std::string map_file_path;
    std::unordered_map<std::tuple<intmax_t, intmax_t>, std::vector<yorcvs::tile>> tiles_chunks {};

    yorcvs::vec2<float> spawn_coord;
    velocity_system velocity_sys;
    animation_system animation_sys;
    combat_system combat_sys;
    collision_system collision_sys;
    std::vector<yorcvs::entity> ysorted_tiles {};
};
}
