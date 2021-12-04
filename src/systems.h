#pragma once
#include "common/ecs.h"
#include "components.h"
#include "windowSDL2.h"
#include <array>

class CollisionSystem
{
  public:
    CollisionSystem(yorcvs::ECS *parent) : world(parent)
    {
        // is this legal? yee
        world->register_system<CollisionSystem>(*this);
        world->add_criteria_for_iteration<CollisionSystem, positionComponent, hitboxComponent>();
    }

    void update() const // checks and resolves collisions
    {
        yorcvs::Rect<float> rectA{};
        yorcvs::Rect<float> rectB{};

        for (const auto &IDA : entityList->entitiesID)
        {
            if (world->has_components<velocityComponent>(IDA))
            {
                rectA.x = world->get_component<positionComponent>(IDA).position.x +
                          world->get_component<hitboxComponent>(IDA).hitbox.x;
                rectA.y = world->get_component<positionComponent>(IDA).position.y +
                          world->get_component<hitboxComponent>(IDA).hitbox.y;
                rectA.w = world->get_component<hitboxComponent>(IDA).hitbox.w;
                rectA.h = world->get_component<hitboxComponent>(IDA).hitbox.h;
                yorcvs::Vec2<float> &rectAvel = world->get_component<velocityComponent>(IDA).vel;
                for (const auto &IDB : entityList->entitiesID)
                {
                    rectB.x = world->get_component<positionComponent>(IDB).position.x +
                              world->get_component<hitboxComponent>(IDB).hitbox.x;
                    rectB.y = world->get_component<positionComponent>(IDB).position.y +
                              world->get_component<hitboxComponent>(IDB).hitbox.y;

                    rectB.w = world->get_component<hitboxComponent>(IDB).hitbox.w;

                    rectB.h = world->get_component<hitboxComponent>(IDB).hitbox.h;
                    if (IDA != IDB)
                    {
                        // left to right
                        check_collision_left_right(rectA, rectB, rectAvel);
                        // right to left
                        check_collision_right_left(rectA, rectB, rectAvel);
                        // up to down
                        check_collision_up_down(rectA, rectB, rectAvel);
                        // down to up
                        check_collision_down_up(rectA, rectB, rectAvel);

                        // top right corner
                        check_collision_corner_top_right(rectA, rectB, rectAvel);
                        // top left corner
                        check_collision_corner_top_left(rectA, rectB, rectAvel);
                        // bottom right corner
                        check_collision_corner_bottom_right(rectA, rectB, rectAvel);
                        // bottom left corner
                        check_collision_corner_bottom_left(rectA, rectB, rectAvel);
                    }
                }
            }
        }
    }
    template <typename render_backend>
    void render_hitboxes(yorcvs::Window<render_backend> &window, const yorcvs::Vec2<float> &render_dimensions, float r,
                         float g, float b, float a)
    {
        yorcvs::Vec2<float> old_rs = window.get_render_scale();
        window.set_render_scale(window.get_size() / render_dimensions);

        yorcvs::Rect<float> rect{};
        for (const auto &ID : entityList->entitiesID)
        {
            rect.x = world->get_component<positionComponent>(ID).position.x +
                     world->get_component<hitboxComponent>(ID).hitbox.x;
            rect.y = world->get_component<positionComponent>(ID).position.y +
                     world->get_component<hitboxComponent>(ID).hitbox.y;
            rect.w = world->get_component<hitboxComponent>(ID).hitbox.w;
            rect.h = world->get_component<hitboxComponent>(ID).hitbox.h;
            window.draw_rect(rect, r, g, b, a);
            if (world->has_components<healthComponent>(ID))
            {
                /// draw health bar
                yorcvs::Rect<float> healthBarRect{};
                healthBarRect.x = rect.x - 16.0f + rect.w / 2;
                healthBarRect.y = rect.y - rect.h;
                healthBarRect.w = 32.0f;
                healthBarRect.h = 8.0f;
                window.draw_rect(healthBarRect, 100, 0, 0, 255);
                healthBarRect.w =
                    (world->get_component<healthComponent>(ID).HP / world->get_component<healthComponent>(ID).maxHP) *
                    32.0f;
                window.draw_rect(healthBarRect, 255, 0, 0, 255);
            }
        }
        window.set_render_scale(old_rs);
    }

  private:
    static bool check_collision_left_right(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                           yorcvs::Vec2<float> &rectAvel)
    {
        if (rectA.x + rectA.w <= rectB.x && rectA.x + rectA.w + (rectAvel.x) > rectB.x && rectA.y + rectA.h > rectB.y &&
            rectA.y - rectB.y < rectB.h)
        {
            rectAvel.x = (rectB.x - rectA.x - rectA.w);
            return true;
        }
        return false;
    }
    static bool check_collision_right_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                           yorcvs::Vec2<float> &rectAvel)
    {
        if (rectA.x >= rectB.x + rectB.w && rectA.x + (rectAvel.x) < rectB.x + rectB.w && rectA.y + rectA.h > rectB.y &&
            rectA.y - rectB.y < rectB.h)
        {
            rectAvel.x = (rectB.x + rectB.w - rectA.x);
            return true;
        }
        return false;
    }
    static bool check_collision_up_down(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                        yorcvs::Vec2<float> &rectAvel)
    {
        if (rectA.y <= rectB.y && rectA.y + rectA.h + (rectAvel.y) > rectB.y && rectA.x - rectB.x < rectB.w &&
            rectA.x + rectA.w > rectB.x)
        {
            rectAvel.y = (rectB.y - rectA.y - rectA.h);
            return true;
        }
        return false;
    }
    static bool check_collision_down_up(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                        yorcvs::Vec2<float> &rectAvel)
    {
        if (rectA.y >= rectB.y + rectB.h && rectA.y + (rectAvel.y) < rectB.y + rectB.h && rectA.x - rectB.x < rectB.w &&
            rectA.x + rectA.w > rectB.x)
        {
            rectAvel.y = (rectB.y + rectB.h - rectA.y);
            return true;
        }
        return false;
    }

    static bool check_collision_corner_top_right(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                 yorcvs::Vec2<float> &rectAvel)
    {
        if (rectA.x + (rectAvel.x) + rectA.w > rectB.x && rectA.x + (rectAvel.x) + rectA.w < (rectB.x + rectB.w) &&
            rectA.y + rectA.h + (rectAvel.y) > rectB.y && rectA.y + rectA.h + (rectAvel.y) < rectB.y + rectB.h &&
            rectA.x < rectB.x && rectA.y < rectB.y)
        {
            rectAvel.x = (rectB.x - (rectA.x + rectA.w));
            rectAvel.y = ((rectA.y + rectA.h) - rectB.y);
            return true;
        }
        return false;
    }
    static bool check_collision_corner_top_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                yorcvs::Vec2<float> &rectAvel)
    {
        if (rectA.x + (rectAvel.x) > rectB.x && rectA.x + (rectAvel.x) < rectB.x + rectB.w &&
            rectA.y + rectA.h + (rectAvel.y) > rectB.y && rectA.y + rectA.h + (rectAvel.y) < rectB.y + rectB.h &&
            rectA.x >= rectB.x && rectA.x + (rectAvel.x) + rectA.w > rectB.x + rectB.w && rectA.y < rectB.y)
        {
            rectAvel.x = ((rectB.x + rectB.w) - rectA.x);
            rectAvel.y = ((rectA.y + rectA.h) - rectB.y);
            return true;
        }
        return false;
    }
    static bool check_collision_corner_bottom_right(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                    yorcvs::Vec2<float> &rectAvel)
    {
        if (rectA.x + (rectAvel.x) < rectB.x && rectA.x + (rectAvel.x) + rectA.w > rectB.x &&
            rectA.x + (rectAvel.x) + rectA.w < rectB.x + rectB.w && rectA.x < rectB.x &&
            rectA.y + (rectAvel.y) > rectB.y && rectA.y + (rectAvel.y) < rectB.y + rectB.h &&
            rectA.y + rectA.h + (rectAvel.y) > rectB.y + rectB.h)
        {
            rectAvel.x = (rectB.x - (rectA.x + rectA.w));
            rectAvel.y = ((rectB.y + rectB.h) - rectA.y);
            return true;
        }
        return false;
    }
    static bool check_collision_corner_bottom_left(const yorcvs::Rect<float> &rectA, const yorcvs::Rect<float> &rectB,
                                                   yorcvs::Vec2<float> &rectAvel)
    {
        if (rectA.x + (rectAvel.x) > rectB.x && rectA.x + (rectAvel.x) < rectB.x + rectB.w &&
            rectA.x + (rectAvel.x) + rectA.w > rectB.x + rectB.w && rectA.y + (rectAvel.y) > rectB.y &&
            rectA.y + (rectAvel.y) < rectB.y + rectB.h && rectA.y + (rectAvel.y) + rectA.h > rectB.y + rectB.h)
        {
            rectAvel.x = ((rectB.x + rectB.w) - rectA.x);
            rectAvel.y = ((rectB.y + rectB.h) - rectA.y);
            return true;
        }
        return false;
    }

  public:
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};

class VelocitySystem
{
  public:
    VelocitySystem(yorcvs::ECS *parent) : world(parent)
    {
        world->register_system<VelocitySystem>(*this);
        world->add_criteria_for_iteration<VelocitySystem, positionComponent, velocityComponent>();
    }
    void update() const
    {
        for (const auto &ID : entityList->entitiesID)
        {
            yorcvs::Vec2<float> posOF = world->get_component<velocityComponent>(ID).vel;
            world->get_component<positionComponent>(ID).position += posOF;
            world->get_component<velocityComponent>(ID).vel = {0, 0};
            if (std::abs(posOF.x) > std::numeric_limits<float>::epsilon())
            {
                world->get_component<velocityComponent>(ID).facing.x = (posOF.x < 0.0f);
            }
            if (std::abs(posOF.y) > std::numeric_limits<float>::epsilon())
            {
                world->get_component<velocityComponent>(ID).facing.y = (posOF.y < 0.0f);
            }
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};

class AnimationSystem
{
  public:
    AnimationSystem(yorcvs::ECS *parent) : world(parent)
    {
        world->register_system<AnimationSystem>(*this);
        world->add_criteria_for_iteration<AnimationSystem, animationComponent, spriteComponent>();
    }
    /**
     * @brief Adds an animation with that name to the component
     *
     * @param entity
     * @param name - name of the animation
     * @return bool - false if failed
     */
    [[nodiscard]] bool add_animation(const size_t entityID, const std::string &name, const float speed) const
    {
        if (!world->has_components<animationComponent>(entityID))
        {
            yorcvs::log("Entity doesn't have  an animation component", yorcvs::MSGSEVERITY::WARNING);
            return false;
        }
        std::unordered_map<std::string, animationComponent::Animation> *entity_anims =
            &world->get_component<animationComponent>(entityID).animations;
        const auto &anim = entity_anims->find(name);
        if (anim == entity_anims->end())
        {
            entity_anims->insert({name, {{}, speed}});
            return true;
        }
        yorcvs::log("Animation " + name + " already exists", yorcvs::MSGSEVERITY::WARNING);
        return false;
    }

    /**
     * @brief adds a frame to the animation with name animation_name to the entity with ID entityID
     *
     * @param entityID
     * @param animation_name
     * @param frame
     * @param frame_pass_time amount of time to pass until next frame
     * @return bool - false if failed
     */
    void add_animation_frame(const size_t entityID, const std::string &animation_name,
                             const yorcvs::Rect<size_t> &frame) const
    {
        if (!world->has_components<animationComponent>(entityID))
        {
            yorcvs::log("Entity doesn't have  an animation component", yorcvs::MSGSEVERITY::WARNING);
            return;
        }
        std::unordered_map<std::string, animationComponent::Animation> *entity_anims =
            &world->get_component<animationComponent>(entityID).animations;
        const auto &anim = entity_anims->find(animation_name);
        if (anim == entity_anims->end())
        {
            yorcvs::log("Entity " + std::to_string(entityID) + " doesn't have an animation with the name " +
                            animation_name,
                        yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        anim->second.frames.push_back({frame});
    }

    void remove_animation(yorcvs::Entity, size_t index);

    void remove_animation_frame(yorcvs::Entity, size_t index);
    /**
     * @brief Set which animation to be used
     *
     * @param entity
     * @param animation_name
     */
    static void set_animation(const yorcvs::Entity &entity, const std::string &animation_name)
    {
        if (!entity.parent->has_components<animationComponent>(entity.id))
        {
            yorcvs::log("Entity doesn't have  an animation component", yorcvs::MSGSEVERITY::WARNING);
            return;
        }
        std::unordered_map<std::string, animationComponent::Animation> *entity_anims =
            &entity.parent->get_component<animationComponent>(entity.id).animations;
        const auto &anim = entity_anims->find(animation_name);
        if (anim == entity_anims->end())
        {
            yorcvs::log("Entity " + std::to_string(entity.id) + " doesn't have an animation with the name " +
                            animation_name,
                        yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        entity.parent->get_component<animationComponent>(entity.id).cur_animation = animation_name;
    }

    /**
     * @brief Set the animation object
     *
     * @param world
     * @param entityID
     * @param animation_name
     */
    static void set_animation(yorcvs::ECS *world, const size_t &entityID, const std::string &animation_name)
    {
        if (!world->has_components<animationComponent>(entityID))
        {
            yorcvs::log("Entity doesn't have  an animation component", yorcvs::MSGSEVERITY::WARNING);
            return;
        }
        std::unordered_map<std::string, animationComponent::Animation> *entity_anims =
            &world->get_component<animationComponent>(entityID).animations;
        const auto &anim = entity_anims->find(animation_name);
        if (anim == entity_anims->end())
        {
            yorcvs::log("Entity " + std::to_string(entityID) + " doesn't have an animation with the name " +
                            animation_name,
                        yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        world->get_component<animationComponent>(entityID).cur_animation = animation_name;
    }

    void update(float elapsed) const
    {
        for (const auto &ID : entityList->entitiesID)
        {
            const animationComponent::Animation* cur_animation = &world->get_component<animationComponent>(ID).animations[world->get_component<animationComponent>(ID).cur_animation];
            world->get_component<animationComponent>(ID).cur_elapsed += elapsed;
            if (world->get_component<animationComponent>(ID).cur_elapsed >
                cur_animation->speed)
            {
                world->get_component<animationComponent>(ID).cur_elapsed = 0;
                world->get_component<animationComponent>(ID).cur_frame++;

                if (world->get_component<animationComponent>(ID).cur_frame >=
                    cur_animation->frames.size()) 
                {
                    world->get_component<animationComponent>(ID).cur_frame = 0;
                }

             
                world->get_component<spriteComponent>(ID).srcRect =
                        cur_animation->frames[world->get_component<animationComponent>(ID).cur_frame]
                        .srcRect;
            }
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
};

class HealthSystem
{
  public:
    HealthSystem(yorcvs::ECS *parent) : world(parent)
    {

        world->register_system<HealthSystem>(*this);
        world->add_criteria_for_iteration<HealthSystem, healthComponent>();
    }
    void update(float dt)
    {
        cur_time += dt;
        if (cur_time >= update_time)
        {
            for (const auto &ID : entityList->entitiesID)
            {
                if (world->get_component<healthComponent>(ID).HP < 0.0f)
                {
                    world->get_component<healthComponent>(ID).is_dead = true;
                    continue;
                }
                world->get_component<healthComponent>(ID).HP += world->get_component<healthComponent>(ID).health_regen;
                if (world->get_component<healthComponent>(ID).HP > world->get_component<healthComponent>(ID).maxHP)
                {
                    world->get_component<healthComponent>(ID).HP = world->get_component<healthComponent>(ID).maxHP;
                }
            }
            cur_time = 0.0f;
        }
    }
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
    static constexpr float update_time = 1000.0f; // update once a second
    float cur_time = 0.0f;
};

class PlayerMovementControl
{
  public:
    PlayerMovementControl(yorcvs::ECS *parent, yorcvs::Window<yorcvs::graphics> *parent_window)
        : world(parent), window(parent_window)
    {
        world->register_system<PlayerMovementControl>(*this);
        world->add_criteria_for_iteration<PlayerMovementControl, playerMovementControlledComponent, velocityComponent,
                                          positionComponent, spriteComponent>();
    }

    void updateControls(const yorcvs::Vec2<float> &render_size)
    {
        w_pressed = window->is_key_pressed(yorcvs::Window<yorcvs::graphics>::YORCVS_KEY_W);
        a_pressed = window->is_key_pressed(yorcvs::Window<yorcvs::graphics>::YORCVS_KEY_A);
        s_pressed = window->is_key_pressed(yorcvs::Window<yorcvs::graphics>::YORCVS_KEY_S);
        d_pressed = window->is_key_pressed(yorcvs::Window<yorcvs::graphics>::YORCVS_KEY_D);

        for (const auto &ID : entityList->entitiesID)
        {
            dir = yorcvs::Vec2<float>(static_cast<float>(d_pressed) + static_cast<float>(a_pressed) * -1.0f,
                                      static_cast<float>(w_pressed) * -1.0f + static_cast<float>(s_pressed));
            dir.normalize();
            world->get_component<velocityComponent>(ID).vel = dir;
            window->set_drawing_offset(world->get_component<positionComponent>(ID).position + dir -
                                       (render_size - world->get_component<spriteComponent>(ID).size) / 2);
        }
    }
    void updateAnimations() const
    {
        for (const auto &ID : entityList->entitiesID)
        {
            if (a_pressed)
            {
                AnimationSystem::set_animation(world, ID, "walkingL");
            }
            else if (d_pressed)
            {
                AnimationSystem::set_animation(world, ID, "walkingR");
            } 
            else if (s_pressed || w_pressed)
            {
                AnimationSystem::set_animation(world, ID, "walkingR");
            }
            else if (world->get_component<velocityComponent>(ID).facing.x)
            {
                AnimationSystem::set_animation(world, ID, "idleL");
            }
            else
            {
                AnimationSystem::set_animation(world, ID, "idleR");
            }
        }
    }
    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
    yorcvs::Window<yorcvs::graphics> *window;
    yorcvs::Vec2<float> dir;
    bool w_pressed{};
    bool a_pressed{};
    bool s_pressed{};
    bool d_pressed{};
};

class SpriteSystem
{
  public:
    SpriteSystem(yorcvs::ECS *parent, yorcvs::Window<yorcvs::graphics> *parentWindow)
        : world(parent), window(parentWindow)
    {
        world->register_system<SpriteSystem>(*this);
        world->add_criteria_for_iteration<SpriteSystem, positionComponent, spriteComponent>();
    }
    void renderSprites(const yorcvs::Vec2<float> &render_dimensions) const
    {
        yorcvs::Vec2<float> rs = window->get_render_scale();
        window->set_render_scale(window->get_size() / render_dimensions);
        std::sort(entityList->entitiesID.begin(), entityList->entitiesID.end(), [&](size_t ID1, size_t ID2) {
            return (world->get_component<spriteComponent>(ID1).offset.y +
                    world->get_component<positionComponent>(ID1).position.y) <
                   (world->get_component<spriteComponent>(ID2).offset.y +
                    world->get_component<positionComponent>(ID2).position.y);
        });
        for (const auto &ID : entityList->entitiesID)
        {
            window->draw_sprite(
                world->get_component<spriteComponent>(ID).texture_path,
                world->get_component<spriteComponent>(ID).offset + world->get_component<positionComponent>(ID).position,
                world->get_component<spriteComponent>(ID).size, world->get_component<spriteComponent>(ID).srcRect, 0.0);
        }
        std::sort(entityList->entitiesID.begin(), entityList->entitiesID.end(),
                  [&](size_t ID1, size_t ID2) { return ID1 < ID2; });
        window->set_render_scale(rs);
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS *world;
    yorcvs::Window<yorcvs::graphics> *window;
};
