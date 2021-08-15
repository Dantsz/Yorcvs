/**
 * @file renderSDL2.h
 * @author Dansz
 * @brief SDL2 interface for yorcvs
 * @version 0.1
 * @date 2021-07-28
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include "common/assetmanager.h"
#include "common/log.h"
#include "common/window.h"

#include <SDL_render.h>
#include <SDL_video.h>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <functional> // callbacks need to be stored in a vector
// SDL TEXTURE MANAGER
// default_delete needs to be specialized for SDL_Texture to be used with STL

template <> struct std::default_delete<SDL_Texture>
{
    void operator()(SDL_Texture *p)
    {
        SDL_DestroyTexture(p);
    }
};

namespace yorcvs
{

/**
 * @brief Interface concept for defining rendering objects that are implemented using SDL2
 *
 */
class SDL2
{
  public:
    const char *name = "SDL2";
};

template <> class Texture<yorcvs::SDL2>
{
  public:
    std::shared_ptr<SDL_Texture> SDLtex;
};

template <> class Text<yorcvs::SDL2>
{
  public:
    std::unique_ptr<SDL_Texture> SDLtex = nullptr;
    std::string message;
    std::string fontPath;
    SDL_Color color = {255, 255, 255, 255};
    int charSize = 0;
    uint32_t lineLength = 0;
};

template <> class Callback<yorcvs::SDL2>
{
  public:
    std::function<void(const SDL_Event &)> func;
};

template <> class Key<yorcvs::SDL2>
{
  public:
    Key<yorcvs::SDL2>() = default;
    Key<yorcvs::SDL2>(SDL_Scancode scancode)
    {
        sdlScancode = scancode;
    }

    SDL_Scancode sdlScancode;
};

/**
 * @brief interfaceWindow that uses SDL2 api
 *
 *
 */
template <> class Window<yorcvs::SDL2>
{
  public:
    Window<yorcvs::SDL2>(const char* name , size_t width , size_t height)
    {
        SDL_version sdlversion{};
        SDL_GetVersion(&sdlversion);

        const SDL_version *sdlimageversion = IMG_Linked_Version();
        const SDL_version *sdlttfversion = TTF_Linked_Version();

        // TODO : replace with std::format
        yorcvs::log(std::string("======SDL2 Version=======\nUsing SDL2 rendering\n") +
                        "COMPILED with SDL2 version: " + std::to_string(SDL_MAJOR_VERSION) + ' ' +
                        std::to_string(SDL_MINOR_VERSION) + ' ' + std::to_string(SDL_PATCHLEVEL) + '\n' +
                        "LINKED SDL2 version: " + std::to_string(sdlversion.major) + ' ' +
                        std::to_string(sdlversion.minor) + ' ' + std::to_string(sdlversion.patch) + '\n' +
                        "COMPILED with SDL2_image version : " + std::to_string(SDL_IMAGE_MAJOR_VERSION) + ' ' +
                        std::to_string(SDL_IMAGE_MINOR_VERSION) + ' ' + std::to_string(SDL_IMAGE_PATCHLEVEL) + '\n' +
                        "LINKED SDL2_image version : " + std::to_string(sdlimageversion->major) + ' ' +
                        std::to_string(sdlimageversion->minor) + ' ' + std::to_string(sdlimageversion->patch) + '\n' +
                        "COMPILED with SDL2_ttf version : " + std::to_string(SDL_TTF_COMPILEDVERSION) + '\n' +
                        "LINKED with SDL2_ttf version: " + std::to_string(sdlttfversion->major) + ' ' +
                        std::to_string(sdlttfversion->minor) + ' ' + std::to_string(sdlttfversion->patch) + '\n' +
                        "=============\n ",
                    yorcvs::MSGSEVERITY::INFO);

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            yorcvs::log("Error initializing SDL2", yorcvs::MSGSEVERITY::ERROR);
        }

        if (IMG_Init(IMG_INIT_PNG) == 0)
        {
            yorcvs::log("Error initializing SDL2_image", yorcvs::MSGSEVERITY::ERROR);
        }
        if (TTF_Init() < 0)
        {
            yorcvs::log("Error initializing SDL2_TTF", yorcvs::MSGSEVERITY::ERROR);
        }

        sdlWindow = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (sdlWindow == nullptr)
        {
            yorcvs::log("Error creating SDL2 window", yorcvs::MSGSEVERITY::ERROR);
        }
        
        renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
        
        if (renderer == nullptr)
        {
            yorcvs::log("Error creating SDL2 renderer", yorcvs::MSGSEVERITY::ERROR);
        }
        SDL_RendererInfo renderInfo{};
        SDL_GetRendererInfo(renderer, &renderInfo);
        const std::string softwareRenderer =
            static_cast<bool>(renderInfo.flags & SDL_RENDERER_SOFTWARE) ? "true" : "false";
        const std::string acceleratedRenderer =
            static_cast<bool>(renderInfo.flags & SDL_RENDERER_ACCELERATED) ? "true" : "false";
        const std::string vsyncRenderer =
            static_cast<bool>(renderInfo.flags & SDL_RENDERER_PRESENTVSYNC) ? "true" : "false";
        const std::string textureRender =
            static_cast<bool>(renderInfo.flags & SDL_RENDERER_TARGETTEXTURE) ? "true" : "false";
        // TODO: DO WITH STD::FORMAT
        yorcvs::log(std::string("====RenderInfo====\n") + "Renderer : " + renderInfo.name + '\n' +
                        "Software Renderer: " + softwareRenderer + '\n' +
                        "Accelerated Renderer: " + acceleratedRenderer + '\n' + "Vsync Enabled : " + vsyncRenderer +
                        '\n' + "Can render to texture: " + textureRender + '\n' +
                        "Maximum texture width: " + std::to_string(renderInfo.max_texture_width) + '\n' +
                        "Maximum texture height: " + std::to_string(renderInfo.max_texture_height) + '\n',
                    yorcvs::MSGSEVERITY::INFO);

        yorcvs::log("creating texture manager");
        assetm = std::make_unique<yorcvs::AssetManager<SDL_Texture>>(
            [&](const std::string &path) {
                SDL_Surface *surf = IMG_Load(path.c_str());
                SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_FreeSurface(surf);
                return tex;
            },
            [](SDL_Texture *p) { SDL_DestroyTexture(p); });
    }
   

    void set_size(size_t width, size_t height) const
    {
        SDL_SetWindowSize(sdlWindow, static_cast<int>(width), static_cast<int>(height));
    }

    yorcvs::Vec2<float> get_size()
    {
        int x = 0;
        int y = 0;
        SDL_GetWindowSize(sdlWindow,&x,&y);
        return {static_cast<float>(x),static_cast<float>(y)};
    }

    void cleanup()
    {
        assetm->cleanup();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(sdlWindow);
        IMG_Quit();
        SDL_Quit();
        TTF_Quit();
    }
    void handle_events()
    {
        while (SDL_PollEvent(&event) == 1)
        {
            for (const auto &f : callbacks)
            {
                f.func(event);
            }
            switch (event.type)
            {
            // QUIT EVENT
            case SDL_QUIT:
                isActive = false;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESTORED)
                {
                    isMinimized = false;
                }
                else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                {
                    isMinimized = true;
                }

                break;

            default:
                break;
            }
        }
    }

    void draw_sprite(const std::string &path, const Rect<float> &dstRect, const Rect<size_t> &srcRect,
                     double angle = 0.0)
    {
        if (!isMinimized)
        {
            // NOTE: SDL_rendercopyF exists for >SDL 2.0.10
            SDL_Rect sourceR = {static_cast<int>(srcRect.x), static_cast<int>(srcRect.y), static_cast<int>(srcRect.w),
                                static_cast<int>(srcRect.h)};
            SDL_Rect dest = {static_cast<int>(dstRect.x - offset.x), static_cast<int>(dstRect.y - offset.y),
                             static_cast<int>(dstRect.w), static_cast<int>(dstRect.h)};
            SDL_RenderCopyEx(renderer, assetm->load_from_file(path).get(), &sourceR, &dest, angle, nullptr,
                             SDL_FLIP_NONE);
        }
    }

    void draw_sprite(const std::string &path, const yorcvs::Vec2<float> &dstRectPos,
                     const yorcvs::Vec2<float> &dstRectSize, const yorcvs::Rect<size_t> &srcRect, double angle = 0.0)
    {
        if (!isMinimized)
        {
            // NOTE: SDL_rendercopyF exists for >SDL 2.0.10
            SDL_Rect sourceR = {static_cast<int>(srcRect.x), static_cast<int>(srcRect.y), static_cast<int>(srcRect.w),
                                static_cast<int>(srcRect.h)};
            SDL_Rect dest = {static_cast<int>(dstRectPos.x - offset.x), static_cast<int>(dstRectPos.y - offset.y),
                             static_cast<int>(dstRectSize.x), static_cast<int>(dstRectSize.y)};
            SDL_RenderCopyEx(renderer, assetm->load_from_file(path).get(), &sourceR, &dest, angle, nullptr,
                             SDL_FLIP_NONE);
        }
    }

    [[nodiscard]] Texture<yorcvs::SDL2> create_texture(const std::string &path)
    {
        Texture<yorcvs::SDL2> tex;
        tex.SDLtex = assetm->load_from_file(path);
        return tex;
    }
    void draw_sprite(const Texture<yorcvs::SDL2> &texture, const yorcvs::Rect<float> &dstRect,
                     const yorcvs::Rect<size_t> &srcRect, double angle = 0.0)
    {
        if (!isMinimized)
        {
            // NOTE: SDL_rendercopyF exists for >SDL 2.0.10
            SDL_Rect sourceR = {static_cast<int>(srcRect.x), static_cast<int>(srcRect.y), static_cast<int>(srcRect.w),
                                static_cast<int>(srcRect.h)};
            SDL_Rect dest = {static_cast<int>(dstRect.x - offset.x), static_cast<int>(dstRect.y - offset.y),
                             static_cast<int>(dstRect.w), static_cast<int>(dstRect.h)};
            SDL_RenderCopyEx(renderer, texture.SDLtex.get(), &sourceR, &dest, angle, nullptr, SDL_FLIP_NONE);
        }
    }

    void draw_texture(const Texture<yorcvs::SDL2> &texture, const yorcvs::Vec2<float> &dstRectPos,
                      const yorcvs::Vec2<float> &dstRectSize, const yorcvs::Rect<size_t> &srcRect, double angle = 0.0)
    {
        if (!isMinimized)
        {
            // NOTE: SDL_rendercopyF exists for >SDL 2.0.10
            SDL_Rect sourceR = {static_cast<int>(srcRect.x), static_cast<int>(srcRect.y), static_cast<int>(srcRect.w),
                                static_cast<int>(srcRect.h)};
            SDL_FRect dest = {static_cast<float>(dstRectPos.x - offset.x), static_cast<float>(dstRectPos.y - offset.y),
                             static_cast<float>(dstRectSize.x), static_cast<float>(dstRectSize.y)};
            SDL_RenderCopyExF(renderer, texture.SDLtex.get(), &sourceR, &dest, angle, nullptr, SDL_FLIP_NONE);
        }
    }

    [[nodiscard]] Text<yorcvs::SDL2> create_text(const std::string &path, const std::string &message, unsigned char r,
                                                 unsigned char g, unsigned char b, unsigned char a, size_t charSize,
                                                 size_t lineLength)
    {
        Text<yorcvs::SDL2> text;
        text.fontPath = path;
        text.message = message;
        text.color = {r, g, b, a};
        text.charSize = static_cast<int>(charSize);
        text.lineLength = static_cast<uint32_t>(lineLength);
        setup_texture(text);
        return text;
    }

    void draw_text(const Text<yorcvs::SDL2> &text, const Rect<float> &dstRect) const
    {
        if (!isMinimized)
        {
            // NOTE: SDL_rendercopyF exists for >SDL 2.0.10
            SDL_Rect dest = {static_cast<int>(dstRect.x), static_cast<int>(dstRect.y), static_cast<int>(dstRect.w),
                             static_cast<int>(dstRect.h)};
            SDL_RenderCopy(renderer, text.SDLtex.get(), nullptr, &dest);
        }
    }

    void set_text_message(Text<yorcvs::SDL2> &text, const std::string &message)
    {
        text.message = message;
        text.SDLtex.reset();
        setup_texture(text);
    }

    void set_text_color(Text<yorcvs::SDL2> &text, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        text.color = {r, g, b, a};
        setup_texture(text);
    }
    void set_text_char_size(Text<yorcvs::SDL2> &text, size_t charSize)
    {
        text.charSize = static_cast<int>(charSize);
        setup_texture(text);
    }
    void set_text_line_length(Text<yorcvs::SDL2> &text, size_t lineLength)
    {
        text.lineLength = static_cast<uint32_t>(lineLength);
        setup_texture(text);
    }
    void set_text_font(Text<yorcvs::SDL2> &text, const std::string &fontPath)
    {
        text.fontPath = fontPath;
        setup_texture(text);
    }

    void present() const
    {
        if (!isMinimized)
        {
            SDL_RenderPresent(renderer);
        }
    }

    void clear() const
    {
        if (!isMinimized)
        {
            SDL_RenderClear(renderer);
        }
    }
    // sdl doesn't need the window to get cursor position so this is static
    yorcvs::Vec2<float> get_cursor_position()
    {
        SDL_GetMouseState(&mouseX, &mouseY);
        return Vec2<float>(static_cast<float>(mouseX), static_cast<float>(mouseY));
    }

    size_t register_callback(const Callback<yorcvs::SDL2> &callback)
    {
        callbacks.push_back(callback);
        return callbacks.size() - 1;
    }

    void unregister_callback(size_t index)
    {
        callbacks.erase(callbacks.begin() + static_cast<ptrdiff_t>(index)); // TODO: static cast looks ugly
    }
    // this can take a SDL_scancode directly
    //
    [[nodiscard]] bool is_key_pressed(yorcvs::Key<yorcvs::SDL2> key)
    {
        keys = SDL_GetKeyboardState(nullptr);
        // TODO: REPLACE WITH SPAN?
        return keys[key.sdlScancode] ? 1 : 0; // NOLINT
    }

    void set_drawing_offset(const yorcvs::Vec2<float> &newOffset)
    {
        offset = newOffset;
    }

    yorcvs::Vec2<float> get_window_size()
    {
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(sdlWindow, &width, &height);
        return yorcvs::Vec2<float>(static_cast<float>(width), static_cast<float>(height));
    }

    void set_viewport(const yorcvs::Rect<float>& viewport)
    {
        SDL_Rect vp = {static_cast<int>(viewport.x), static_cast<int>(viewport.y), static_cast<int>(viewport.w),
                             static_cast<int>(viewport.h)};
        SDL_RenderSetViewport(renderer,&vp);
      
    }

    yorcvs::Rect<float> get_viewport()
    {
        SDL_Rect vp{};
        SDL_RenderGetViewport(renderer,&vp);
        return {static_cast<float>(vp.x),static_cast<float>(vp.y),static_cast<float>(vp.w),static_cast<float>(vp.h)};

    }

    void set_render_scale(const yorcvs::Vec2<float>& scale)
    {
        SDL_RenderSetScale(renderer,scale.x,scale.y);
    }

    yorcvs::Vec2<float> get_render_scale()
    {
        yorcvs::Vec2<float> scale{};
        SDL_RenderGetScale(renderer,&scale.x,&scale.y);
        return scale;
    }

    bool isActive = true;

  private:
    void setup_texture(yorcvs::Text<yorcvs::SDL2> &text)
    {
        TTF_Font *font = TTF_OpenFont(text.fontPath.c_str(), text.charSize);
        if(font == nullptr)
        {
            yorcvs::log("FONT " +  text.fontPath + " could not be found",yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        SDL_Surface *textSurf = TTF_RenderText_Blended_Wrapped(font, text.message.c_str(), text.color, text.lineLength);
        text.SDLtex = std::unique_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer, textSurf));
        SDL_FreeSurface(textSurf);
        TTF_CloseFont(font);
    }
    SDL_Event event{};
    std::vector<Callback<yorcvs::SDL2>> callbacks{};
    yorcvs::Vec2<float> offset = {0.0f, 0.0f};
    SDL_Window *sdlWindow = nullptr;
    SDL_Renderer *renderer = nullptr;
    std::unique_ptr<AssetManager<SDL_Texture>> assetm = nullptr;
    unsigned char const *keys{};
    int mouseX{};
    int mouseY{};
    bool isMinimized = false;
};

} // namespace yorcvs
