#pragma once
#include "../common/ecs.h"
#include "../common/utilities/log.h"
#include "../engine/entity_loader.h"
#include "components.h"
#include <filesystem>
#include <string>

namespace yorcvs {
template <typename T>
inline void to_json(json::json& j, const yorcvs::vec2<T>& vec)
{
    j = json::json { { "x", vec.x }, { "y", vec.y } };
}
template <typename T>
inline void from_json(const json::json& j, yorcvs::vec2<T>& vec)
{
    j.at("x").get_to(vec.x);
    j.at("y").get_to(vec.y);
}
template <typename T>
inline void to_json(json::json& j, const yorcvs::rect<T>& rect)
{
    j = json::json { { "x", rect.x }, { "y", rect.y }, { "w", rect.w }, { "h", rect.h } };
}
template <typename T>
inline void from_json(const json::json& j, yorcvs::rect<T>& rect)
{
    j.at("x").get_to(rect.x);
    j.at("y").get_to(rect.y);
    j.at("w").get_to(rect.w);
    j.at("h").get_to(rect.h);
}
}
inline void to_json(json::json& j, const identification_component& p)
{
    j = json::json { { "name", p.name } };
}
inline void from_json(const json::json& j, identification_component& p)
{
    j.at("name").get_to(p.name);
}
inline void to_json(json::json& j, const health_component& comp)
{
    j = json::json { { "current", comp.HP } }; //,
}
inline void from_json(const json::json& j, health_component& comp)
{
    j.at("current").get_to(comp.HP);
}
inline void to_json(json::json& j, const health_stats_component& comp)
{
    j = { { "max", comp.max_HP }, { "regen", comp.health_regen } };
}
inline void from_json(const json::json& j, health_stats_component& comp)
{
    j.at("max").get_to(comp.max_HP);
    j.at("regen").get_to(comp.health_regen);
}
inline void to_json(json::json& j, const stamina_component& comp)
{
    j = json::json { { "current", comp.stamina } };
}
inline void from_json(const json::json& j, stamina_component& comp)
{
    j.at("current").get_to(comp.stamina);
}
inline void to_json(json::json& j, const stamina_stats_component& comp)
{
    j = json::json { { "max", comp.max_stamina }, { "regen", comp.stamina_regen } };
}
inline void from_json(const json::json& j, stamina_stats_component& comp)
{
    j.at("max").get_to(comp.max_stamina);
    j.at("regen").get_to(comp.stamina_regen);
}
inline void to_json(json::json& j, const hitbox_component& comp)
{
    j = comp.hitbox;
}
inline void from_json(const json::json& j, hitbox_component& comp)
{
    j.get_to(comp.hitbox);
}

inline void to_json(json::json& j, const sprite_component& comp)
{
    j["offset"] = comp.offset;
    j["size"] = comp.size;
    j["srcRect"] = comp.src_rect;
    std::filesystem::path sprite_path = comp.texture_path;
    j["spriteName"] = sprite_path.filename().string();
}
inline void from_json(const json::json& j, sprite_component& comp)
{
    comp = { { j["offset"]["x"], j["offset"]["y"] },
        { j["size"]["x"], j["size"]["y"] },
        { j["srcRect"]["x"], j["srcRect"]["y"], j["srcRect"]["w"], j["srcRect"]["h"] },
        j["spriteName"] };
}

inline void to_json(json::json& j, const animation_component& comp)
{
    for (const auto& [rect, next_id, speed] : comp.frames) {
        json::json frame {};
        frame["rectangle"] = rect;
        frame["next_id"] = next_id;
        frame["speed"] = speed;
        j["frames"].push_back(frame);
    }
    for (const auto& [anim_name, anim_start_index] : comp.animation_name_to_start_frame_index) {
        j["animations"][anim_name] = anim_start_index;
    }
    j["current_frame"] = comp.current_frame;
    j["current_elapsed_time"] = comp.current_elapsed_time;
    j["current_animation_name"] = comp.current_animation_name;
}
inline void from_json(const json::json& j, animation_component& comp)
{
    comp.animation_name_to_start_frame_index.clear();
    comp.frames.clear();
    comp.current_animation_name = j["current_animation_name"];
    comp.current_frame = j["current_frame"];
    comp.current_elapsed_time = j["current_elapsed_time"];
    for (const auto& frame : j["frames"]) {
        comp.frames.emplace_back(frame["rectangle"], frame["next_id"], frame["speed"]);
    }
    const size_t no_of_frames = comp.frames.size();
    for (const auto& animation : j["animations"].items()) {
        if (animation.value() >= no_of_frames) {
            continue;
        }
        comp.animation_name_to_start_frame_index[animation.key()] = animation.value();
    }
}
inline void to_json(json::json& j, const offensive_stats_component& comp)
{
    j["strength"] = comp.strength;
    j["agility"] = comp.agility;
    j["dexterity"] = comp.dexterity;
    j["piercing"] = comp.piercing;
    j["intellect"] = comp.intellect;
}
inline void from_json(const json::json& j, offensive_stats_component& comp)
{
    if (j.contains("strength")) {
        comp.strength = j["strength"];
    }
    if (j.contains("agility")) {
        comp.agility = j["agility"];
    }
    if (j.contains("dexterity")) {
        comp.dexterity = j["dexterity"];
    }
    if (j.contains("piercing")) {
        comp.piercing = j["piercing"];
    }
    if (j.contains("intellect")) {
        comp.intellect = j["intellect"];
    }
}

inline void to_json(json::json& j, const defensive_stats_component& comp)
{
    j["defense"] = comp.defense;
    j["dodge"] = comp.dodge;
    j["block"] = comp.block;
    j["spirit"] = comp.spirit;
}
inline void from_json(const json::json& j, defensive_stats_component& comp)
{
    if (j.contains("defense")) {
        comp.defense = j["defense"];
    }
    if (j.contains("dodge")) {
        comp.dodge = j["dodge"];
    }
    if (j.contains("block")) {
        comp.block = j["block"];
    }
    if (j.contains("spirit")) {
        comp.spirit = j["spirit"];
    }
}
inline void to_json(json::json& j, const item_component& comp)
{
    j["quality"] = comp.quality;
}
inline void from_json(const json::json& j, item_component& comp)
{
    if (j.contains("quality")) {
        comp.quality = j["quality"];
    }
}
namespace yorcvs::components {
template <>
json::json serialize([[maybe_unused]] yorcvs::ECS* world, const inventory_component& comp)
{
    /*TODO:
     *component contains an array of entities
     *serialize each entity
     *add to array
     *return
     */
    json::json j;
    entity_loader<identification_component, health_stats_component, stamina_stats_component, offensive_stats_component, defensive_stats_component, sprite_component> loader { world, { "identification", "health_stats", "stamina_stats", "offsensive_stats", "defensive_stats", "sprite" } };
    for (const auto& item : comp.items) {
        if (item.has_value()) {
            j.push_back(loader.save_entity(item.value()));
        }
    }
    return j;
}
template <>
[[nodiscard]] bool deserialize([[maybe_unused]] yorcvs::ECS* world, inventory_component& dst, const json::json& j)
{
    try {
        /*TODO:
         *create new entities for each item
         *assign each entity an item_component
         */
        entity_loader<identification_component, health_stats_component, stamina_stats_component, offensive_stats_component, defensive_stats_component, sprite_component> loader { world, { "identification", "health_stats", "stamina_stats", "offsensive_stats", "defensive_stats", "sprite" } };
        size_t item_index = 0;
        for (const auto& item : j) {
            dst.items[item_index] = world->create_entity_ID();
            loader.load_entity_from_string(dst.items[item_index].value(), item);
            item_index++;
        }
    } catch (...) {
        yorcvs::log("failed to deserialize component");
        return false;
    }

    return true;
}
} // namespace yorcvs::components
