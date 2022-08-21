#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <functional> //TODO : MAYBE REPLACE std::function

#include "utilities.h"

namespace yorcvs {

/**
 * @brief Manages resources to reduce reading multiple times from the disk
 *
 * @tparam assetType - the type of asset to manage
 */
template <typename assetType>
class asset_manager {
public:
    asset_manager() = default;
    asset_manager(const asset_manager& other) = delete;
    asset_manager(asset_manager&& other) = delete;
    asset_manager(std::function<std::shared_ptr<assetType>(const std::string& path)> pCtor, std::function<void(assetType*)> pDtor)
        : ctor(pCtor)
        , dtor(pDtor)
    {
    }
    asset_manager& operator=(const asset_manager& other) = delete;
    asset_manager& operator=(asset_manager&& other) = delete;
    ~asset_manager() = default;

    /**
     * @brief Loads the resource from the disk and returns a pointer to it.
     * Returns nullptr if the file is not found
     *
     * @param path path to resource
     * @return std::shared_ptr<assetType> pointer to the resource or nulllptr if it couldn't be found
     */
    [[nodiscard]] std::shared_ptr<assetType> load_from_file(const std::string& path)
    {
        if (path.empty()) {
            yorcvs::log("Attempt to pass an empty string to an assetManager", yorcvs::MSGSEVERITY::ERROR);
            return nullptr;
        }
        // search for the texture in the map
        const auto rez = assetMap.find(path.c_str());
        // return  it if it's there
        if (rez != assetMap.end()) {
            return rez->second;
        }
        yorcvs::log(std::string("Loading asset : ") + path);

        std::shared_ptr<assetType> ptr { nullptr, dtor };
        ptr = ctor(path);
        if (ptr == nullptr) {
            yorcvs::log("Could not create specified resource from " + path, yorcvs::MSGSEVERITY::ERROR);
            return nullptr;
        }
        assetMap.insert({ path, ptr });
        return ptr;
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
        for (auto it : assetMap) {
            yorcvs::log(std::string("Unloading asset") + it.first);
        }
        assetMap.clear();
    }
    std::unordered_map<std::string, std::shared_ptr<assetType>>& get_assetmap()
    {
        return assetMap;
    }
    const std::unordered_map<std::string, std::shared_ptr<assetType>>& get_assetmap() const
    {
        return assetMap;
    }

private:
    /**
     * @brief function that constructs a resource from file and returns it's pointer
     *
     */
    std::function<std::shared_ptr<assetType>(const std::string& path)> ctor;
    /**
     * @brief function that frees that memory
     *
     */
    std::function<void(assetType*)> dtor;
    std::unordered_map<std::string, std::shared_ptr<assetType>> assetMap;
};
} // namespace yorcvs
