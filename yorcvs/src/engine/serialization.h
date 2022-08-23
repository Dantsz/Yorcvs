#pragma once
#include "../common/ecs.h"
#include <nlohmann/json.hpp>
namespace json = nlohmann;
namespace yorcvs::components {

// TEMPLATES//
// Serialization to json
/**
 * @brief Template definition for serializing a component to json
 *
 * @tparam T
 * @return json::json
 */
template <typename T>
json::json serialize([[maybe_unused]] yorcvs::ECS* world, const T& comp)
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
 * @return returns fals on failure
 */
template <typename T>
[[nodiscard]] bool deserialize([[maybe_unused]] yorcvs::ECS* world, T& dst, const json::json& j)
{
    try {
        dst = j;
    } catch (...) {
        yorcvs::log("failed to deserialize component");
        return false;
    }

    return true;
}
}
