#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <SDL.h>
#include <string>
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
     * @brief Loads the resource from the diosk and returns a pointer to it
     * 
     * @param path 
     * @return std::shared_ptr<assetType> 
     */
    std::shared_ptr<assetType> loadFromFile(const char* path)
    {

    }

    std::shared_ptr<assetType> loadFromFile(const std::string& path)
    {
        loadFromFile(path.c_str());
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
template <>
struct std::default_delete<SDL_Texture>{
    void operator()(SDL_Texture* p) { SDL_DestroyTexture(p); }
};

template<>
class assetManager<SDL_Texture>
{
    public:
    std::shared_ptr<SDL_Texture> loadFromFile(const char* path)
    {
         
            
    }
    void refresh()
    {

    }
    private:
    std::unordered_map<const char*,std::shared_ptr<SDL_Texture>> assetMap;
};