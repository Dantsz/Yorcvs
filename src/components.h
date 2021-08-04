#pragma once
#include "common/types.h"

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


struct bodyTextureComponent;
struct defensiveStatsComponent;
struct offensiveStatsComponent;
