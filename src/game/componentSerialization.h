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
    j["max"] = comp.maxHP;
    j["regen"] = comp.health_regen;
    return j;
}
template <> inline void deserialize(healthComponent &dst, const json::json &j)
{
    dst.HP = j["current"];
    dst.maxHP = j["max"];
    dst.is_dead = false;
    dst.health_regen = j["regen"];
}
// staminaComponent
template <> inline json::json serialize(const staminaComponent &comp)
{
    json::json j;
    j["current"] = comp.stamina;
    j["max"] = comp.maxStamina;
    j["regen"] = comp.stamina_regen;
    return j;
}
inline void deserialize(staminaComponent &dst, const json::json &j)
{
    dst.stamina = j["current"];
    dst.maxStamina = j["max"];
    dst.stamina_regen = j["regen"];
}
// hitboxComponent
template <> inline json::json serialize(const hitboxComponent &comp)
{
    json::json j;
    j["x"] = comp.hitbox.x;
    j["y"] = comp.hitbox.y;
    j["w"] = comp.hitbox.w;
    j["h"] = comp.hitbox.h;
    return j;
}
template <> inline void deserialize(hitboxComponent &dst, const json::json &j)
{
    dst.hitbox.x = j["x"];
    dst.hitbox.y = j["y"];
    dst.hitbox.w = j["w"];
    dst.hitbox.h = j["h"];
}
// positionComponent

// spriteComponent
template <> inline json::json serialize(const spriteComponent &comp)
{
    json::json j;
    j["offset"]["x"] = comp.offset.x;
    j["offset"]["y"] = comp.offset.y;

    j["size"]["x"] = comp.size.x;
    j["size"]["y"] = comp.size.y;

    j["srcRect"]["x"] = comp.srcRect.x;
    j["srcRect"]["y"] = comp.srcRect.y;
    j["srcRect"]["w"] = comp.srcRect.w;
    j["srcRect"]["h"] = comp.srcRect.h;

    std::filesystem::path sprite_path = comp.texture_path;
    j["spriteName"] = sprite_path.filename();
    return j;
}
template<>
inline void deserialize(spriteComponent &dst, const json::json &j)
{
    
    dst = {
        {j["offset"]["x"], j["offset"]["y"]},
        {j["size"]["x"], j["size"]["y"]},
        {j["srcRect"]["x"], j["srcRect"]["y"],
         j["srcRect"]["w"], j["srcRect"]["h"]},
        j["spriteName"]};
}
// animationComponent
 template<>
 inline json::json serialize(const animationComponent& comp) 
 {
      json::json j;
      for (const auto &[name, animation] : comp.animations)
        {
                json::json anim;
                anim["name"] = name;
                anim["speed"] = animation.speed;
                for (const auto &frame : animation.frames)
                {
                    json::json jframe;
                    jframe["x"] = frame.srcRect.x;
                    jframe["y"] = frame.srcRect.y;
                    jframe["w"] = frame.srcRect.w;
                    jframe["h"] = frame.srcRect.h;
                    anim["frames"].push_back(jframe);
                }

                j.push_back(anim);
        }
        return j;
 }
 template<> inline void deserialize(animationComponent& dst, const json::json& j)
 {
        dst.animations.clear();
        dst = {};
        for (const auto &animation : j)
        {
            bool animation_succes = AnimationSystem::add_animation_to_component(dst,animation["name"],animation["speed"]);
            if (animation_succes)
            {
                for (const auto &frame : animation["frames"])
                {
                    bool rez = AnimationSystem::add_frame_to_animation(dst, animation["name"],
                                              {frame["x"], frame["y"], frame["w"], frame["h"]});
                    if(!rez)
                    {
                        yorcvs::log("Something went wriong loading an animation frame");
                    }
                }
            }
        }
 }
// behaviourComponent
} // namespace yorcvs::components