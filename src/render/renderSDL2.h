/**
 * @file renderSDL2.h
 * @author Dansz
 * @brief SDL2 renderer interface for yorcvs
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include "render.h"
#include "../assetmanager/assetmanager.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>



//SDL TEXTURE MANAGER
//default_delete needs to be specialized for SDL_Texture to be used with STL
    
template <>
 struct std::default_delete<SDL_Texture>
   {
     void operator()(SDL_Texture* p) { SDL_DestroyTexture(p); }
   };

template<>
 class yorcvs::AssetManager<SDL_Texture>
{
        public:
        std::shared_ptr<SDL_Texture> loadFromFile(const std::string& path, SDL_Renderer* renderer)
        {

        if(path.empty())
        {
                return nullptr;
        }
        //search for the texture in the map
        const auto rez = assetMap.find(path);
        //return  it if it's there
        if(rez != assetMap.end())
        {
            return rez->second;
        }
        
        SDL_Surface* surf = IMG_Load(path.c_str());
        //can't use make_shared because SDL_Texture is not a complete type?
        auto texture = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer,surf),[](SDL_Texture* tex){SDL_DestroyTexture(tex);});
        assetMap.insert({path,texture});
        return texture;
        

                
        }
        void refresh()
        {

        }

        void cleanup()
        {
            for(auto it : assetMap)
            {
                it.second.reset();
            }
        }
        private:
        std::unordered_map<std::string,std::shared_ptr<SDL_Texture>> assetMap{};
 };


namespace yorcvs
{   

    class SDL2
    {
      public:
      const char* name = "SDL2";
    };

    template<>
    class Texture<SDL2>
    {
        public:
        std::shared_ptr<SDL_Texture> SDLtex;
    };


    /**
     * @brief interfaceWindow that uses SDL2 api
     * 
     *
     */
    template<>
    class Window<yorcvs::SDL2>
    {
        public:

            void Init(const std::string& name , size_t width , size_t height)
            {
                 if (SDL_Init(SDL_INIT_VIDEO ) < 0) {
                        std::cout << "Error SDL2 Initialization : " << SDL_GetError();
                    }
                        
                    if (IMG_Init(IMG_INIT_PNG) == 0) {
                        std::cout << "Error SDL2_image Initialization";
                    }
                    if(TTF_Init() < 0)
                    {
                        std::cout<< "Font error" << '\n';
                    }
                 
                    sdlWindow  = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,static_cast<int>(width),static_cast<int>(height), SDL_WINDOW_OPENGL);
                    if (sdlWindow == nullptr) {
                        std::cout << "Error window creation";
                        
                    }

                    renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
                    if (renderer == nullptr) {
                        std::cout << "Error renderer creation";
                    }
            }



            void setSize(size_t width, size_t height) const
            {
                SDL_SetWindowSize(sdlWindow,static_cast<int>(width),static_cast<int>(height));
            }

            void cleanup() 
            {
                assetm.cleanup();
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(sdlWindow);
                IMG_Quit();
                SDL_Quit();
                TTF_Quit();	
            }
            void handleEvents()
            {
                while(SDL_PollEvent(&event) == 1)
                {
                   switch(event.type)
                   {
                       //QUIT EVENT
                       case SDL_QUIT:
                       break;

                       case SDL_WINDOWEVENT:
                        if(event.window.event == SDL_WINDOWEVENT_RESTORED)
                        {
                            isMinimized  = false;
                        } 
                        else if(event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                        {
                            isMinimized = true;
                        }

                       break;

                       default:
                       break;

                   }

                }
            }
            void drawSprite(const std::string& path,const Rect<float>& dstRect ,const Rect<size_t>& srcRect , double angle = 0.0 )
            {
                SDL_Rect sourceR = {static_cast<int>(srcRect.x),static_cast<int>(srcRect.y),static_cast<int>(srcRect.w),static_cast<int>(srcRect.h)};
                SDL_FRect dest = {dstRect.x,dstRect.y,dstRect.w,dstRect.h};
                SDL_RenderCopyExF(renderer,
                                  assetm.loadFromFile(path,renderer).get(),
                                  &sourceR,
                                  &dest,
                                  angle,nullptr,
                                  SDL_FLIP_NONE);
            }

         
            yorcvs::Texture<yorcvs::SDL2> createTextTexture(const std::string& path,const std::string& message, uint8_t r, uint8_t g, uint8_t b, uint8_t a,size_t charSize,size_t lineLength)
            {
               
                TTF_Font* font = TTF_OpenFont(path.c_str(),static_cast<int>(charSize));
                SDL_Surface* textSurf = TTF_RenderText_Blended_Wrapped(font,message.c_str(),{r,g,b,a},static_cast<uint32_t>(lineLength));
                Texture<yorcvs::SDL2> tex;
                tex.SDLtex =  std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer,textSurf),[](SDL_Texture* p){ SDL_DestroyTexture(p);});    
                SDL_FreeSurface(textSurf);
                TTF_CloseFont(font);
                return tex;
            }

            void drawText(const Texture<yorcvs::SDL2>& texture,const Rect<float>& dstRect) const
            {
                SDL_FRect dest = {dstRect.x,dstRect.y,dstRect.w,dstRect.h};
                SDL_RenderCopyF(renderer,texture.SDLtex.get(),nullptr,&dest);
            }


            void present() const
            {
                if(!isMinimized)
                {
                     SDL_RenderPresent(renderer);
                }
            }

            void clear() const
            {
                 if(!isMinimized)
                 {
                      SDL_RenderClear(renderer);
                 }
            }

        private:
        SDL_Window* sdlWindow = nullptr;    
        SDL_Renderer* renderer = nullptr;
        AssetManager<SDL_Texture> assetm{};
        AssetManager<TTF_Font> fontmanager{};
        bool isMinimized = false;
        SDL_Event event{};
    };
    
    

   
   




}
   
