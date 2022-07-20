#pragma once
#include "nlohmann/json.hpp"
#include "tmxlite/Layer.hpp"
#include "tmxlite/Map.hpp"
#include <filesystem>

#include "../common/ecs.h"
#include "../common/utilities.h"
#include "../game/componentSerialization.h"
#include "../game/systems.h"
#include "tmxlite/Object.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/Property.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/Tileset.hpp"
namespace json = nlohmann;
namespace yorcvs {
struct Tile {
    yorcvs::Vec2<float> coords;
    yorcvs::Rect<size_t> srcRect;
    std::string texture_path;
};
/**
 * @brief Loads tmx map data into the ecs
 *
 */
class Map {
public:
    explicit Map(yorcvs::ECS* world)
        : ecs(world)
        , init_ecs(*world)
        , health_system(world)
        , sprint_system(world)
        , velocity_system(world)
        , animation_system(world)
        , combat_system(world)
        , collision_system(world)
    {
    }
    /**
     * @brief Construct a new Map object
     *
     * @param path path to map file
     * @param world the ecs in which data shoudl be added
     */
    Map(const std::string& path, yorcvs::ECS* world)
        : Map(world)
    {
        load(world, path);
    }
    Map(const Map& other) = delete;
    Map(Map&& other) = delete;
    ~Map()
    {
        clear();
    }
    Map& operator=(const Map& other) = delete;
    Map& operator=(Map&& other) = delete;
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
    /**
     * @brief Checks if the passed json object contains an compoennt of the specified name, tests if the entity has the component, if it doesn't it's default constructed, and deserealizez the data to it
     *
     * @tparam T
     * @param entity_id
     * @param json_entity_obj
     * @param component_name
     * @param transform function to be applied to the component after it has been added
     * @return return false on parsing failure
     */
    template <typename T>
    [[nodiscard]] bool deserialize_component_from_json(
        const size_t entity_id, json::json& json_entity_obj, const std::string& component_name, [[maybe_unused]] std::function<void(T&)> transform = [](T&) {})
    {
        if (json_entity_obj.contains(component_name)) {
            T comp {};
            if (!yorcvs::components::deserialize(comp,
                    json_entity_obj[component_name])) {
                return false;
            }
            if (!ecs->has_components<T>(entity_id)) {
                ecs->add_component<T>(entity_id, comp);
            }
            transform(ecs->get_component<T>(entity_id));
        }
        return true;
    }
    /**
     * @brief serializes the component and adds it to the json to the as an object with the name <component_name>
     *
     * @tparam T
     * @param entity_id
     * @param component_name
     * @param json_obj
     * @param transform transform json before serialization
     */
    template <typename T>
    void serialize_component_to_json(
        const size_t entity_id, const std::string& component_name, json::json& json_obj, [[maybe_unused]] std::function<void(json::json&, const T&)> transform = [](json::json&, const T&) {}) const
    {
        if (ecs->has_components<T>(entity_id)) {
            transform(json_obj, ecs->get_component<T>(entity_id));
            json_obj[component_name] = yorcvs::components::serialize(ecs->get_component<T>(entity_id));
        }
    }
    /**
     * @brief Loads json entity data into the entity
     *
     * @param entity_id id of the entity
     * @param path path to the json file
     */
    void load_character_from_path(const size_t entity_id, const std::string& path)
    {
        std::filesystem::path file = path;
        const std::string directory_path = file.remove_filename().generic_string();

        std::ifstream entityIN(path);
        std::string entityDATA { (std::istreambuf_iterator<char>(entityIN)), (std::istreambuf_iterator<char>()) };
        auto entityJSON = json::json::parse(entityDATA, nullptr, false); // don't throw exception
        if (entityJSON.is_discarded()) {
            yorcvs::log("Failed to load entity data " + path + " !");
            return;
        }
        if (!deserialize_component_from_json<identificationComponent>(entity_id, entityJSON, "identification")) {
            yorcvs::log("identificationComponent (" + path + ") is not valid");
            return;
        }
        if (!deserialize_component_from_json<hitboxComponent>(entity_id, entityJSON, "hitbox")) {
            yorcvs::log("hitboxComponent (" + path + ") is not valid");
            return;
        }
        if (!deserialize_component_from_json<healthComponent>(entity_id, entityJSON, "health")) {
            yorcvs::log("healthComponent (" + path + ") is not valid");
            return;
        }
        if (!deserialize_component_from_json<staminaComponent>(entity_id, entityJSON, "stamina")) {
            yorcvs::log("staminaComponent (" + path + ") is not valid");
            return;
        }
        if (!deserialize_component_from_json<offensiveStatsComponent>(entity_id, entityJSON, "offsensive_stats")) {
            yorcvs::log("offensiveStatsComponent (" + path + ") is not valid");
            return;
        }
        if (!deserialize_component_from_json<defensiveStatsComponent>(entity_id, entityJSON, "defensive_stats")) {
            yorcvs::log("defensiveStatsComponent (" + path + ") is not valid");
            return;
        }
        if (!deserialize_component_from_json<spriteComponent>(entity_id, entityJSON, "sprite", [&](spriteComponent& /*spr*/) {
                const std::string sprite_path = directory_path + std::string(entityJSON["sprite"]["spriteName"]);
                ecs->get_component<spriteComponent>(entity_id).texture_path = sprite_path;
                if (entityJSON["sprite"].contains("animations"))
                {
                    if (!ecs->has_components<animationComponent>(entity_id))
                    {
                        ecs->add_component<animationComponent>(entity_id, {});
                    }
                    if(!yorcvs::components::deserialize(ecs->get_component<animationComponent>(entity_id),
                        entityJSON["sprite"]["animations"]))
                    {
                      yorcvs::log("animationComponent is not valid");
                      return;
                    }
                    AnimationSystem::set_animation(ecs, entity_id, "idleR");
                } })) {
            yorcvs::log("spriteComponent (" + path + ") is not valid");
            return;
        }
        // These components should not be serialized as the position and velocity is relative to the map!!!
        if (!ecs->has_components<positionComponent>(entity_id)) {
            ecs->add_component<positionComponent>(entity_id, {});
            // TODO: move this elsewhere
            ecs->get_component<positionComponent>(entity_id) = { get_spawn_position() };
        }
        if (!ecs->has_components<velocityComponent>(entity_id)) {
            ecs->add_component<velocityComponent>(entity_id, {});
        }
        ecs->get_component<velocityComponent>(entity_id) = { { 0.0f, 0.0f }, { false, false } };
    }
    void load_character_from_path(yorcvs::Entity& entity, const std::string& path)
    {
        load_character_from_path(entity.id, path);
    }

    /**
     * @brief Serializes an entities components and returns the string reprezentation
     *
     * @param entity
     * @return std::string
     */
    [[nodiscard]] std::string save_character(const size_t entity) const
    {
        json::json j;

        serialize_component_to_json<identificationComponent>(entity, "name", j);
        serialize_component_to_json<healthComponent>(entity, "health", j);
        serialize_component_to_json<staminaComponent>(entity, "stamina", j);
        serialize_component_to_json<hitboxComponent>(entity, "hitbox", j);
        serialize_component_to_json<spriteComponent>(entity, "sprite", j,
            [&](json::json& /*json_object*/, const spriteComponent&) { // if sprite is serialized, also serialize sprites
                serialize_component_to_json<animationComponent>(entity, "animations", j["sprite"]);
            });
        serialize_component_to_json<defensiveStatsComponent>(entity, "defensive_stats", j);
        serialize_component_to_json<offensiveStatsComponent>(entity, "offsensive_stats", j);
        return j.dump(4);
    }

private:
    void parse_tile_layer(tmx::Map& map, tmx::TileLayer& tileLayer)
    {
        const auto& chunks = tileLayer.getChunks();
        for (const auto& chunk : chunks) // parse chunks
        {
            yorcvs::Vec2<float> chunk_position = { static_cast<float>(chunk.position.x),
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
                    yorcvs::Tile tile {};
                    if (tile_set == nullptr) {
                        yorcvs::log("No tileset in map " + map.getWorkingDirectory() + "  contains tile: " + std::to_string(chunk.tiles[tileIndex].ID), yorcvs::MSGSEVERITY::ERROR);
                    } else {
                        tile.texture_path = tile_set->getImagePath();
                    }
                    tile.coords = chunk_position * tilesSize + tilesSize * yorcvs::Vec2<float> { static_cast<float>(chunk_x), static_cast<float>(chunk_y) };
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
            yorcvs::Vec2<float> chunk_position = { static_cast<float>(chunk.position.x),
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

                    /// add object
                    ysorted_tiles.emplace_back(ecs);
                    const size_t entity = ysorted_tiles[ysorted_tiles.size() - 1].id;
                    ecs->add_component<positionComponent>(
                        entity,
                        { chunk_position * tilesSize + tilesSize * yorcvs::Vec2<float> { static_cast<float>(chunk_x), static_cast<float>(chunk_y) } });
                    ecs->add_component<spriteComponent>(entity, { { 0, 0 }, { static_cast<float>(tile_set->getTileSize().x), static_cast<float>(tile_set->getTileSize().y) }, get_src_rect_from_uid(map, chunk.tiles[tileIndex].ID), tile_set->getImagePath() });
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
            ecs->add_component<hitboxComponent>(entity, { { 0, 0, object.getAABB().width, object.getAABB().height } });
            // TILED HAS A WEIRD BEHAVIOUR THAT IF AN TILE IS INSERTED AS A OBJECT IT'S Y POSITION IS DIFFERENT
            // FROM AN RECTANGLE OBJECT AND DOESN'T LOOK LIKE IN THE EDITOR
            if (!ecs->has_components<spriteComponent>(entity)) {
                ecs->get_component<hitboxComponent>(entity).hitbox.y += object.getAABB().height;
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
            if (!ecs->has_components<behaviourComponent>(entity)) {
                ecs->add_component<behaviourComponent>(entity, {});
            }
            ecs->get_component<behaviourComponent>(entity).dt = property.getFloatValue();
            ecs->get_component<behaviourComponent>(entity).accumulated = 0.0f;
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
            load_character_from_path(entity, directory_path + filePath);
            return true;
        }
        if (property.getName() == "behaviour") {
            ecs->add_component<behaviourComponent>(entity, {});
            if (filePath.empty()) {
                yorcvs::log("Entity " + std::to_string(entity) + " has not been specified a valid behaviour, using default", yorcvs::MSGSEVERITY::ERROR);
                ecs->get_component<behaviourComponent>(entity).code_path = directory_path + "scripts/behavior_chicken.lua";
                return true;
            }
            ecs->get_component<behaviourComponent>(entity).code_path = directory_path + filePath;
            return true;
        }
        return false;
    }
    [[nodiscard]] bool object_handle_property_int(const size_t entity, const tmx::Property& property) const
    {
        // NOTE HANDLES HP
        if (property.getName() == "HP") {
            if (!ecs->has_components<healthComponent>(entity)) {
                ecs->add_component<healthComponent>(entity, {});
            }
            ecs->get_component<healthComponent>(entity).HP = property.getFloatValue();
            return true;
        }
        if (property.getName() == "HP_max") {
            if (!ecs->has_components<healthComponent>(entity)) {
                ecs->add_component<healthComponent>(entity, {});
            }
            ecs->get_component<healthComponent>(entity).max_HP = property.getFloatValue();
            return true;
        }
        if (property.getName() == "HP_regen") {
            if (!ecs->has_components<healthComponent>(entity)) {
                ecs->add_component<healthComponent>(entity, {});
            }
            ecs->get_component<healthComponent>(entity).health_regen = property.getFloatValue();
            return true;
        }
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
            ecs->add_component<positionComponent>(
                entity, { { object.getPosition().x, object.getPosition().y - object.getAABB().height } });
            if (object.getTileID() != 0 && object.visible()) {
                const auto* tileSet = get_tileset_containing(map, object.getTileID());
                // add sprite component

                ecs->add_component<spriteComponent>(entity, { { 0, 0 }, { object.getAABB().width, object.getAABB().height }, get_src_rect_from_uid(map, object.getTileID()), tileSet->getImagePath() });
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
    [[nodiscard]] yorcvs::Vec2<float> get_spawn_position() const
    {
        return spawn_coord;
    }

    // NOTE: this can be a free function
    static yorcvs::Rect<size_t> get_src_rect_from_uid(const tmx::Map& map, const size_t UID)
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
        yorcvs::Rect<size_t> srcRect {};
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
            world.register_component<identificationComponent>();
            world.register_component<hitboxComponent, positionComponent, velocityComponent, healthComponent,
                staminaComponent>();
            world.register_component<playerMovementControlledComponent, behaviourComponent>();
            world.register_component<spriteComponent, animationComponent>();
            world.register_component<offensiveStatsComponent, defensiveStatsComponent>();
        }
    };
    // class to initialize the ecs before systems are constructed
    ecs_Initializer init_ecs;

    yorcvs::Vec2<float> tilesSize;

    std::vector<size_t> entities {}; // not a vector of Entities because the map is not responsible for their lifetimes( they can be destroyed by other stuff)
    HealthSystem health_system;
    StaminaSystem sprint_system;

    std::string map_file_path;
    std::unordered_map<std::tuple<intmax_t, intmax_t>, std::vector<yorcvs::Tile>> tiles_chunks {};

    yorcvs::Vec2<float> spawn_coord;
    VelocitySystem velocity_system;
    AnimationSystem animation_system;
    CombatSystem combat_system;
    CollisionSystem collision_system;
    std::vector<yorcvs::Entity> ysorted_tiles {};
};
}
