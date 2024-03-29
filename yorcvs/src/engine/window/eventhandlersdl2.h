#pragma once
#include "../../common/utilities.h"
#include "eventhandler.h"
#include "imgui_impl_sdl2.h"
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <SDL_video.h>
#include <array>
#include <functional>
#include <unordered_map>
namespace yorcvs {
class event_sdl2 : public event {
public:
    [[nodiscard]] yorcvs::Events::Type get_type() const override
    {
        return type;
    }
    [[nodiscard]] yorcvs::Events::Key get_key() const override
    {
        return key;
    }
    // update the type and key of the event based on  the sdl event
    void update_type(SDL_Event* e)
    {
        switch (e->type) {
        case SDL_KEYDOWN:
            type = yorcvs::Events::Type::KEYBOARD_PRESSED;
            break;
        case SDL_TEXTINPUT:
            type = yorcvs::Events::Type::TEXT_INPUT;
            break;
        case SDL_QUIT:
            type = yorcvs::Events::Type::WINDOW_QUIT;
            break;
        case SDL_MOUSEBUTTONDOWN:
            type = yorcvs::Events::Type::MOUSE_CLICKED;
            break;
        case SDL_WINDOWEVENT:
            switch (e->window.event) {
            case SDL_WINDOWEVENT_RESTORED:
                type = yorcvs::Events::Type::WINDOW_RESTORED;
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
                type = yorcvs::Events::Type::WINDOW_MINIMIZED;
                break;
            }
            break;
        default:
            type = yorcvs::Events::Type::UNKNOWN;
            break;
        }
    }

private:
    yorcvs::Events::Type type {};
    yorcvs::Events::Key key {};
    friend class eventhandler_sdl2;
};
class eventhandler_sdl2 : public event_handler<eventhandler_sdl2> {
public:
    eventhandler_sdl2()
        : key_to_impl(
            { { yorcvs::Events::Key::YORCVS_KEY_A, SDL_SCANCODE_A },
                { yorcvs::Events::Key::YORCVS_KEY_D, SDL_SCANCODE_D },
                { yorcvs::Events::Key::YORCVS_KEY_C, SDL_SCANCODE_C },
                { yorcvs::Events::Key::YORCVS_KEY_E, SDL_SCANCODE_E },
                { yorcvs::Events::Key::YORCVS_KEY_I, SDL_SCANCODE_I },
                { yorcvs::Events::Key::YORCVS_KEY_K, SDL_SCANCODE_K },
                { yorcvs::Events::Key::YORCVS_KEY_S, SDL_SCANCODE_S },
                { yorcvs::Events::Key::YORCVS_KEY_W, SDL_SCANCODE_W },
                { yorcvs::Events::Key::YORCVS_KEY_Q, SDL_SCANCODE_Q },
                { yorcvs::Events::Key::YORCVS_KEY_R, SDL_SCANCODE_R },
                { yorcvs::Events::Key::YORCVS_KEY_BACKSPACE, SDL_SCANCODE_BACKSPACE },
                { yorcvs::Events::Key::YORCVS_KEY_LCTRL, SDL_SCANCODE_LCTRL },
                { yorcvs::Events::Key::YORCVS_KEY_ENTER, SDL_SCANCODE_RETURN },
                { yorcvs::Events::Key::YORCVS_KEY_TILDE, SDL_SCANCODE_GRAVE } })
        , impl_to_key(yorcvs::build_reverse_unordered_map(key_to_impl))
    {
    }
    void handle_events()
    {
        while (SDL_PollEvent(&sdl_event) == 1) {
            ImGui_ImplSDL2_ProcessEvent(&sdl_event);
            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse) {
                continue;
            }
            i_event.update_type(&sdl_event);
            if (i_event.get_type() == yorcvs::Events::Type::KEYBOARD_PRESSED && impl_to_key.contains(sdl_event.key.keysym.scancode)) {
                i_event.key = impl_to_key.at(sdl_event.key.keysym.scancode);
            }
            for (const auto& [ID, f] : callbacks[i_event.get_type()]) {
                f(static_cast<yorcvs::event&>(*(&i_event)));
            }
        }
    }

    bool is_key_pressed(yorcvs::Events::Key key)
    {
        static auto keys = SDL_GetKeyboardState(nullptr);
        return keys[key_to_impl.at(key)] ? 1 : 0; // NOLINT
    }

    yorcvs::vec2<float> get_pointer_position()
    {
        int x = 0;
        int y = 0;
        SDL_GetMouseState(&x, &y);
        return yorcvs::vec2<float> { static_cast<float>(x), static_cast<float>(y) };
    }
    [[nodiscard]] size_t add_callback_on_event(yorcvs::Events::Type type, const std::function<void(const yorcvs::event&)>& callback)
    {
        callbacks[type].insert({ unassigned_callback_id, callback });
        return unassigned_callback_id++;
    }
    void unregister_callback(size_t callback_ID)
    {
        for (auto& callback_container : callbacks) {
            const auto it = callback_container.find(callback_ID);
            if (it != callback_container.end()) {
                callback_container.erase(it);
                return;
            }
        }
    }

private:
    SDL_Event sdl_event {};
    yorcvs::event_sdl2 i_event {};
    size_t unassigned_callback_id = 0;
    /**
     * @brief Holds callbacks to be called when a type of event happens
     */
    std::array<std::unordered_map<size_t, std::function<void(const yorcvs::event&)>>, yorcvs::Events::Type::TYPES> callbacks {};
    std::unordered_map<yorcvs::Events::Key, SDL_Scancode> key_to_impl {};
    std::unordered_map<SDL_Scancode, yorcvs::Events::Key> impl_to_key {};
};
}
