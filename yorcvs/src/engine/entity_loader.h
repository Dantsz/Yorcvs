#include "../common/ecs.h"
#include "../game/componentSerialization.h"
#include <filesystem>
#pragma once

template <typename... Components>
class Entity_Loader {
public:
    Entity_Loader(yorcvs::ECS* world)
        : world { world }
    {
    }
    /**
     * @brief Loads json entity data into the entity
     *
     * @param entity_id id of the entity
     * @param path path to the json file
     */
    void load_character_from_path(size_t entity_id, const std::string& path)
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
        if (!deserialize_component_from_json<healthStatsComponent>(entity_id, entityJSON, "health_stats")) {
            yorcvs::log("healthComponent (" + path + ") is not valid");
            return;
        }
        if (!deserialize_component_from_json<staminaComponent>(entity_id, entityJSON, "stamina")) {
            yorcvs::log("staminaComponent (" + path + ") is not valid");
            return;
        }
        if (!deserialize_component_from_json<staminaStatsComponent>(entity_id, entityJSON, "stamina_stats")) {
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
        if (!deserialize_component_from_json<spriteComponent>(entity_id, entityJSON, "sprite")) {
            yorcvs::log("spriteComponent (" + path + ") is not valid");
            return;
        }
        if (!deserialize_component_from_json<animationComponent>(entity_id, entityJSON, "animation")) {
            yorcvs::log("AnimationComponent (" + path + ") is not valid");
            return;
        } else {
            const std::string sprite_path = directory_path + world->get_component<spriteComponent>(entity_id).texture_path;
            world->get_component<spriteComponent>(entity_id).texture_path = sprite_path;
        }
        OnCharacterDeserialized(entity_id);
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

        serialize_component_to_json<identificationComponent>(entity, "identification", j);
        serialize_component_to_json<healthComponent>(entity, "health", j);
        serialize_component_to_json<healthStatsComponent>(entity, "health_stats", j);
        serialize_component_to_json<staminaComponent>(entity, "stamina", j);
        serialize_component_to_json<staminaStatsComponent>(entity, "stamina_stats", j);
        serialize_component_to_json<hitboxComponent>(entity, "hitbox", j);
        serialize_component_to_json<spriteComponent>(entity, "sprite", j);
        serialize_component_to_json<animationComponent>(entity, "animations", j);
        serialize_component_to_json<defensiveStatsComponent>(entity, "defensive_stats", j);
        serialize_component_to_json<offensiveStatsComponent>(entity, "offsensive_stats", j);
        return j.dump(4);
    }

protected:
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
        const size_t entity_id, json::json& json_entity_obj, const std::string& component_name)
    {
        if (json_entity_obj.contains(component_name)) {
            T comp {};
            if (!yorcvs::components::deserialize(comp,
                    json_entity_obj[component_name])) {
                return false;
            }
            if (!world->has_components<T>(entity_id)) {
                world->add_component<T>(entity_id, comp);
            }
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
        if (world->has_components<T>(entity_id)) {
            transform(json_obj, world->get_component<T>(entity_id));
            json_obj[component_name] = yorcvs::components::serialize(world->get_component<T>(entity_id));
        }
    }
    virtual void OnCharacterDeserialized([[maybe_unused]] size_t entity_id) {};
    yorcvs::ECS* world;
};
