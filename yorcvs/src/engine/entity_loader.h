#include "../common/ecs.h"
#include "serialization.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <tuple>
#include <unordered_map>
#pragma once

template <typename... Components>
class entity_loader {
public:
    /**
     * @brief Entity_Loader
     * @param world
     * @param json_names names for each component as they appear in the json file
     */
    constexpr entity_loader(yorcvs::ECS* world, std::array<std::string, sizeof...(Components)> json_names)
        : world { world }
        , json_names { std::move(json_names) }

    {
    }
    /**
     * @brief Loads json entity data into the entity
     *
     * @param entity_id id of the entity
     * @param path path to the json file
     */
    void load_entity_from_path(size_t entity_id, const std::string& path)
    {
        std::ifstream entityIN(path);
        std::string entityDATA { (std::istreambuf_iterator<char>(entityIN)), (std::istreambuf_iterator<char>()) };
        load_entity_from_string(entity_id, entityDATA);
    }
    void load_entity_from_string(size_t entity_id, const std::string& data)
    {
        auto entityJSON = json::json::parse(data, nullptr, false); // don't throw exception
        if (entityJSON.is_discarded()) {
            yorcvs::log("Failed to load entity data " + data);
            return;
        }
        [&]<std::size_t... I>([[maybe_unused]] std::index_sequence<I...> seq)
        {
            if (!(deserialize_component_from_json<Components>(entity_id, entityJSON, std::get<I>(json_names)) && ...)) {
                yorcvs::log("Entity could not be deserialized!" + data);
                return;
            }
        }
        (std::make_index_sequence<sizeof...(Components)>());
    }
    /**
     * @brief Serializes an entities components and returns the string reprezentation
     *
     * @param entity
     * @return std::string
     */
    [[nodiscard]] std::string save_entity(const size_t entity_id) const
    {
        return [&]<size_t... I>([[maybe_unused]] std::index_sequence<I...> seq)
        {
            json::json j;
            (serialize_component_to_json<Components>(entity_id, std::get<I>(json_names), j), ...);
            return j.dump(4);
        }
        (std::make_index_sequence<sizeof...(Components)>());
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
            if (!yorcvs::components::deserialize(world, comp,
                    json_entity_obj[component_name])) {
                yorcvs::log(component_name + " could not be deserialized! ", yorcvs::MSGSEVERITY::ERROR);
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
            json_obj[component_name] = yorcvs::components::serialize(world, world->get_component<T>(entity_id));
        }
    }

    yorcvs::ECS* world;
    const std::array<std::string, sizeof...(Components)> json_names {};
};
