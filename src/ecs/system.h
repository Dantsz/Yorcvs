#pragma once

#include "entity.h"
#include <algorithm>
#include <memory>
#include <unordered_map>

template <typename T> typename std::vector<T>::iterator insert_sorted(std::vector<T> &vec, T const &item)
{
    if (std::binary_search(vec.begin(), vec.end(), item))
    {
        return vec.end();
    }
    auto upper_bound = std::upper_bound(vec.begin(), vec.end(), item);
    return vec.insert(upper_bound, item);
}

namespace yorcvs
{

class ECS; // forward declaration

/**
 * @brief Contains a list of entities matching parents signature
 *
 */
class EntitySystemList
{
  public:
    // the id of the entities the system works on
    std::vector<size_t> entitiesID;
};

// concept for a valid system
// must have a vector of size_t
template <typename systemt>
concept systemT = requires(systemt sys)
{
    //std::same_as<decltype(sys.entityList), std::shared_ptr<yorcvs::EntitySystemList>>;
    {sys.entityList->entitiesID[0]};
    {sys.entityList->entitiesID.size()};
};

class SystemManager
{
  public:
    // creates a system of type T and puts it in the map
    template <systemT T> bool register_system(T &system)
    {
        const char *systemType = typeid(T).name();

        // if the system is already present
        if (typetosystem.find(systemType) != typetosystem.end())
        {
            yorcvs::log("Unable to register system: system is already registered.", yorcvs::ERROR);
            return false;
        }
        std::shared_ptr<EntitySystemList> systemEVec = std::make_shared<EntitySystemList>();
        typetosystem.insert({systemType, systemEVec});
        set_signature<T>(std::vector<bool>{});
        system.entityList = systemEVec;
        return true;
    }
    // sets the signature of the system with type T
    template <systemT T> void set_signature(const std::vector<bool> &signature)
    {
        const char *systemType = typeid(T).name();

        // if the system is not found  //throw
        if (typetosystem.find(systemType) == typetosystem.end())
        {
            yorcvs::log("Unable to set the signature: system does not exist.", yorcvs::ERROR);
        }

        typetosignature[systemType] = signature;
    }

    // gets signature of a system
    template <systemT T> std::vector<bool> get_system_signature()
    {

        const char *systemType = typeid(T).name();

        // if the system is not found  //throw
        if (typetosystem.find(systemType) == typetosystem.end())
        {
            yorcvs::log("Unable to fetch the signature: system does not exist.");
        }

        return typetosignature[systemType];
    }

    // erase entity from all systems
    void OnEntityDestroy(const size_t entityID)
    {
        for (auto const &it : typetosystem)
        {
            it.second->entitiesID.erase(
                std::remove(it.second->entitiesID.begin(), it.second->entitiesID.end(), entityID),
                it.second->entitiesID.end());
        }
    }

    // Notify each system that an entity's signature changed
    void on_entity_signature_change(const size_t entityID, std::vector<bool> &signature)
    {
        for (auto const &it : typetosystem)
        {
            auto const &type = it.first;
            auto const &system = it.second;
            auto const &systemSignature = typetosignature[type];
            if (compare_entity_to_system(signature, systemSignature))
            {
                // TODO : MAKE A METHOD TO SYSTEM , method needs to be virtual /Onewntitierase/insert
                insert_sorted(system->entitiesID, entityID);
            }
            else
            {
                system->entitiesID.erase(std::remove(system->entitiesID.begin(), system->entitiesID.end(), entityID),
                                         system->entitiesID.end());
            }
        }
    }

    // compare the signature//return 1  if they are equal
    static bool compare_signatures(const std::vector<bool> &signature1, const std::vector<bool> &signature2)
    {
        if (signature1.size() != signature2.size())
        {
            return false;
        }
        for (size_t i = 0; i < signature1.size(); i++)
        {
            // if a signature  is missing somewhere
            if (signature1[i] != signature2[i])
            {
                return false;
            }
        }
        return true;
    }
    /**
     * @brief compares the signature of an entity to a specific system and if they are compatible
     *
     * @param entity_s The entity
     * @param system_s The system
     * @return true the entity has all the systems components
     * @return false if the system has a component but the entity does not or if the system signature doesn't match the
     * entity
     */
    static bool compare_entity_to_system(const std::vector<bool> &entity_s, const std::vector<bool> &system_s)
    {

        /// if the system signature doesn't match the entity
        if (system_s.size() > entity_s.size())
        {
            return false;
        }
        // if a signature has a system but the entity does not
        // return false
        for (size_t i = 0; i < system_s.size(); i++)
        {
            if (system_s[i] && !entity_s[i])
            {
                return false;
            }
        }
        return true;
    }

    // get the signature of a system based on type
    std::unordered_map<const char *, std::vector<bool>> typetosignature{};
    // get the system based on type
    std::unordered_map<const char *, std::shared_ptr<EntitySystemList>> typetosystem{};
};

} // namespace yorcvs
