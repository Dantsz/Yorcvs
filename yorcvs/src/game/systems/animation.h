#pragma once
#include "../../common/ecs.h"
#include "../components.h"

/**
 * @brief Handles Animation
 *
 */
class animation_system {
public:
    explicit animation_system(yorcvs::ECS* parent)
        : world(parent)
    {
        world->register_system<animation_system>(*this);
        world->add_criteria_for_iteration<animation_system, animation_component, sprite_component>();
    }

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
     * @param world
     * @param entityID
     * @param animation_name
     */
    static void set_animation_global(yorcvs::ECS* world, size_t entityID, const std::string& animation_name)
    {
        const auto& anim_comp = world->get_component<animation_component>(entityID);
        if (!world->has_components<animation_component>(entityID)) {
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
        world->get_component<animation_component>(entityID).current_elapsed_time = 0.0f;
        world->get_component<animation_component>(entityID).current_animation_name = animation_name;
        world->get_component<animation_component>(entityID).current_frame = anim->second;
    }
    /**
     * @brief Set which animation to be used
     *
     * @param entity
     * @param animation_name
     */
    static void set_animation_global(const yorcvs::Entity& entity, const std::string& animation_name)
    {
        yorcvs::ECS* world = entity.parent;
        set_animation_global(world, entity.id, animation_name);
    }
    void set_animation(size_t entityID, const std::string& animation_name)
    {
        set_animation_global(this->world, entityID, animation_name);
    }

    void update(const float elapsed) const
    {
        for (const auto& ID : *entityList) {
            auto& anim_comp = world->get_component<animation_component>(ID);
            if (anim_comp.frames.empty()) {
                continue;
            }
            world->get_component<animation_component>(ID).current_elapsed_time += elapsed;
            if (world->get_component<animation_component>(ID).current_elapsed_time > std::get<2>(anim_comp.frames[anim_comp.current_frame])) {
                world->get_component<animation_component>(ID).current_elapsed_time = 0;
                world->get_component<sprite_component>(ID).src_rect = std::get<0>(anim_comp.frames[anim_comp.current_frame]);
                anim_comp.current_frame = std::get<1>(anim_comp.frames[anim_comp.current_frame]);
            }
        }
    }

    std::shared_ptr<yorcvs::entity_system_list> entityList;
    yorcvs::ECS* world;
};
