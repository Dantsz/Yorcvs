#pragma once


#include <memory>
#include <queue>
#include <vector>
#include "entity.h"
#include <unordered_map>

#include "../common/log.h"
// thanks to Austin Morlan for providing a ECS template

namespace yorcvs
{
// max number of components
constexpr size_t maxComponentID = std::numeric_limits<size_t>::max();
// virtual container for polymorphism
class VContainer
{
  public:
    virtual ~VContainer() = default;
    VContainer(VContainer& other) = default;
    VContainer(VContainer&& other) = default;
    VContainer& operator=(VContainer& other) = delete;
    VContainer& operator=(VContainer&& other) = delete;

    virtual void addComponent(size_t entityID) = 0;
    virtual void OnEntityDestroyed(size_t entityID) noexcept = 0;
    virtual void copyEntityComponent(size_t dstID, size_t srcID) = 0;

    // lookup the component of a entity
    // lookup the entity to component, it's now done through 2 vectors
    std::vector<bool> entityhascomponent{};
    std::vector<size_t> entitytocomponent{};
};

// contains..components
template <typename T> class ComponentContainer final : public VContainer
{
  public:


    // update a specific component
    void addComponent(const size_t entityID,const T& component) 
    {

        // if the entity does have this type of component throw exception
        if (entityhascomponent.size() > entityID && entityhascomponent[entityID] == 1)
        {
            yorcvs::log("Trying to add an component to an entity which already has it",yorcvs::ERROR);
        }
        // if there isn't any free space,create one
        if (freeIndex.empty())
        {
            components.push_back(component); // create component

            while (entitytocomponent.size() <= entityID)
            {
                entitytocomponent.push_back(0);
                entityhascomponent.push_back(0);
            }
            entitytocomponent[entityID] = components.size() - 1;
            entityhascomponent[entityID] = 1;
        }
        else // just take an unused component
        {

            components[freeIndex.front()] = component;

            while (entitytocomponent.size() < entityID)
            {
                entitytocomponent.push_back(0);
                entityhascomponent.push_back(0);
            }
            entitytocomponent[entityID] = freeIndex.front();
            entityhascomponent[entityID] = 1;
            freeIndex.pop();
        }
    }
    /**
    * @brief Adds a default intialize component to the entity
    * 
    * @param entityID 
    */
    void addComponent(const size_t entityID) override  
    {
        T newComponent{};
        addComponent(entityID,newComponent);
    }

    T &getComponent(const size_t entityID)
    {

        if (entityhascomponent.size() <= entityID || entityhascomponent[entityID] != 1)
        {
            yorcvs::log("Cannot get component : entity doesn't own the specified type of component: " + std::string(typeid(T).name()),yorcvs::ERROR);
        }

        return components[entitytocomponent[entityID]];
    }

    void removeComponent(const size_t entityID)
    {

        // if the entity doesn't have this type of component throw exception
        if (entityhascomponent.size() <= entityID || entityhascomponent[entityID] != 1)
        {
            yorcvs::log("Cannot delete component: the entity doesn't have this type of component : " + std::string(typeid(T).name()) ,yorcvs::ERROR); 
        }

        // get the index of the removed entity
        size_t removedIndex = entitytocomponent[entityID];

        // add the id to the list of unused ids
        freeIndex.push(removedIndex);

        // delete the entity from the registries

        entityhascomponent[entityID] = 0;
    }

    // checks if entity has component
    bool hasComponent(const size_t entityID)
    {
        if (entityhascomponent.size() < entityID)
            return false;
        if (entityhascomponent[entityID] == 0)
            return false;
        return true;
    }

    void OnEntityDestroyed(const size_t entityID)  noexcept override  // this function never throws exception because not finding the component of the entity is  intended
    {
        // there is already an exception to point if something happens
        removeComponent(entityID);
        // add more relevant code
    }


    void copyEntityComponent(const size_t dstID, const size_t srcID) override
    {
        components[entitytocomponent[dstID]] = components[entitytocomponent[srcID]];
    }

    // components vector
    std::vector<T> components{};

    // the next component's index to be used
    std::queue<size_t> freeIndex{};


};

// handles all components
class ComponentManager
{

  public:
    // register a component
    // Note: addComponent already registers a component that has not been use before
    // this function is not really useful at this point
    template <typename T> void registerComponent()
    {

        const char *componentid = typeid(T).name();
        // and it does what it looks it should do

        // check if the container type is registered
        if (component_type.find(componentid) == component_type.end())
        {
            // if the type of the container is not registered ,register it
            component_type.insert({componentid, topComponentType++});

            componentContainers.insert({componentid, std::make_shared<ComponentContainer<T>>()});
        }
        else
        {
            yorcvs::log("Component " + std::string(componentid) + " already registered",yorcvs::ERROR);
        }
    }

    template <typename T> void addComponent(const size_t entityID, T &component)
    {

        getContainer<T>()->addComponent(entityID, component);
    }
    template <typename T> void removeComponent(const size_t entityID)
    {
        getContainer<T>()->removeComponent(entityID);
    }
    template <typename T> T &getComponent(const size_t entityID)
    {
        return getContainer<T>()->getComponent(entityID);
    }

    // gets the id of a component in the manager
    template <typename T> size_t getComponentID()
    {
        const char *component_name = typeid(T).name();

        if (component_type.find(component_name) == component_type.end())
        {
            yorcvs::log("Cannot fetch component id" + std::string(component_name) + " : invalid component",yorcvs::ERROR);
        }
        return component_type[component_name];
    }

    // iterate through all components of the entity and delete them
    void OnEntityDestroyed(const size_t entityID)
    {
        for (auto const &i : componentContainers)
        {
            if(i.second->entityhascomponent.size() <= entityID)
            {
                continue;
            }
            if(i.second->entityhascomponent[entityID])
            {
                 i.second->OnEntityDestroyed(entityID);
            }
         
        }
    }

    // gets the container of the specified type
    template <typename T> std::shared_ptr<ComponentContainer<T>> getContainer()
    {
        //(apparently this is possible)
        const char *componentid = typeid(T).name();
        // and it does what it looks it should do

        // check if the container type is registered
        if (component_type.find(componentid) == component_type.end())
        {
            // if the type of the container is not registered ,register it
            component_type.insert({componentid, topComponentType++});

            componentContainers.insert({componentid, std::make_shared<ComponentContainer<T>>()});
        }
        return std::static_pointer_cast<ComponentContainer<T>>(componentContainers[componentid]);
    }
        
    /**
     * @brief Copies the component data from one entity to the other, if the destination doesn't have the component it adds it
     * 
     * @param dstEntityID Destination
     * @param srcEntityID Source
     */
    void entityCopyComponentToEntity(const size_t dstEntityID , const size_t srcEntityID)
    {
        //delete all components of destination
        OnEntityDestroyed(dstEntityID);
        for(const auto& container : componentContainers )
        {
            if(container.second->entityhascomponent.size() > srcEntityID && container.second->entityhascomponent[srcEntityID])
            {
                if(container.second->entityhascomponent.size() <= dstEntityID || !container.second->entityhascomponent[dstEntityID])
                {
                    //add a default constructed component to the entity
                    container.second->addComponent(dstEntityID);
                }
                container.second->copyEntityComponent(dstEntityID,srcEntityID);
            }
        }
    }

    // the largest id
    size_t topComponentType = 0;
    // type -> id
    std::unordered_map<const char *, size_t> component_type{};

    // contains
    std::unordered_map<const char *, std::shared_ptr<yorcvs::VContainer>> componentContainers{};
};

} // namespace yorcvs
