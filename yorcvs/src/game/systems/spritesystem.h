#pragma once
#include "../../common/ecs.h"
#include "../../engine/window/windowsdl2.h"
#include "../components.h"
/**
 * @brief Draws the entity to the window
 *
 */
class SpriteSystem {
public:
    SpriteSystem(yorcvs::ECS* parent, yorcvs::sdl2_window* parentWindow)
        : world(parent)
        , window(parentWindow)
    {
        world->register_system<SpriteSystem>(*this);
        world->add_criteria_for_iteration<SpriteSystem, position_component, sprite_component>();
    }
    void renderSprites(const yorcvs::vec2<float>& render_dimensions) const
    {
        yorcvs::vec2<float> rs = window->get_render_scale();
        window->set_render_scale(window->get_window_size() / render_dimensions);
        std::sort(entityList->begin(), entityList->end(), [&](size_t ID1, size_t ID2) {
            return (world->get_component<sprite_component>(ID1).offset.y + world->get_component<position_component>(ID1).position.y) < (world->get_component<sprite_component>(ID2).offset.y + world->get_component<position_component>(ID2).position.y);
        });
        for (const auto& ID : *entityList) {
            window->draw_texture(world->get_component<sprite_component>(ID).texture_path,
                world->get_component<sprite_component>(ID).offset + world->get_component<position_component>(ID).position,
                world->get_component<sprite_component>(ID).size,
                world->get_component<sprite_component>(ID).src_rect, 0.0);
        }
        std::sort(entityList->begin(), entityList->end(),
            [&](size_t ID1, size_t ID2) { return ID1 < ID2; });
        window->set_render_scale(rs);
    }

    std::shared_ptr<yorcvs::entity_system_list> entityList;

    yorcvs::ECS* world;

    yorcvs::sdl2_window* window;
};
