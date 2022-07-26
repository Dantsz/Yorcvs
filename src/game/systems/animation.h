#pragma once
#include "../../common/ecs.h"
#include "../components.h"

/**
 * @brief Handles Animation
 *
 */
class AnimationSystem {
public:
    explicit AnimationSystem(yorcvs::ECS* parent)
        : world(parent)
    {
        world->register_system<AnimationSystem>(*this);
        world->add_criteria_for_iteration<AnimationSystem, animationComponent, spriteComponent>();
    }

    /**
     * @brief ADDS an animation with the specified name to the component
     *
     * @param comp - the destination component
     * @param name - the name of the animation
     * @param speed - the speed at which animation frames update
     * @return true - succes
     * @return false - the animation already exists
     */
    //    [[nodiscard]] static bool add_animation_to_component(animationComponent& comp, const std::string& name, const size_t start_frame, const float speed)
    //    {
    //                const auto& anim = comp.animations.find(name);
    //                if (anim == comp.animations.end()) {
    //                   comp.animations.insert({ name, { {}, speed } });
    //                    return true;
    //                }

    //                return false;

    //    }
    /**
     * @brief Adds an animation with that name to the component
     *
     * @param entity
     * @param name - name of the animation
     * @return bool - false if failed
     */
    //    [[nodiscard]] bool add_animation(const size_t entityID, const std::string& name, const size_t start_frame, const float speed) const
    //    {
    //        if (!world->has_components<animationComponent>(entityID)) {
    //            yorcvs::log("Entity doesn't have  an animation component", yorcvs::MSGSEVERITY::WARNING);
    //            return false;
    //        }
    //        if (!add_animation_to_component(world->get_component<animationComponent>(entityID), name, speed)) {
    //            yorcvs::log("Animation " + name + " already exists", yorcvs::MSGSEVERITY::WARNING);
    //            return false;
    //        }
    //        return true;
    //    }

    /**
     * @brief adds a frame to the animation with name animation_name to the entity with ID entityID
     *
     * @param entityID
     * @param animation_name
     * @param frame
     */
    //    void add_animation_frame(const size_t entityID, const std::string& animation_name,
    //        const yorcvs::Rect<size_t>& frame) const
    //    {
    //        if (!world->has_components<animationComponent>(entityID)) {
    //            yorcvs::log("Entity doesn't have  an animation component", yorcvs::MSGSEVERITY::WARNING);
    //            return;
    //        }
    //        if (!add_frame_to_animation(world->get_component<animationComponent>(entityID), animation_name, frame)) {
    //            yorcvs::log("Entity " + std::to_string(entityID) + " doesn't have an animation with the name " + animation_name,
    //                yorcvs::MSGSEVERITY::ERROR);
    //            return;
    //        }
    //    }
    /**
     * @brief Removes an animation from the entity. Currently unecessary, might be useful for editing entities
     *
     * @param animation i
     */
    void remove_animation(yorcvs::Entity, std::string animation);

    void remove_animation_frame(yorcvs::Entity, std::string animation, size_t index);
    /**
     * @brief Set which animation to be used
     *
     * @param entity
     * @param animation_name
     */
    //    static void set_animation(const yorcvs::Entity& entity, const std::string& animation_name)
    //    {
    //        if (!entity.parent->has_components<animationComponent>(entity.id)) {
    //            yorcvs::log("Entity doesn't have  an animation component", yorcvs::MSGSEVERITY::WARNING);
    //            return;
    //        }
    //        std::unordered_map<std::string, animationComponent::Animation>* entity_anims = &entity.parent->get_component<animationComponent>(entity.id).animation_name_to_start_frame_index;
    //        const auto& anim = entity_anims->find(animation_name);
    //        if (anim == entity_anims->end()) {
    //            yorcvs::log("Entity " + std::to_string(entity.id) + " doesn't have an animation with the name " + animation_name,
    //                yorcvs::MSGSEVERITY::ERROR);
    //            return;
    //        }
    //        entity.parent->get_component<animationComponent>(entity.id).cur_animation = animation_name;
    //    }

    /**
     * @brief Set the animation object
     *
     * @param world
     * @param entityID
     * @param animation_name
     */
    static void set_animation(yorcvs::ECS* world, const size_t& entityID, const std::string& animation_name)
    {
        const auto& anim_comp = world->get_component<animationComponent>(entityID);
        if (!world->has_components<animationComponent>(entityID)) {
            yorcvs::log("Entity doesn't have  an animation component", yorcvs::MSGSEVERITY::WARNING);
            return;
        }
        const auto& anim = anim_comp.animation_name_to_start_frame_index.find(animation_name);
        if (anim == anim_comp.animation_name_to_start_frame_index.end()) {
            yorcvs::log("Entity: " + std::to_string(entityID) + " doesn't have an animation with the name " + animation_name,
                yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        if (anim->second > anim_comp.frames.size()) {
            yorcvs::log("Entity: " + std::to_string(entityID) + " animation: " + animation_name + " has the invalid ID: " + std::to_string(anim->second), yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        if (anim->first == anim_comp.current_animation_name) {
            return;
        }
        world->get_component<animationComponent>(entityID).current_animation_name = animation_name;
        world->get_component<animationComponent>(entityID).current_frame = anim->second;
    }

    void update(const float elapsed) const
    {
        for (const auto& ID : entityList->entitiesID) {
            auto& anim_comp = world->get_component<animationComponent>(ID);
            if (anim_comp.frames.empty()) {
                continue;
            }
            world->get_component<animationComponent>(ID).current_elapsed_time += elapsed;
            if (world->get_component<animationComponent>(ID).current_elapsed_time > std::get<2>(anim_comp.frames[anim_comp.current_frame])) {
                world->get_component<animationComponent>(ID).current_elapsed_time = 0;
                world->get_component<spriteComponent>(ID).src_rect = std::get<0>(anim_comp.frames[anim_comp.current_frame]);
                anim_comp.current_frame = std::get<1>(anim_comp.frames[anim_comp.current_frame]);
            }
        }
    }

    std::shared_ptr<yorcvs::EntitySystemList> entityList;
    yorcvs::ECS* world;
};
