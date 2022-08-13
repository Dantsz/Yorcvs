#pragma once
#include "../common/types.h"
#include <string>
#include <unordered_map>
#include <vector>
/**
 * @brief Defines some identification information for entity
 *
 */
struct identificationComponent {
    std::string name;
};

/**
 * @brief Represent a hitbox (x and y represents offset from positionComponent)
 *
 */
struct hitboxComponent {
    yorcvs::Rect<float> hitbox;
};
struct positionComponent {
    yorcvs::Vec2<float> position;
};
struct velocityComponent {
    yorcvs::Vec2<float> vel;
    yorcvs::Vec2<bool> facing; // x - true if last velocity change was right, left if false
                               // y - true if last velocity change was down , down if up
};

struct playerMovementControlledComponent {
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

struct healthComponent {
    float HP;
    bool is_dead; // no health regen if dead
};
struct healthStatsComponent {
    float max_HP;
    float health_regen; // per tick
};

struct staminaComponent {
    float stamina;
};
struct staminaStatsComponent {
    float max_stamina;
    float stamina_regen;
};

struct spriteComponent {
    yorcvs::Vec2<float> offset; // position offset
    yorcvs::Vec2<float> size; // size of sprite
    yorcvs::Rect<size_t> src_rect; // part of texture to render
    std::string texture_path;
};

struct animationComponent {
    /**
     *A frame of an animation
     *The rectangle represents the area of the texture to render
     *The size_t represents the next frame in the animation
     *The float represents animation speed
     */
    using animation_frame = std::tuple<yorcvs::Rect<size_t>, size_t, float>;
    std::vector<animation_frame> frames {};
    std::unordered_map<std::string, size_t> animation_name_to_start_frame_index;
    std::string current_animation_name {};
    size_t current_frame;
    float current_elapsed_time;
};

struct behaviourComponent {
    float dt;
    float accumulated;
    std::string code_path;
};
struct defensiveStatsComponent {
    float defense;
    float block;
    float dodge;
    float spirit;
};
struct offensiveStatsComponent {
    float strength;
    float agility;
    float dexterity;
    float piercing;
    float intellect;
};
