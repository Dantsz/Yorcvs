#pragma once
#include "components.h"
#include "systems.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
namespace json = nlohmann;

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
template <typename T> json::json serialize(const T &);

// Deserialization from json
/**
 * @brief Template definition for deserialize from json
 *
 * @tparam T
 * @param dst destination
 *
 */
template <typename T> void deserialize(T &dst, const json::json &);
//WORK IN PROGRESS 
template<typename JSON_TYPE, typename T, typename Member_ptr>
void exp_serialize_member(JSON_TYPE& json_obj, const T& comp, std::tuple<const char*, Member_ptr T::*> member)
{
    json_obj[std::get<0>(member)] = comp.*std::get<1>(member);
}
template<typename T, typename ...Args>
json::json exp_serialize(const T& comp, Args ...args)
{
    json::json j;
    (exp_serialize_member(j, comp, args), ...);
    return j;
}
template<typename JSON_TYPE, typename T, typename Member_ptr>
void exp_deserialize_member(T& dst, const JSON_TYPE& json, std::tuple<const char*, Member_ptr T::*> member)
{
    dst.*std::get<1>(member) = json[std::get<0>(member)];
}

template <typename T, typename ...Args>
void exp_deserialize(T& dst, const json::json& j, Args ...args)
{
    (exp_deserialize_member(dst, j, args), ...);
}
//Specialization for simple types
template <typename T> inline json::json serialize(const yorcvs::Vec2<T>& vec)
{
    return exp_serialize(vec, std::make_tuple("x", &yorcvs::Vec2<T>::x), std::make_tuple("y", &yorcvs::Vec2<T>::y));
}
template <typename T> inline void deserialize(yorcvs::Vec2<T>& vec, const json::json& j)
{
    exp_deserialize(vec, j, std::make_tuple("x", &yorcvs::Vec2<T>::x), std::make_tuple("y", &yorcvs::Vec2<T>::y));
}

template <typename T> inline json::json serialize(const yorcvs::Rect<T>& rect)
{
    return exp_serialize(rect, std::make_tuple("x", &yorcvs::Rect<T>::x), std::make_tuple("y", &yorcvs::Rect<T>::y), std::make_tuple("w", &yorcvs::Rect<T>::w), std::make_tuple("h", &yorcvs::Rect<T>::h));
}
template <typename T> inline void deserialize(yorcvs::Rect<T>& rect, const json::json& j)
{
    exp_deserialize(rect, j, std::make_tuple("x", &yorcvs::Rect<T>::x), std::make_tuple("y", &yorcvs::Rect<T>::y), std::make_tuple("w", &yorcvs::Rect<T>::w), std::make_tuple("h", &yorcvs::Rect<T>::h));
}

// Specialization for each component
// identificationComponent
template <> inline json::json serialize(const identificationComponent &comp)
{
    json::json j;
    j["name"] = comp.name;
    return j;
}
template <> inline void deserialize(identificationComponent &dst, const json::json &j)
{
    dst.name = j["name"];
}
// healthComponent
template <> inline json::json serialize(const healthComponent &comp)
{
    json::json j;
    j["current"] = comp.HP;
    j["max"] = comp.max_HP;
    j["regen"] = comp.health_regen;
    return j;
}
template <> inline void deserialize(healthComponent &dst, const json::json &j)
{
    dst.HP = j["current"];
    dst.max_HP = j["max"];
    dst.is_dead = false;
    dst.health_regen = j["regen"];
}
// staminaComponent
template <> inline json::json serialize(const staminaComponent &comp)
{
    json::json j;
    j["current"] = comp.stamina;
    j["max"] = comp.max_stamina;
    j["regen"] = comp.stamina_regen;
    return j;
}
inline void deserialize(staminaComponent &dst, const json::json &j)
{
    dst.stamina = j["current"];
    dst.max_stamina = j["max"];
    dst.stamina_regen = j["regen"];
}
// hitboxComponent
template <> inline json::json serialize(const hitboxComponent &comp)
{
    return serialize(comp.hitbox);
}
template <> inline void deserialize(hitboxComponent &dst, const json::json &j)
{
    deserialize(dst.hitbox, j);
}
// positionComponent

// spriteComponent
template <> inline json::json serialize(const spriteComponent &comp)
{
    json::json j;
    j["offset"] = serialize(comp.offset);
    j["size"] = serialize(comp.size);
    j["srcRect"] = serialize(comp.src_rect);
    std::filesystem::path sprite_path = comp.texture_path;
    j["spriteName"] = sprite_path.filename().string();
    return j;
}
template <> inline void deserialize(spriteComponent &dst, const json::json &j)
{
    dst = {{j["offset"]["x"], j["offset"]["y"]},
           {j["size"]["x"], j["size"]["y"]},
           {j["srcRect"]["x"], j["srcRect"]["y"], j["srcRect"]["w"], j["srcRect"]["h"]},
           j["spriteName"]};
}
// animationComponent
template <> inline json::json serialize(const animationComponent &comp)
{
    json::json j;
    for (const auto &[name, animation] : comp.animations)
    {
        json::json anim;
        anim["name"] = name;
        anim["speed"] = animation.speed;
        for (const auto &frame : animation.frames)
        {
            json::json jframe = serialize(frame);
            anim["frames"].push_back(jframe);
        }
        j.push_back(anim);
    }
    return j;
}
template <> inline void deserialize(animationComponent &dst, const json::json &j)
{
    dst.animations.clear();
    dst = {};
    for (const auto &animation : j)
    {
        bool animation_succes = AnimationSystem::add_animation_to_component(dst, animation["name"], animation["speed"]);
        if (animation_succes)
        {
            for (const auto &frame : animation["frames"])
            {
                bool rez = AnimationSystem::add_frame_to_animation(dst, animation["name"],
                                                                   {frame["x"], frame["y"], frame["w"], frame["h"]});
                if (!rez)
                {
                    yorcvs::log("Something went wrong loading an animation frame");
                }
            }
        }
    }
}
// behaviourComponent

// offensiveStatsComponent
template <> inline json::json serialize(const offensiveStatsComponent &comp)
{
    json::json j;
    j["strength"] = comp.strength;
    j["agility"] = comp.agility;
    j["dexterity"] = comp.dexterity;
    j["piercing"] = comp.piercing;
    j["intellect"] = comp.intellect;
    return j;
}
template <> inline void deserialize(offensiveStatsComponent &dst, const json::json &json)
{
    if (json.contains("strength"))
    {
        dst.strength = json["strength"];
    }
    if (json.contains("agility"))
    {
        dst.agility = json["agility"];
    }
    if (json.contains("dexterity"))
    {
        dst.dexterity = json["dexterity"];
    }
    if (json.contains("piercing"))
    {
        dst.piercing = json["piercing"];
    }
    if (json.contains("intellect"))
    {
        dst.intellect = json["intellect"];
    }
}
// defensiveStatsComponent
template <> inline json::json serialize(const defensiveStatsComponent &comp)
{
    json::json j;
    j["defense"] = comp.defense;
    j["dodge"] = comp.dodge;
    j["block"] = comp.block;
    j["spirit"] = comp.spirit;
    return j;
}
template <> inline void deserialize(defensiveStatsComponent &dst, const json::json &json)
{
    if (json.contains("defense"))
    {
        dst.defense = json["defense"];
    }
    if (json.contains("dodge"))
    {
        dst.dodge = json["dodge"];
    }
    if (json.contains("block"))
    {
        dst.block = json["block"];
    }
    if (json.contains("spirit"))
    {
        dst.spirit = json["spirit"];
    }
}



} // namespace yorcvs::components