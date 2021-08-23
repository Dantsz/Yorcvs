/**
 * @file ecs.h
 * @author Dansz
 * @brief 
 * @version 0.1
 * @date 2021-08-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include "utilities.h"
#include <algorithm>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

/*The ECS is heaviley isnpired by AUSTIN MORLAN's implementation , but it has a bit more functionality and ease of use.
  It's a bit more 'loose'. It's minimal and doesn't have very specific function so it may be interchangeable with other
  ECS.
*/

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
// max number of components
constexpr size_t maxComponentID = std::numeric_limits<size_t>::max();

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
template <typename systemt> concept systemT = requires(systemt sys)
{
    // std::same_as<decltype(sys.entityList), std::shared_ptr<yorcvs::EntitySystemList>>;
    {sys.entityList->entitiesID[0]};
    {sys.entityList->entitiesID.size()};
};

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
            yorcvs::log("Cannot fetch entity signature : id : " + std::to_string(id) + "doesn't exist",
                        yorcvs::MSGSEVERITY::ERROR);
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

    // the lowest unallocated id that has not been interacted with
    size_t lowestUnallocatedID = 0;
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
            yorcvs::log("Trying to add an component to an entity which already has it", yorcvs::MSGSEVERITY::ERROR);
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
            yorcvs::log("Cannot get component : entity " + std::to_string(entityID) +
                            " doesn't own the specified type of component: " + std::string(typeid(T).name()),
                        yorcvs::MSGSEVERITY::ERROR);
        }

        return components[entitytocomponent[entityID]];
    }

    void remove_component(const size_t entityID)
    {

        // if the entity doesn't have this type of component throw exception
        if (entity_has_component.size() <= entityID || entity_has_component[entityID] != 1)
        {
            yorcvs::log("Cannot delete component: the entity " + std::to_string(entityID) +
                            " doesn't have this type of component : " + std::string(typeid(T).name()),
                        yorcvs::MSGSEVERITY::ERROR);
        }

        // get the index of the removed entity
        size_t removedIndex = entitytocomponent[entityID];

        // add the id to the list of unused ids
        freeIndex.push(removedIndex);

        // delete the entity from the registries

        entity_has_component[entityID] = 0;
    }

    // checks if entity has component
    [[nodiscard]] bool has_component(const size_t entityID) const
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
            yorcvs::log("Component " + std::string(componentid) + " already registered", yorcvs::MSGSEVERITY::ERROR);
        }
    }

    template <typename T> void add_component(const size_t entityID, T &component)
    {
        if (get_container<T>() == nullptr)
        {
            yorcvs::log(std::string("Component") + typeid(T).name() + " has not been registered yet !!!!",
                        yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        get_container<T>()->add_component(entityID, component);
    }
    template <typename T> void remove_component(const size_t entityID)
    {
        if (get_container<T>() == nullptr)
        {
            yorcvs::log(std::string("Component") + typeid(T).name() + " has not been registered yet !!!!",
                        yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        get_container<T>()->remove_component(entityID);
    }
    template <typename T> T &get_component(const size_t entityID)
    {
        if (get_container<T>() == nullptr)
        {
            yorcvs::log(std::string("Component") + typeid(T).name() + " has not been registered yet !!!!",
                        yorcvs::MSGSEVERITY::ERROR);
            exit(120);
        }
        return get_container<T>()->get_component(entityID);
    }

    // gets the id of a component in the manager
    template <typename T>[[nodiscard]] size_t get_component_ID()
    {
        const char *component_name = typeid(T).name();

        if (component_type.find(component_name) == component_type.end())
        {
            yorcvs::log("Cannot fetch component id" + std::string(component_name) + " : invalid component",
                        yorcvs::MSGSEVERITY::ERROR);
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
            yorcvs::log("Unable to register system: system is already registered.", yorcvs::MSGSEVERITY::ERROR);
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
            yorcvs::log("Unable to set the signature: system does not exist.", yorcvs::MSGSEVERITY::ERROR);
        }

        typetosignature[systemType] = signature;
    }

    // gets signature of a system
    template <systemT T>[[nodiscard]] std::vector<bool> get_system_signature()
    {

        const char *systemType = typeid(T).name();

        // if the system is not found  //throw
        if (typetosystem.find(systemType) == typetosystem.end())
        {
            yorcvs::log("Unable to fetch the signature: system does not exist.");
            return {};
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

/**
 * @brief Main part of the ecs, that createsd entities , registers components and systems
 *
 */
class ECS
{
  public:
    /**
     * @brief Initializez the ECS
     *
     */
    ECS()
    {
        yorcvs::log("Initializing ECS", yorcvs::MSGSEVERITY::INFO);
        componentmanager = std::make_unique<yorcvs::ComponentManager>();
        entitymanager = std::make_unique<yorcvs::EntityManager>();
        systemmanager = std::make_unique<yorcvs::SystemManager>();
    }
    ECS(ECS &&other) noexcept
    {
        this->componentmanager = std::move(other.componentmanager);
        this->entitymanager = std::move(other.entitymanager);
        this->systemmanager = std::move(other.systemmanager);
    }
    ECS(const ECS &other) =
        delete; // copy would be so expensive  the copy constructor will probably be called by accident
    ECS &operator=(const ECS &other) = delete;
    ECS &operator=(ECS &&other) = delete;
    ~ECS() noexcept
    {
        yorcvs::log("Destroying ECS...", yorcvs::MSGSEVERITY::INFO);
        if (componentmanager != nullptr)
        {
            componentmanager.reset();
        }
        if (entitymanager != nullptr)
        {
            entitymanager.reset();
        }
        if (systemmanager != nullptr)
        {
            systemmanager.reset();
        }
    }

    // assigns a new id for an entity
    /**
     * @brief Create a Entity ID object
     *
     * @return size_t the ID of the entity
     * NOTE: IDs created by this function are not managed by the ecs and should be freed using destroy_entity
     */
    [[nodiscard]] size_t create_entity_ID()
    {
        size_t ID = entitymanager->addEntity();
        systemmanager->on_entity_signature_change(ID, entitymanager->get_signature(ID));

        return ID;
    }
    /**
     * @brief Frees an entity ID and removes it's components
     *
     * @param id ID of the entity
     */
    void destroy_entity(const size_t id)
    {
        entitymanager->delete_entity(id);
        componentmanager->on_entity_destroyed(id);
        systemmanager->OnEntityDestroy(id);
    }
    /**
     * @brief Get the Entity Signature
     *
     * @param entityID Entity ID
     * @return std::vector<bool> List of all components, 1 if they have the component  or 0 otherwise
     */
    std::vector<bool> get_entity_signature(const size_t entityID)
    {
        return entitymanager->get_signature(entityID);
    }
    /**
     * @brief Registers a component making the ECS aware of it
     *
     * @tparam T The component object
     */
    template <typename T> void register_component()
    {
        componentmanager->register_component<T>();
    }

    /**
     * @brief Registers multiple components in the same function call
     *
     * @tparam T A component
     * @tparam secondT  Another component
     * @tparam OtherT  The rest
     */
    template <typename T, typename secondT, typename... OtherT> void register_component()
    {
        componentmanager->register_component<T>();
        register_component<secondT, OtherT...>();
    }

    /**
     * @brief Checks if a component is registered
     *
     * @tparam T component
     * @return true the component is registered
     * @return false it's not registered
     */
    template <typename T>[[nodiscard]] bool is_component_registered() const
    {
        const char *component_name = typeid(T).name();
        return (componentmanager->component_type.find(component_name) != componentmanager->component_type.end());
    }
    /**
     * @brief Adds a component to an entity
     *
     * @tparam T Component type
     * @param entityID ID of the entity
     * @param component Component
     */
    template <typename T> void add_component(const size_t entityID, T component)
    {
        // add the component
        componentmanager->add_component<T>(entityID, component);
        // modify the signature to match the new addition
        std::vector<bool> e_signature = entitymanager->get_signature(entityID);
        size_t component_type = componentmanager->get_component_ID<T>();
        // while the vector of signature doesn't have elements until the current component add 0 to the signature
        while (e_signature.size() <= component_type)
        {
            e_signature.push_back(false);
        }
        // add the new signature
        e_signature[component_type] = true;
        entitymanager->set_signature(entityID, e_signature);
        systemmanager->on_entity_signature_change(entityID, e_signature);
    }
    /**
     * @brief Adds multiple components to and entity
     *
     * @tparam T first component type
     * @tparam Other other components type
     * @param entityID The ID the entity
     * @param component First component
     * @param other Other components
     */
    template <typename T, typename... Other>
    void add_component(const size_t entityID, T component, const Other &... other)
    {
        // add the component
        componentmanager->add_component<T>(entityID, component);
        // modify the signature to match the new addition
        std::vector<bool> e_signature = entitymanager->get_signature(entityID);
        size_t component_type = componentmanager->get_component_ID<T>();
        // while the vector of signature doesn't have elements until the current component add 0 to the signature
        while (e_signature.size() <= component_type)
        {
            e_signature.push_back(false);
        }
        // add the new signature
        e_signature[component_type] = true;
        entitymanager->set_signature(entityID, e_signature);
        systemmanager->on_entity_signature_change(entityID, e_signature);
        add_component<Other...>(entityID, other...);
    }
    /**
     * @brief Removes component T from the entity
     *
     * @tparam T Component
     * @param entityID entity
     */
    template <typename T> void remove_component(const size_t entityID)
    {

        std::vector<bool> &e_signature = entitymanager->get_signature(entityID);
        size_t component_type = componentmanager->get_component_ID<T>();
        e_signature[component_type] = false;
        systemmanager->on_entity_signature_change(entityID, e_signature);
        componentmanager->remove_component<T>(entityID);
    }
    /**
     * @brief Removes two or more components from the entity
     *
     * @tparam T first component type
     * @tparam secondT second component type
     * @tparam Other other components type
     * @param entityID the ID of the entity
     */
    template <typename T, typename secondT, typename... Other> void remove_component(const size_t entityID)
    {

        std::vector<bool> &e_signature = entitymanager->get_signature(entityID);
        size_t component_type = componentmanager->get_component_ID<T>();
        e_signature[component_type] = false;
        systemmanager->on_entity_signature_change(entityID, e_signature);
        componentmanager->remove_component<T>(entityID);

        remove_component<secondT, Other...>(entityID);
    }
    /**
     * @brief Checks if an entity has a component
     *
     * @tparam T The component type
     * @param entityID Id of the entity
     * @return true it has the component
     * @return false it dowsn't
     */
    template <typename T> bool has_components(const size_t entityID)
    {
        return componentmanager->get_container<T>()->has_component(entityID);
    }
    /**
     * @brief Checks if an entity has all the components specified
     *
     * @tparam T fiest component type
     * @tparam secondT second component type
     * @tparam Other other components tpye
     * @param entityID id of the entity
     * @return true it has all of them
     * @return false it's missing one or more
     */
    template <typename T, typename secondT, typename... Other> bool has_components(const size_t entityID)
    {
        if (!componentmanager->get_container<T>()->has_component(entityID))
        {
            return false;
        }
        return has_components<secondT, Other...>(entityID);
    }
    /**
     * @brief Returns a reference the component of the entity
     *
     * @tparam T the type of component
     * @param entityID ID of the entity
     * @return T& the component
     */
    template <typename T> T &get_component(const size_t entityID)
    {
        return componentmanager->get_component<T>(entityID);
    }

    /**
     * @brief Registers a system so the ECS can track which entities should be used by the system
     *
     * @tparam T The system type
     * @param sys Reference to an instance of type system
     */
    template <typename T> void register_system(T &sys)
    {
        // if registering the system succeded
        if (systemmanager->register_system<T>(sys))
        {
            on_system_signature_change<T>();
        }
    }
    /**
     * @brief Checks if a system is registered
     *
     * @tparam T The system
     * @return true It is
     * @return false It is not
     */
    template <typename T>[[nodiscard]] bool is_system_registered() const
    {
        const char *systemType = typeid(T).name();

        return (systemmanager->typetosystem.find(systemType) != systemmanager->typetosystem.end());
    }

    /**
     * @brief Sets the signature of a system
     *
     * @tparam T System type
     * @param signature New system signature
     */
    template <typename T> void set_system_signature(std::vector<bool> &signature)
    {
        systemmanager->set_signature<T>(signature);
    }

    /**
     * @brief Returns the signature of a system
     *
     * @tparam T The system
     * @return std::vector<bool> Value of the systems signature
     */
    template <typename T>[[nodiscard]] std::vector<bool> get_system_signature() const
    {
        return systemmanager->get_system_signature<T>();
    }

    /**
     * @brief  add the components to the system <sys> as a criteria for iteration , if the entity doen't have the
     * components  specified , it will not iterate ovr them
     *
     *
     */

    template <typename sys> void add_criteria_for_iteration()
    {
        // update signature when all components have been added
        on_system_signature_change<sys>();
    }
    /**
     * @brief Adds more components at once
     *
     * @tparam sys the system
     * @tparam comp first components
     * @tparam comps other components
     */
    template <typename sys, typename comp, typename... comps> void add_criteria_for_iteration()
    {
        // get the current signature of sys
        std::vector<bool> signature = get_system_signature<sys>();
        // get the id of the component
        size_t componentID = get_component_ID<comp>();
        // modify the signature to fit the new component
        while (signature.size() <= componentID)
        {
            signature.push_back(false);
        }
        // mark the component as being a part of the system
        signature[componentID] = true;
        // set the new signature
        set_system_signature<sys>(signature);

        add_criteria_for_iteration<sys, comps...>();
    }

    template <typename sys> void set_criteria_for_iteration()
    {
        on_system_signature_change<sys>();
    }
    /**
     * @brief Sets the Criteria For Iteration,removes other criteria
     *
     * @tparam sys The system
     * @tparam comp First component
     * @tparam comps Other components
     */
    template <typename sys, typename comp, typename... comps> void set_criteria_for_iteration()
    {
        // get the current signature of sys
        std::vector<bool> signature = get_system_signature<sys>();

        // reset criteria
        for (auto && i : signature)
        {
            i = false;
        }

        // get the id of the component
        size_t componentID = get_component_ID<comp>();
        // modify the signature to fit the new component
        while (signature.size() <= componentID)
        {
            signature.push_back(false);
        }
        // mark the component as being a part of the system
        signature[componentID] = true;
        // set the new signature
        set_system_signature<sys>(signature);
    }

    /**
     * @brief Get the number of entities that have the specified component
     *
     * @tparam T Component
     * @return size_t The number of entities with that component
     * NOTE: This is might be costly
     */
    template <typename T>[[nodiscard]] size_t get_entities_with_component() const
    {
        // get component index
        size_t cIndex = componentmanager->get_component_ID<T>();
        size_t entities = 0;
        // unused entites have an emtpy signature so a false pozitive should happen
        for (const auto &i : entitymanager->entitySignatures)
        {
            if (i.size() > cIndex)
            {
                entities += static_cast<size_t>(i[cIndex]);
            }
        }
        return entities;
    }
    /**
     * @brief Adds all components of source to the destinations and copies their data, if the source has any components, they are destroyed before the copying
     * 
     * @param dstEntityID destination
     * @param srcEntityID source
     */
    void copy_component_to_from_entity(const size_t dstEntityID, const size_t srcEntityID)
    {
        componentmanager->copy_component_to_from_entity(dstEntityID, srcEntityID);
        std::vector<bool> newSignature = get_entity_signature(srcEntityID);
        systemmanager->on_entity_signature_change(dstEntityID, newSignature);
    }

    /**
     * @brief Get the number of active entities
     * 
     * @return size_t Entities that have been created but not destroyed
     */
    [[nodiscard]]size_t get_active_entities_number() const
    {
        return entitymanager->lowestUnallocatedID - entitymanager->freedIndices.size();
    }
  private:
    std::unique_ptr<yorcvs::ComponentManager> componentmanager;
    std::unique_ptr<yorcvs::EntityManager> entitymanager;
    std::unique_ptr<yorcvs::SystemManager> systemmanager;
    // gets maximum number of components of a certain type created
    // for example if 2 components have been created and 1 deleted
    // calling get_maximum_component_count will return 2
    // only used for debbuging
    template <typename comp, typename returnType = size_t> returnType get_maximum_component_count()
    {
        return componentmanager->get_container<comp>()->components.size();
    }
    template <typename T> size_t get_component_ID()
    {
        return componentmanager->get_component_ID<T>();
    }
    template <typename T> void on_system_signature_change()
    {
        const char *systemType = typeid(T).name();
        // add matching entities to it
        // couldn't find a better place to put it

        for (size_t entity = 0; entity < entitymanager->lowestUnallocatedID; entity++)
        {

            if (systemmanager->compare_entity_to_system(entitymanager->entitySignatures[entity],
                                                        systemmanager->get_system_signature<T>()))
            {
                // TODO : MAKE A METHOD TO SYSTEM , method needs to be virtual /Onewntitierase/insert
                insert_sorted(systemmanager->typetosystem.at(systemType)->entitiesID, entity);
            }
            else
            {
                // not looking good
                systemmanager->typetosystem.at(systemType)
                    ->entitiesID.erase(std::remove(systemmanager->typetosystem.at(systemType)->entitiesID.begin(),
                                                   systemmanager->typetosystem.at(systemType)->entitiesID.end(),
                                                   entity),
                                       systemmanager->typetosystem.at(systemType)->entitiesID.end());
            }
        }
    }
};

/**
 *
 * @brief RAII wrapper for ids
 *
 */
class Entity
{
  public:
    Entity(ECS *ecs)
    {
        parent = ecs;
        id = parent->create_entity_ID();
        yorcvs::log("Created entity with id: " + std::to_string(id), yorcvs::MSGSEVERITY::INFO);
    }

    Entity(const Entity &other)
    {
        parent = other.parent;
        id = parent->create_entity_ID();
        parent->copy_component_to_from_entity(id, other.id);
        yorcvs::log("The copy constructor for an Entity " + std::to_string(id) +
                        " has been called: this might be an unecesary expensive option",
                    yorcvs::MSGSEVERITY::WARNING);
    }
    Entity(Entity &&other) noexcept
    {
        parent = other.parent;
        id = other.id;
        other.parent = nullptr;
    }
    Entity &operator=(const Entity &other)
    {
        if (this == &other)
        {
            return *this;
        }
        parent = other.parent;
        parent->copy_component_to_from_entity(id, other.id);
        yorcvs::log("The copy assginment operator for Entity" + std::to_string(id) +
                        " has been called: this might be an unecesary expensive option",
                    yorcvs::MSGSEVERITY::WARNING);
        return *this;
    }

    Entity &operator=(Entity &&other) noexcept
    {
        parent = other.parent;
        id = other.id;
        other.parent = nullptr;
        return *this;
    }

    ~Entity() noexcept
    {
        if (parent != nullptr)
        {
            parent->destroy_entity(id);
            yorcvs::log("Destroyed entity with id: " + std::to_string(id), yorcvs::MSGSEVERITY::INFO);
        }
    }

    size_t id = 0;

  private:
    ECS *parent = nullptr; // non-owning pointer to parent
};

} // namespace yorcvs
