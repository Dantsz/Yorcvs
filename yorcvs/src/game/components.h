#pragma once
#include "../common/types.h"
#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
/**
 * @brief Defines some identification information for entity
 *
 */
struct identification_component {
    std::string name;
};

/**
 * @brief Represent a hitbox (x and y represents offset from positionComponent)
 *
 */
struct hitbox_component {
    yorcvs::rect<float> hitbox;
};
struct position_component {
    yorcvs::vec2<float> position;
};
struct velocity_component {
    yorcvs::vec2<float> vel;
    yorcvs::vec2<bool> facing; // x - true if last velocity change was right, left if false
                               // y - true if last velocity change was down , down if up
};

struct player_movement_controlled_component {
public:
    enum player_state : size_t {
        PLAYER_IDLE_R,
        PLAYER_IDLE_L,
        PLAYER_MOVING_R,
        PLAYER_MOVING_L,
        PLAYER_USE_R,
        PLAYER_USE_L,
        PLAYER_ATTACK_R,
        PLAYER_ATTACK_L
    };
    player_state current_state { PLAYER_IDLE_R };
    float update_time {};
};

struct health_component {
    float HP;
    bool is_dead; // no health regen if dead
};
struct health_stats_component {
    float max_HP;
    float health_regen; // per tick
};

struct stamina_component {
    float stamina;
};
struct stamina_stats_component {
    float max_stamina;
    float stamina_regen;
};

struct sprite_component {
    yorcvs::vec2<float> offset; // position offset
    yorcvs::vec2<float> size; // size of sprite
    yorcvs::rect<size_t> src_rect; // part of texture to render
    std::string texture_path;
};

struct animation_component {
    /**
     *A frame of an animation
     *The rectangle represents the area of the texture to render
     *The size_t represents the next frame in the animation
     *The float represents animation speed
     */
    using animation_frame = std::tuple<yorcvs::rect<size_t>, size_t, float>;
    std::vector<animation_frame> frames {};
    std::unordered_map<std::string, size_t> animation_name_to_start_frame_index;
    std::string current_animation_name {};
    size_t current_frame;
    float current_elapsed_time;
};

struct behaviour_component {
    float dt;
    float accumulated;
    std::string code_path;
};
struct defensive_stats_component {
    float defense;
    float block;
    float dodge;
    float spirit;
};
struct offensive_stats_component {
    float strength;
    float agility;
    float dexterity;
    float piercing;
    float intellect;
};
struct owned_component {
    size_t owner_id;
};
struct item_component {
    size_t quality;
};
struct inventory_component {
    static constexpr size_t inventory_size = 16;
    std::array<std::optional<size_t>, inventory_size> items;
};
