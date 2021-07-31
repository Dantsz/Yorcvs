#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <functional> //TODO : MAYBE REPLACE std::function

#include "../common/log.h"

namespace yorcvs
{

/**
 * @brief Manages resources to reduce reading multiple times from the disk
 *
 * @tparam assetType - the type of asset to manage
 */
template <typename assetType> class AssetManager
{

  public:
    AssetManager() = default;
    AssetManager(std::function<assetType *(const std::string &path)> pCtor, std::function<void(assetType *)> pDtor)
    {
        ctor = pCtor;
        dtor = pDtor;
    }

    /**
     * @brief Loads the resource from the diosk and returns a pointer to it.
     * Returns nullptr if the file is not found
     *
     * @param path path to resource
     * @return std::shared_ptr<assetType>
     */
    std::shared_ptr<assetType> loadFromFile(const std::string &path)
    {
        if (path.empty())
        {
            yorcvs::log("Attempt to pass an empty string to an assetManager", yorcvs::ERROR);
            return nullptr;
        }
        // search for the texture in the map
        const auto rez = assetMap.find(path.c_str());
        // return  it if it's there
        if (rez != assetMap.end())
        {
            return rez->second;
        }
        yorcvs::log(std::string("Loading asset : ") + path);
        std::shared_ptr<assetType> shrptr = std::shared_ptr<assetType>(ctor(path), dtor);
        assetMap.insert({path, shrptr});
        return shrptr;
    }
    /**
     * @brief Removes unused assets from the map
     *
     */
    void refresh();
    /**
     * @brief Clears the assetmanager
     *
     */
    void cleanup()
    {
        for (auto it : assetMap)
        {
            yorcvs::log(std::string("Unloading asset") + it.first);
            it.second.reset();
        }
    }

  private:
    /**
     * @brief function that constructs a resource from file and returns it's pointer
     *
     */
    std::function<assetType *(const std::string &path)> ctor;
    /**
     * @brief function that frees that memory
     *
     */
    std::function<void(assetType *)> dtor;
    std::unordered_map<std::string, std::shared_ptr<assetType>> assetMap;
};

} // namespace yorcvs
