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
#include "../assetmanager/assetmanager.h"
#include "../common/log.h"
#include "window.h"

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
    void Init(const std::string &name, size_t width, size_t height)
    {
        SDL_version sdlversion{};
        SDL_GetVersion(&sdlversion);

        const SDL_version *sdlimageversion = IMG_Linked_Version();
        const SDL_version *sdlttfversion = TTF_Linked_Version();
        // TODO : replace with std::format
        yorcvs::log(std::string("Using SDL2 rendering\n") +
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
                        std::to_string(sdlttfversion->minor) + ' ' + std::to_string(sdlttfversion->patch) + '\n',
                    yorcvs::INFO);

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            yorcvs::log("Error initializing SDL2", yorcvs::ERROR);
        }

        if (IMG_Init(IMG_INIT_PNG) == 0)
        {
            yorcvs::log("Error initializing SDL2_image", yorcvs::ERROR);
        }
        if (TTF_Init() < 0)
        {
            yorcvs::log("Error initializing SDL2_TTF", yorcvs::ERROR);
        }

        sdlWindow = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_OPENGL);
        if (sdlWindow == nullptr)
        {
            yorcvs::log("Error creating SDL2 window", yorcvs::ERROR);
        }

        renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr)
        {
            yorcvs::log("Error creating SDL2 renderer", yorcvs::ERROR);
        }
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

    void setSize(size_t width, size_t height) const
    {
        SDL_SetWindowSize(sdlWindow, static_cast<int>(width), static_cast<int>(height));
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
    void handleEvents()
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

    void drawSprite(const std::string &path, const Rect<float> &dstRect, const Rect<size_t> &srcRect,
                    double angle = 0.0)
    {
        if (!isMinimized)
        {
            SDL_Rect sourceR = {static_cast<int>(srcRect.x), static_cast<int>(srcRect.y), static_cast<int>(srcRect.w),
                                static_cast<int>(srcRect.h)};
            SDL_FRect dest = {dstRect.x, dstRect.y, dstRect.w, dstRect.h};
            SDL_RenderCopyExF(renderer, assetm->loadFromFile(path).get(), &sourceR, &dest, angle, nullptr,
                              SDL_FLIP_NONE);
        }
    }

    yorcvs::Text<yorcvs::SDL2> createText(const std::string &path, const std::string &message, unsigned char r,
                                          unsigned char g, unsigned char b, unsigned char a, size_t charSize,
                                          size_t lineLength)
    {
        Text<yorcvs::SDL2> text;
        text.fontPath = path;
        text.message = message;
        text.color = {r, g, b, a};
        text.charSize = static_cast<int>(charSize);
        text.lineLength = static_cast<uint32_t>(lineLength);
        setupTexture(text);
        return text;
    }

    void drawText(const Text<yorcvs::SDL2> &text, const Rect<float> &dstRect) const
    {
        if (!isMinimized)
        {
            SDL_FRect dest = {dstRect.x, dstRect.y, dstRect.w, dstRect.h};
            SDL_RenderCopyF(renderer, text.SDLtex.get(), nullptr, &dest);
        }
    }

    void setTextMessage(Text<yorcvs::SDL2> &text, const std::string &message)
    {
        text.message = message;
        text.SDLtex.reset();
        setupTexture(text);
    }

    void setTextColor(Text<yorcvs::SDL2> &text, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        text.color = {r, g, b, a};
        setupTexture(text);
    }
    void setTextCharSize(Text<yorcvs::SDL2> &text, size_t charSize)
    {
        text.charSize = static_cast<int>(charSize);
        setupTexture(text);
    }
    void setTextLineLength(Text<yorcvs::SDL2> &text, size_t lineLength)
    {
        text.lineLength = static_cast<uint32_t>(lineLength);
        setupTexture(text);
    }
    void setTextFont(Text<yorcvs::SDL2> &text, const std::string &fontPath)
    {
        text.fontPath = fontPath;
        setupTexture(text);
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
    static yorcvs::Vec2<float> getCursorPosition()
    {
        int x{};
        int y{};
        SDL_GetMouseState(&x, &y);
        return Vec2<float>(static_cast<float>(x), static_cast<float>(y));
    }

    size_t registerCallback(const Callback<yorcvs::SDL2> &callback)
    {
        callbacks.push_back(callback);
        return callbacks.size() - 1;
    }

    void unregisterCallback(size_t index)
    {
        callbacks.erase(callbacks.begin() + static_cast<ptrdiff_t>(index)); // TODO: static cast looks ugly
    }
    // this can take a SDL_scancode directly
    //
    static bool isKeyPressed(yorcvs::Key<yorcvs::SDL2> key)
    {
        unsigned char const *keys = SDL_GetKeyboardState(nullptr);
        // TODO: REPLACE WITH SPAN?
        return keys[key.sdlScancode] ? 1 : 0; // NOLINT
    }

    bool isActive = true;

  private:
    void setupTexture(yorcvs::Text<yorcvs::SDL2> &text)
    {
        TTF_Font *font = TTF_OpenFont(text.fontPath.c_str(), text.charSize);
        SDL_Surface *textSurf = TTF_RenderText_Blended_Wrapped(font, text.message.c_str(), text.color, text.lineLength);
        text.SDLtex = std::unique_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer, textSurf));
        SDL_FreeSurface(textSurf);
        TTF_CloseFont(font);
    }

    SDL_Window *sdlWindow = nullptr;
    SDL_Renderer *renderer = nullptr;

    std::unique_ptr<AssetManager<SDL_Texture>> assetm = nullptr;
    bool isMinimized = false;
    SDL_Event event{};
    std::vector<Callback<yorcvs::SDL2>> callbacks{};
};

} // namespace yorcvs
