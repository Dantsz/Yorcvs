#pragma once
#include "components.h"
#include "systems.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
namespace json = nlohmann;

namespace yorcvs
{
    template<typename T>
    inline void to_json(json::json& j, const yorcvs::Vec2<T>& vec) {
        j = json::json{{"x", vec.x},{"y",vec.y}};
    }
    template<typename T>
    inline void from_json(const json::json& j, yorcvs::Vec2<T>& vec) {
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
    }
    template<typename T>
    inline void to_json(json::json& j, const yorcvs::Rect<T>& rect) {
        j = json::json{{"x", rect.x},{"y",rect.y},{"w",rect.w},{"h",rect.h}};
    }
    template<typename T>
    inline void from_json(const json::json& j, yorcvs::Rect<T>& rect) {
        j.at("x").get_to(rect.x);
        j.at("y").get_to(rect.y);
        j.at("w").get_to(rect.w);
        j.at("h").get_to(rect.h);
    }
}
inline void to_json(json::json& j, const identificationComponent& p) {
    j = json::json{{"name", p.name}};
}
inline void from_json(const json::json& j, identificationComponent& p) {
    j.at("name").get_to(p.name);
}
inline void to_json(json::json& j, const healthComponent& comp) {
    j = json::json{{"current", comp.HP},{"max",comp.max_HP},{"regen",comp.health_regen}};
}
inline void from_json(const json::json& j, healthComponent& comp) {
    j.at("current").get_to(comp.HP);
    j.at("max").get_to(comp.max_HP);
    j.at("regen").get_to(comp.health_regen);
}
inline void to_json(json::json& j, const staminaComponent& comp) {
    j = json::json{{"current", comp.stamina},{"max",comp.max_stamina},{"regen",comp.stamina_regen}};
}
inline void from_json(const json::json& j, staminaComponent& comp) {
    j.at("current").get_to(comp.stamina);
    j.at("max").get_to(comp.max_stamina);
    j.at("regen").get_to(comp.stamina_regen);
}
inline void to_json(json::json& j, const hitboxComponent& comp) {
    j = json::json{ comp.hitbox};
}
inline void from_json(const json::json& j, hitboxComponent& comp) {
    j.get_to(comp.hitbox);
}

inline void to_json(json::json& j, const spriteComponent& comp) {
    j["offset"] = comp.offset;
    j["size"] = comp.size;
    j["srcRect"] = comp.src_rect;
    std::filesystem::path sprite_path = comp.texture_path;
    j["spriteName"] = sprite_path.filename().string();
}
inline void from_json(const json::json& j, spriteComponent& comp) {
    comp = {{j["offset"]["x"], j["offset"]["y"]},
           {j["size"]["x"], j["size"]["y"]},
           {j["srcRect"]["x"], j["srcRect"]["y"], j["srcRect"]["w"], j["srcRect"]["h"]},
           j["spriteName"]};
}

inline void to_json(json::json& j, const animationComponent& comp) {
    for (const auto &[name, animation] : comp.animations)
    {
        json::json anim;
        anim["name"] = name;
        anim["speed"] = animation.speed;
        for (const auto &frame : animation.frames)
        {
            json::json jframe = frame;
            anim["frames"].push_back(jframe);
        }
        j.push_back(anim);
    }
}
inline void from_json(const json::json& j, animationComponent& comp) {
    comp.animations.clear();
    comp = {};
    for (const auto &animation : j)
    {
        bool animation_succes = AnimationSystem::add_animation_to_component(comp, animation["name"], animation["speed"]);
        if (animation_succes)
        {
            for (const auto &frame : animation["frames"])
            {
                bool rez = AnimationSystem::add_frame_to_animation(comp, animation["name"],
                                                                   {frame["x"], frame["y"], frame["w"], frame["h"]});
                if (!rez)
                {
                    yorcvs::log("Something went wrong loading an animation frame");
                }
            }
        }
    }
}
inline void to_json(json::json& j, const offensiveStatsComponent& comp) {
    j["strength"] = comp.strength;
    j["agility"] = comp.agility;
    j["dexterity"] = comp.dexterity;
    j["piercing"] = comp.piercing;
    j["intellect"] = comp.intellect;
}
inline void from_json(const json::json& j, offensiveStatsComponent& comp) {
    if (j.contains("strength"))
    {
        comp.strength = j["strength"];
    }
    if (j.contains("agility"))
    {
        comp.agility = j["agility"];
    }
    if (j.contains("dexterity"))
    {
        comp.dexterity = j["dexterity"];
    }
    if (j.contains("piercing"))
    {
        comp.piercing = j["piercing"];
    }
    if (j.contains("intellect"))
    {
        comp.intellect = j["intellect"];
    }
}

inline void to_json(json::json& j, const defensiveStatsComponent& comp) {
    j["defense"] = comp.defense;
    j["dodge"] = comp.dodge;
    j["block"] = comp.block;
    j["spirit"] = comp.spirit;
}
inline void from_json(const json::json& j, defensiveStatsComponent& comp) {
    if (j.contains("defense"))
    {
        comp.defense = j["defense"];
    }
    if (j.contains("dodge"))
    {
        comp.dodge = j["dodge"];
    }
    if (j.contains("block"))
    {
        comp.block = j["block"];
    }
    if (j.contains("spirit"))
    {
        comp.spirit = j["spirit"];
    }
}

namespace yorcvs::components
{

// TEMPLATES//
// Serialization to json
/**
 * @brief Template definition for serializing a component to json
 *
 * @tparam T
 * @return json::json
 */
template <typename T> json::json serialize(const T &comp)
{
    json::json j = comp;
    return j;
}


// Deserialization from json
/**
 * @brief Template definition for deserialize from json
 *
 * @tparam T
 * @param dst destination
 *
 */
template <typename T> void deserialize(T &dst, const json::json &j)
{
    dst = j;
}


} // namespace yorcvs::components