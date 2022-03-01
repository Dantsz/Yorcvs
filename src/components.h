#pragma once
#include "common/types.h"
#include "windowSDL2.h"
#include <unordered_map>
namespace yorcvs{
using graphics = yorcvs::SDL2;
}
/**
 * @brief Defines some identification information for entity
 * 
 */
struct identificationComponent
{
    std::string name;
};

/**
 * @brief Represent a hitbox (x and y represents offset from positionComponent)
 * 
 */
struct hitboxComponent
{   
    yorcvs::Rect<float> hitbox;
};
struct positionComponent
{
    yorcvs::Vec2<float> position;
};
struct velocityComponent
{   
    yorcvs::Vec2<float> vel;
    yorcvs::Vec2<bool> facing;// x - true if last velocity change was right, left if false
                              // y - true if last velocity change was down , down if up
};

struct playerMovementControlledComponent{};


struct healthComponent
{
    float HP;
    float maxHP;
    float health_regen;//per tick
    bool is_dead;// no health regen if dead
};
struct staminaComponent
{
    float stamina;
    float maxStamina;
    float stamina_regen;
};
struct spriteComponent
{
    yorcvs::Vec2<float> offset;//position offset
    yorcvs::Vec2<float> size;// size of sprite
    yorcvs::Rect<size_t> srcRect;//part of texture to render
    std::string texture_path;
};

struct animationComponent
{
    struct AnimationFrame
    {
        yorcvs::Rect<size_t> srcRect;
    };
    struct Animation
    {
        std::vector<AnimationFrame> frames;// not ideal
        float speed;// in milliseconds
    };
    std::unordered_map<std::string, Animation> animations;//ugly
    std::string cur_animation; // non-owning pointer to an animation in the map
    size_t cur_frame;
    float cur_elapsed;
};

struct behaviourComponent
{
    float dt;
    float accumulated;
};
struct defensiveStatsComponent;
struct offensiveStatsComponent;
