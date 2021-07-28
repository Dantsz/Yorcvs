#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
/**
 * @brief Manages resources to reduce reading multiple times from the disk
 * 
 * @tparam assetType - the type of asset to manage
 * 
 * 
 */
template<typename assetType>
class assetManager
{
    public:

    /**
     * @brief Loads the resource from the diosk and returns a pointer to it. Returns nullptr if the file is not found
     * 
     * @param path path to resource
     * @return std::shared_ptr<assetType> 
     */
    std::shared_ptr<assetType> loadFromFile(const std::string& path)
    {

    }
    /**
     * @brief Removes unused assets from the map 
     * 
    */
    void refresh()
    {

    }
    private:

    std::unordered_map<const char*,std::shared_ptr<assetType>> assetMap;
};

//SDL TEXTURE MANAGER
//default_delete needs to be specialized for SDL_Texture to be used with STL
extern SDL_Renderer* renderer;
template <>
struct std::default_delete<SDL_Texture>{
    void operator()(SDL_Texture* p) { SDL_DestroyTexture(p); }
};

template<>
class assetManager<SDL_Texture>
{
    public:
    std::shared_ptr<SDL_Texture> loadFromFile(const std::string& path)
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
    private:
    std::unordered_map<std::string,std::shared_ptr<SDL_Texture>> assetMap{};
};


