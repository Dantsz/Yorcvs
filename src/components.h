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
    yorcvs::Vec2<bool> facing;// x - true if last velocity change was right, left if false
                              // y - true if last velocity change was down , down if up
};

struct playerMovementControlledComponent{};


struct healthComponent
{
    float HP;
    float maxHP;
};

struct spriteComponent
{
    yorcvs::Vec2<float> offset;//position offset
    yorcvs::Vec2<float> size;// size of sprite
    yorcvs::Rect<size_t> srcRect;//part of texture to render
    yorcvs::Texture<yorcvs::graphics> texture;
};

struct animationComponent
{
    size_t cur_frame;
    size_t frames;
    float cur_elapsed;
    float speed;// in milliseconds
};

struct defensiveStatsComponent;
struct offensiveStatsComponent;
