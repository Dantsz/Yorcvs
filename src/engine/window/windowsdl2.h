#pragma once
#include "SDL_image.h"
#include "window.h"
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <SDL_ttf.h>
#include <SDL_version.h>
#include <SDL_video.h>


#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_sdl.h"


#include <memory>
#include <string>
#include <string_view>


#include "../../common/assetmanager.h"
#include "eventhandlersdl2.h"
namespace yorcvs
{
    class sdl2_window : public window<sdl2_window>, public yorcvs::eventhandler_sdl2
    {
    public:
        sdl2_window()
        {
            const std::string name = "Yorcvs";
            const size_t width = 960;
            const size_t height = 480;

            SDL_version sdlversion{};
            SDL_GetVersion(&sdlversion);

            const SDL_version* sdlimageversion = IMG_Linked_Version();
            const SDL_version* sdlttfversion = TTF_Linked_Version();

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

            sdlWindow =
                SDL_CreateWindow(name.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int>(width),
                    static_cast<int>(height), SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
            if (sdlWindow == nullptr)
            {
                yorcvs::log("Error creating SDL2 window", yorcvs::MSGSEVERITY::ERROR);
            }
            renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);

            if (renderer == nullptr)
            {
                yorcvs::log("Error creating SDL2 renderer", yorcvs::MSGSEVERITY::ERROR);
            }
            ImGui::CreateContext();
            ImGuiSDL::Initialize(renderer, width, height);
            ImGui_ImplSDL2_InitForSDLRenderer(sdlWindow, renderer);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
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
                [&](const std::string& path) {
                    SDL_Surface* surf = nullptr;
                    SDL_RWops* rwop = SDL_RWFromFile(path.c_str(), "rb");

                    surf = IMG_Load_RW(rwop, 1);
                    if (surf == nullptr)
                    {
                        yorcvs::log(IMG_GetError(), yorcvs::MSGSEVERITY::ERROR);
                    }
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

                    SDL_FreeSurface(surf);

                    return tex;
                },
                [](SDL_Texture* p) { SDL_DestroyTexture(p); });
            //adding minimization
            add_callback([&isMinimized = this->isMinimized](const yorcvs::event& e){
                if (e.get_type() == yorcvs::Events::Type::WINDOW_MINIMIZED)
                {
                    isMinimized = true;
                }
                if (e.get_type() == yorcvs::Events::Type::WINDOW_RESTORED)
                {
                    isMinimized = false;
                }
                });
        }
        ~sdl2_window() override
        {
            ImGuiSDL::Deinitialize();
            ImGui::DestroyContext();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(sdlWindow);
            IMG_Quit();
            TTF_Quit();
            SDL_Quit();
        }

        void clear()
        {
            if (!isMinimized)
            {
                SDL_RenderClear(renderer);
            }
        }
        void present()
        {
            if (!isMinimized)
            {
                SDL_RenderPresent(renderer);
            }
        }
        void draw_texture(const std::string& path, const yorcvs::Rect<float>& dstRect, const yorcvs::Rect<size_t>& srcRect,
            double angle = 0.0)
        {
            if (!isMinimized)
            {
                // NOTE: SDL_rendercopyF exists for >SDL 2.0.10
                SDL_Rect sourceR = { static_cast<int>(srcRect.x), static_cast<int>(srcRect.y), static_cast<int>(srcRect.w),
                                    static_cast<int>(srcRect.h) };
                SDL_FRect dest = { static_cast<float>(dstRect.x - offset.x), static_cast<float>(dstRect.y - offset.y),
                                  static_cast<float>(dstRect.w), static_cast<float>(dstRect.h) };
                SDL_RenderCopyExF(renderer, assetm->load_from_file(path).get(), &sourceR, &dest, angle, nullptr,
                    SDL_FLIP_NONE);
            }
        }

        void draw_texture(const std::string& path, const yorcvs::Vec2<float>& dstRectPos,
            const yorcvs::Vec2<float>& dstRectSize, const yorcvs::Rect<size_t>& srcRect, double angle = 0.0)
        {
            if (!isMinimized)
            {
                // NOTE: SDL_rendercopyF exists for >SDL 2.0.10
                SDL_Rect sourceR = { static_cast<int>(srcRect.x), static_cast<int>(srcRect.y), static_cast<int>(srcRect.w),
                                    static_cast<int>(srcRect.h) };
                SDL_FRect dest = { static_cast<float>(dstRectPos.x - offset.x), static_cast<float>(dstRectPos.y - offset.y),
                                  static_cast<float>(dstRectSize.x), static_cast<float>(dstRectSize.y) };
                SDL_RenderCopyExF(renderer, assetm->load_from_file(path).get(), &sourceR, &dest, angle, nullptr,
                    SDL_FLIP_NONE);
            }
        }

        void draw_text(const std::string&/*font_path*/, const std::string&/*message*/, const yorcvs::Rect<float>& /*dstRect*/, unsigned char /*r*/, unsigned char /*g*/,
            unsigned char /*b*/, unsigned char /*a*/, size_t /*charSize*/, size_t /*lineLength*/)
        {
            // if (!isMinimized)
            // {
            // TTF_Font *font = TTF_OpenFont(font_path.c_str(), static_cast<int>(charSize));
            // if (font == nullptr)
            // {
            //     yorcvs::log("FONT " + font_path + " could not be found", yorcvs::MSGSEVERITY::ERROR);
            //     return;
            // }
            // SDL_Color color = {r,g,b,a};
            // SDL_Surface *textSurf = TTF_RenderText_Blended_Wrapped(font, message.c_str(), color, lineLength);
            // const auto SDLtex = std::unique_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer, textSurf));
            // SDL_FreeSurface(textSurf);
            // TTF_CloseFont(font);

            //     // NOTE: SDL_rendercopyF exists for >SDL 2.0.10
            //     SDL_Rect dest = {static_cast<int>(dstRect.x), static_cast<int>(dstRect.y), static_cast<int>(dstRect.w),
            //                      static_cast<int>(dstRect.h)};
            //     SDL_RenderCopy(renderer, SDLtex.get(), nullptr, &dest);
            // }
        }

        void draw_rect(const yorcvs::Rect<float>& rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            SDL_FRect dest = { static_cast<float>(rect.x - offset.x), static_cast<float>(rect.y - offset.y),
                             static_cast<float>(rect.w), static_cast<float>(rect.h) };
            uint8_t r_old = 0;
            uint8_t g_old = 0;
            uint8_t b_old = 0;
            uint8_t a_old = 0;
            SDL_GetRenderDrawColor(renderer, &r_old, &g_old, &b_old, &a_old);
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            SDL_RenderFillRectF(renderer, &dest);
            SDL_SetRenderDrawColor(renderer, r_old, g_old, b_old, a_old);
        }
        void set_drawing_offset(const yorcvs::Vec2<float>& newOffset)
        {
            offset = newOffset;
        }

        yorcvs::Vec2<float> get_window_size()
        {
            int width = 0;
            int height = 0;
            SDL_GetWindowSize(sdlWindow, &width, &height);
            return { static_cast<float>(width), static_cast<float>(height) };
        }

        void set_viewport(const yorcvs::Rect<float>& viewport)
        {
            SDL_Rect vp = { static_cast<int>(viewport.x), static_cast<int>(viewport.y), static_cast<int>(viewport.w),
                           static_cast<int>(viewport.h) };
            SDL_RenderSetViewport(renderer, &vp);
        }

        yorcvs::Rect<float> get_viewport()
        {
            SDL_Rect vp{};
            SDL_RenderGetViewport(renderer, &vp);
            return { static_cast<float>(vp.x), static_cast<float>(vp.y), static_cast<float>(vp.w), static_cast<float>(vp.h) };
        }

        void set_render_scale(const yorcvs::Vec2<float>& scale)
        {
            SDL_RenderSetScale(renderer, scale.x, scale.y);
        }

        yorcvs::Vec2<float> get_render_scale()
        {
            yorcvs::Vec2<float> scale{};
            SDL_RenderGetScale(renderer, &scale.x, &scale.y);
            return scale;
        }
    private:
        std::unique_ptr<yorcvs::AssetManager<SDL_Texture>> assetm = nullptr;
        SDL_Window* sdlWindow = nullptr;
        SDL_Renderer* renderer = nullptr;
        bool isMinimized = false;
        yorcvs::Vec2<float> offset = { 0.0F,0.0F };
    };
}