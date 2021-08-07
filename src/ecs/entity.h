#pragma once
#include "../common/log.h"
#include <algorithm>
#include <vector>

// thanks to Austin Morlan for providing a ECS template

namespace yorcvs
{
/**
 * @brief Manages entity ids
 *
 */
class EntityManager
{
  public:
    /**
     * @brief  takes a freed or makes a new one if there's no id
     *
     * @return size_t ID of the new entity
     */
    size_t addEntity()
    {
        // if there isn't any in the the queue,create a new one and a new entry in the signature list
        if (freedIndices.empty())
        {
            entitySignatures.emplace_back();
            return lowestUnallocatedID++;
        }

        // take the id from   the front of the queue
        size_t id = freedIndices.back();
        // popit
        freedIndices.pop_back();
        // clear signature
        entitySignatures[id].clear();
        return id;
    }
    /**
     * @brief deletes an entity, removes all components
     *
     * @param id id of the entity
     */
    void delete_entity(const size_t id)
    {

        if (id > lowestUnallocatedID)
        {
            yorcvs::log("Invalid id for deletion : id doesn't exist", yorcvs::MSGSEVERITY::ERROR);
        }
        // if the id appears in the delted entities
        if (std::binary_search(freedIndices.begin(), freedIndices.end(), id))
        {
            yorcvs::log("Invalid id deletion : id is not a valid entity", yorcvs::MSGSEVERITY::ERROR);
        }

        entitySignatures[id].clear();
        // push the freed id into the queue

        freedIndices.push_back(id);
        // the vector is always sorted in order to check if id apears in it using binary search
        std::sort(freedIndices.begin(), freedIndices.end());
    }

    /**
     * @brief Set the signature of an entity
     *
     * @param id id of the entity
     * @param signature new signature
     *
     */
    void set_signature(const size_t id, const std::vector<bool> &signature)
    {

        if (id > lowestUnallocatedID)
        {
            yorcvs::log("Cannot set id signature  : id doesn't exist", yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        // if the id appears in the delted entities
        if (std::binary_search(freedIndices.begin(), freedIndices.end(), id))
        {
            yorcvs::log("Cannot set id signature : id is not a valid entity");
            return;
        }

        entitySignatures[id] = signature;
    }

    /**
     * @brief Returns the signature object
     *
     * @param id id of the entity
     * @return std::vector<bool>& signature
     */
    std::vector<bool> &get_signature(const size_t id)
    {
        if (id > lowestUnallocatedID)
        {
            yorcvs::log("Cannot fetch entity signature : id : " + std::to_string(id) + "doesn't exist", yorcvs::MSGSEVERITY::ERROR);
        }
        // if the id appears in the delted entities
        if (std::binary_search(freedIndices.begin(), freedIndices.end(), id))
        {
            yorcvs::log("Cannot fetch entity signature : id: " + std::to_string(id) + "is not a valid entity",
                        yorcvs::MSGSEVERITY::ERROR);
        }

        return entitySignatures[id];
    }

    // ids that had once an entity but now are
    std::vector<size_t> freedIndices;

    // stores the signature of an entity with the id as
    // vector<bool> spooky
    std::vector<std::vector<bool>> entitySignatures;

    // the lowest unallocated id that has not interacted with
    size_t lowestUnallocatedID = 0;
};
} // namespace yorcvs
