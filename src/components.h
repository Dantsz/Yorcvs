#pragma once
#include "common/types.h"
#include "window/windowSDL2.h"
namespace yorcvs{
using graphics = yorcvs::SDL2;
}
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
};

struct playerMovementControlledComponent{};


struct healthComponent
{
    float HP;
    float maxHP;
};

struct spriteComponent
{
    yorcvs::Rect<size_t> rect;
    size_t frames;
    float speed;// in milliseconds
    yorcvs::Texture<yorcvs::graphics> texture;
};


struct defensiveStatsComponent;
struct offensiveStatsComponent;
