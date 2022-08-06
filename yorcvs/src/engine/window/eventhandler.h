#pragma once
#include "../../common/types.h"
#include <functional>
namespace yorcvs {
namespace Events {
    enum Key {
        YORCVS_KEY_UKNOWN,
        YORCVS_KEY_W,
        YORCVS_KEY_A,
        YORCVS_KEY_C,
        YORCVS_KEY_S,
        YORCVS_KEY_D,
        YORCVS_KEY_R,
        YORCVS_KEY_E,
        YORCVS_KEY_I,
        YORCVS_KEY_K,
        YORCVS_KEY_Q,
        YORCVS_KEY_LCTRL,
        YORCVS_KEY_BACKSPACE,
        YORCVS_KEY_ENTER,
        YORCVS_KEY_TILDE
    };
    enum Type {
        UNKNOWN,
        KEYBOARD_PRESSED,
        MOUSE_CLICKED,
        TEXT_INPUT,
        WINDOW_MINIMIZED,
        WINDOW_RESTORED,
        WINDOW_QUIT
    };
}
class event {
public:
    event() = default;
    event(const event&) = default;
    event(event&&) = default;
    virtual ~event() = default;
    event& operator=(const event&) = default;
    event& operator=(event&&) = default;
    [[nodiscard]] virtual yorcvs::Events::Type get_type() const = 0;
    [[nodiscard]] virtual yorcvs::Events::Key get_key() const = 0;

private:
};

template <typename event_manager_implementation>
class event_handler {
public:
    void handle_events()
    {
        static_cast<event_manager_implementation&>(*this).handle_events();
    }
    void add_callback(const std::function<void(const yorcvs::event&)>& n_callback)
    {
        static_cast<event_manager_implementation&>(*this).add_callback(n_callback);
    }
    bool is_key_pressed(yorcvs::Events::Key key)
    {
        return static_cast<event_manager_implementation&>(*this).is_key_pressed(key);
    }
    yorcvs::Vec2<float> get_pointer_position()
    {
        return static_cast<event_manager_implementation&>(*this).get_pointer_position();
    }
};
}
