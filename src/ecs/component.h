#pragma once

#include "entity.h"
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>


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
    VContainer() = default;
    VContainer(VContainer &other) = default;
    VContainer(VContainer &&other) = default;
    VContainer &operator=(VContainer &other) = delete;
    VContainer &operator=(VContainer &&other) = delete;

    virtual void add_component(size_t entityID) = 0;
    virtual void on_entity_destroyed(size_t entityID) noexcept = 0;
    virtual void copy_entity_component(size_t dstID, size_t srcID) = 0;

    // lookup the component of a entity
    // lookup the entity to component, it's now done through 2 vectors
    std::vector<bool> entity_has_component{};
    std::vector<size_t> entitytocomponent{};
};

// contains..components
template <typename T> class ComponentContainer final : public VContainer
{
  public:
    // update a specific component
    void add_component(const size_t entityID, const T &component)
    {

        // if the entity does have this type of component throw exception
        if (entity_has_component.size() > entityID && entity_has_component[entityID] == 1)
        {
            yorcvs::log("Trying to add an component to an entity which already has it", yorcvs::ERROR);
        }
        // if there isn't any free space,create one
        if (freeIndex.empty())
        {
            components.push_back(component); // create component

            while (entitytocomponent.size() <= entityID)
            {
                entitytocomponent.push_back(0);
                entity_has_component.push_back(0);
            }
            entitytocomponent[entityID] = components.size() - 1;
            entity_has_component[entityID] = 1;
        }
        else // just take an unused component
        {

            components[freeIndex.front()] = component;

            while (entitytocomponent.size() < entityID)
            {
                entitytocomponent.push_back(0);
                entity_has_component.push_back(0);
            }
            entitytocomponent[entityID] = freeIndex.front();
            entity_has_component[entityID] = 1;
            freeIndex.pop();
        }
    }
    /**
     * @brief Adds a default intialize component to the entity
     *
     * @param entityID entity to add component to
     */
    void add_component(const size_t entityID) override
    {
        T newComponent{};
        add_component(entityID, newComponent);
    }

    T &get_component(const size_t entityID)
    {

        if (entity_has_component.size() <= entityID || entity_has_component[entityID] != 1)
        {
            yorcvs::log("Cannot get component : entity " + std::to_string(entityID)  + " doesn't own the specified type of component: " +
                            std::string(typeid(T).name()),
                        yorcvs::ERROR);
        }

        return components[entitytocomponent[entityID]];
    }

    void remove_component(const size_t entityID)
    {

        // if the entity doesn't have this type of component throw exception
        if (entity_has_component.size() <= entityID || entity_has_component[entityID] != 1)
        {
            yorcvs::log("Cannot delete component: the entity " + std::to_string(entityID) + " doesn't have this type of component : " +
                            std::string(typeid(T).name()),
                        yorcvs::ERROR);
        }

        // get the index of the removed entity
        size_t removedIndex = entitytocomponent[entityID];

        // add the id to the list of unused ids
        freeIndex.push(removedIndex);

        // delete the entity from the registries

        entity_has_component[entityID] = 0;
    }

    // checks if entity has component
    [[nodiscard]]bool has_component(const size_t entityID) const
    {
        if (entity_has_component.size() <= entityID)
        {
            return false;
        }
        if (entity_has_component[entityID] == 0)
        {
            return false;
        }
        return true;
    }

    void on_entity_destroyed(const size_t entityID) noexcept
        override // this function never throws exception because not finding the component of the entity is  intended
    {
        // there is already an exception to point if something happens
        remove_component(entityID);
        // add more relevant code
    }

    void copy_entity_component(const size_t dstID, const size_t srcID) override
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
    // Note: add_component already registers a component that has not been use before
    // this function is not really useful at this point
    template <typename T> void register_component()
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
            yorcvs::log("Component " + std::string(componentid) + " already registered", yorcvs::ERROR);
        }
    }

    template <typename T> void add_component(const size_t entityID, T &component)
    {
        if(get_container<T>() == nullptr)
        {
            yorcvs::log(std::string("Component") + typeid(T).name() + " has not been registered yet !!!!", yorcvs::ERROR);
            return;
        }
        get_container<T>()->add_component(entityID, component);
    }
    template <typename T> void remove_component(const size_t entityID)
    {   
        if(get_container<T>() == nullptr)
        {
            yorcvs::log(std::string("Component") + typeid(T).name() + " has not been registered yet !!!!", yorcvs::ERROR);
            return;
        }
        get_container<T>()->remove_component(entityID);
    }
    template <typename T> T &get_component(const size_t entityID)
    {
        if(get_container<T>() == nullptr)
        {
           yorcvs::log(std::string("Component") + typeid(T).name() + " has not been registered yet !!!!", yorcvs::ERROR);
           exit(120);
        }
        return get_container<T>()->get_component(entityID);
    }

    // gets the id of a component in the manager
    template <typename T> size_t get_component_ID()
    {
        const char *component_name = typeid(T).name();

        if (component_type.find(component_name) == component_type.end())
        {
            yorcvs::log("Cannot fetch component id" + std::string(component_name) + " : invalid component",
                        yorcvs::ERROR);
        }
        return component_type[component_name];
    }

    // iterate through all components of the entity and delete them
    void on_entity_destroyed(const size_t entityID)
    {
        for (auto const &i : componentContainers)
        {
            if (i.second->entity_has_component.size() <= entityID)
            {
                continue;
            }
            if (i.second->entity_has_component[entityID])
            {
                i.second->on_entity_destroyed(entityID);
            }
        }
    }

    // gets the container of the specified type
    template <typename T> std::shared_ptr<ComponentContainer<T>> get_container()
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
     * @brief Copies the component data from one entity to the other, if the destination doesn't have the component it
     * adds it
     *
     * @param dstEntityID Destination
     * @param srcEntityID Source
     */
    void copy_component_to_from_entity(const size_t dstEntityID, const size_t srcEntityID)
    {
        // delete all components of destination
        on_entity_destroyed(dstEntityID);
        for (const auto &container : componentContainers)
        {
            if (container.second->entity_has_component.size() > srcEntityID &&
                container.second->entity_has_component[srcEntityID])
            {
                if (container.second->entity_has_component.size() <= dstEntityID ||
                    !container.second->entity_has_component[dstEntityID])
                {
                    // add a default constructed component to the entity
                    container.second->add_component(dstEntityID);
                }
                container.second->copy_entity_component(dstEntityID, srcEntityID);
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
