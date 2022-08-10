#pragma once
#include "../common/ecs.h"
#include "../engine/window/eventhandler.h"
#include "../engine/window/window.h"
#include "imgui.h"
template <typename eventhandler_impl, typename window_impl>
class entityInteractionWidget {
public:
    entityInteractionWidget(yorcvs::event_handler<eventhandler_impl>& event_handler, yorcvs::window<window_impl>& window, yorcvs::ECS& world)
        : event_handler(&event_handler)
        , window(&window)
        , world(&world)
        , entity_is_clicked_callback(event_handler.add_callback_on_event(yorcvs::Events::Type::MOUSE_CLICKED,
              [&](const yorcvs::event&) {

              }))
    {
    }
    ~entityInteractionWidget()
    {
        window = nullptr;
        event_handler->unregister_callback(entity_is_clicked_callback);
    }

private:
    yorcvs::event_handler<eventhandler_impl>* const event_handler;
    yorcvs::window<window_impl>* const window;
    yorcvs::ECS* const world;
    const size_t entity_is_clicked_callback;
};
