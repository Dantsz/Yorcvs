#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
namespace yorcvs
{

/**
 * @brief Manages resources to reduce reading multiple times from the disk
 * 
 * @tparam assetType - the type of asset to manage
 * 
 * 
 */
template<typename assetType>
class AssetManager
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
    /**
     * @brief Clears the assetmanager
     * 
     */
    void cleanup()
    {
        
    }

    private:

    std::unordered_map<const char*,std::shared_ptr<assetType>> assetMap;
};


}
