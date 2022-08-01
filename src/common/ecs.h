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
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
/*The ECS is heaviley inspired by AUSTIN MORLAN's implementation , but it has a bit more functionality and ease of use.
  It's a bit more 'loose'. It's minimal and doesn't have very specific function so it may be interchangeable with other
  ECS.
*/

/**
 * @brief Inserts the element at its ordered position in the sorted array
 *
 * @param vec
 * @param item
 * @return std::vector<T>::iterator
 */
template <typename T>
typename std::vector<T>::iterator insert_sorted(std::vector<T>& vec, T const& item)
{
    if (std::binary_search(vec.begin(), vec.end(), item)) {
        return vec.end();
    }
    auto upper_bound = std::upper_bound(vec.begin(), vec.end(), item);
    return vec.insert(upper_bound, item);
}
namespace yorcvs {

class ECS; // forward declaration

/**
 * @brief Contains a list of entities matching parents signature
 *
 */
using EntitySystemList = std::vector<size_t>;

// concept for a valid system
// must have a vector of size_t
template <typename systemt>
concept systemT = requires(systemt sys)
{
    // std::same_as<decltype(sys.entityList), std::shared_ptr<yorcvs::EntitySystemList>>;
    { (*sys.entityList)[0] };
    { sys.entityList->size() };
};
/**
 * @brief Manages entity ids
 *
 */
class EntityManager {
public:
    EntityManager() = default;
    ~EntityManager() = default;
    EntityManager(const EntityManager& other) = default;
    EntityManager(EntityManager&& other) noexcept
        : freedIndices(std::move(other.freedIndices))
        , entitySignatures(std::move(other.entitySignatures))
    {
    }
    EntityManager& operator=(const EntityManager& other)
    {
        if (this == &other) {
            return *this;
        }
        this->entitySignatures = other.entitySignatures;
        this->freedIndices = other.freedIndices;
        return *this;
    }
    EntityManager& operator=(EntityManager&& other) noexcept
    {
        this->entitySignatures = std::move(other.entitySignatures);
        this->freedIndices = std::move(other.freedIndices);
        return *this;
    }

    /**
     * @brief  takes a freed or makes a new one if there's no id
     *
     * @return size_t ID of the new entity
     */
    size_t addEntity()
    { // if there isn't any in the the queue,create a new one and a new entry in the signature list
        if (freedIndices.empty()) {
            entitySignatures.emplace_back();
            return entitySignatures.size() - 1;
        }
        // take the id from   the front of the queue
        const size_t id = freedIndices.back();
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
    void delete_entity(const size_t id) noexcept
    {
        if (id > entitySignatures.size()) {
            yorcvs::log("Invalid id for deletion : id doesn't exist", yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        // if the id appears in the delted entities
        if (std::binary_search(freedIndices.begin(), freedIndices.end(), id)) {
            yorcvs::log("Invalid id deletion : id " + std::to_string(id) + " is not a valid entity",
                yorcvs::MSGSEVERITY::ERROR);
            return;
        }

        entitySignatures[id].clear();
        // push the freed id into the queue

        freedIndices.push_back(id);
        // the vector is always sorted in order to check if id apears in it using binary search
        std::sort(freedIndices.begin(), freedIndices.end());
    }
    /**
     * @brief Returns whether the entity is valid or not(THE ENTITY COULD'VE BEEN OVERWRITTEN)
     *
     * @param id entity
     * @return true the entity is valid
     * @return false the entity has been destroyed or it has never existed
     */
    bool is_valid_entity(const size_t id)
    {
        if (std::find(freedIndices.begin(), freedIndices.end(), id) != freedIndices.end()) // the entity was freed
        {
            return false;
        }
        if (id > entitySignatures.size()) // the entity was never created
        {
            return false;
        }
        return true;
    }
    /**
     * @brief Set the signature of an entity
     *
     * @param id id of the entity
     * @param signature new signature
     *
     */
    void set_signature(const size_t id, const std::vector<bool>& signature)
    {
        if (id > entitySignatures.size()) {
            yorcvs::log("Cannot set id signature  : id doesn't exist", yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        // if the id appears in the delted entities
        if (std::binary_search(freedIndices.begin(), freedIndices.end(), id)) {
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
    std::vector<bool>& get_signature(const size_t id)
    {
        if (id > entitySignatures.size()) {
            yorcvs::log("Cannot fetch entity signature : id : " + std::to_string(id) + "doesn't exist",
                yorcvs::MSGSEVERITY::ERROR);
        }
        // if the id appears in the delted entities
        if (std::binary_search(freedIndices.begin(), freedIndices.end(), id)) {
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
};

/**
 * @brief Base class for all components container
 *
 */
class VContainer {
public:
    virtual ~VContainer() = default;
    VContainer() = default;
    VContainer(VContainer& other) = default;
    VContainer(VContainer&& other) = default;
    VContainer& operator=(VContainer& other) = delete;
    VContainer& operator=(VContainer&& other) = delete;

    virtual void add_component(size_t entityID) = 0;
    virtual void on_entity_destroyed(size_t entityID) noexcept = 0;
    virtual void copy_entity_component(size_t dstID, size_t srcID) = 0;
    [[nodiscard]] virtual size_t get_allocated_components() const = 0;
    // lookup the component of a entity
    // lookup the entity to component, it's now done through 2 vectors
    std::vector<bool> entity_has_component {};
    std::vector<size_t> entity_to_component {};
};

/**
 * @brief Stores a type of component
 *
 * @tparam T Type of compoent stored
 */
template <typename T>
class ComponentContainer final : public VContainer {
public:
    // update a specific component
    void add_component(const size_t entityID, const T& component)
    {
        // if the entity does have this type of component throw exception
        if (entity_has_component.size() > entityID && entity_has_component[entityID] == 1) {
            yorcvs::log("Trying to add an component to an entity which already has it", yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        // if there isn't any free space,create one
        if (freeIndex.empty()) {
            components.push_back(component); // create component

            while (entity_to_component.size() <= entityID) {
                entity_to_component.push_back(0);
                entity_has_component.push_back(0);
            }
            entity_to_component[entityID] = components.size() - 1;
            entity_has_component[entityID] = 1;
        } else // just take an unused component
        {
            components[freeIndex.front()] = component;

            while (entity_to_component.size() <= entityID) {
                entity_to_component.push_back(0);
                entity_has_component.push_back(0);
            }
            entity_to_component[entityID] = freeIndex.front();
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
        T newComponent {};
        add_component(entityID, newComponent);
    }

    T& get_component(const size_t entityID)
    {
        if (entity_has_component.size() <= entityID || entity_has_component[entityID] != 1) {
            yorcvs::log("Cannot get component : entity " + std::to_string(entityID) + " doesn't own the specified type of component: " + std::string(typeid(T).name()),
                yorcvs::MSGSEVERITY::ERROR);
        }
        return components[entity_to_component[entityID]];
    }
    /**
     * @brief Removed component from entity
     *
     * @param entityID
     */
    void remove_component(const size_t entityID)
    {
        // if the entity doesn't have this type of component throw exception
        if (entity_has_component.size() <= entityID || entity_has_component[entityID] != 1) {
            yorcvs::log("Cannot delete component: the entity " + std::to_string(entityID) + " doesn't have this type of component : " + std::string(typeid(T).name()),
                yorcvs::MSGSEVERITY::ERROR);
        }

        // get the index of the removed entity
        const size_t removedIndex = entity_to_component[entityID];

        // add the id to the list of unused ids
        freeIndex.push(removedIndex);

        // delete the entity from the registries

        entity_has_component[entityID] = 0;
    }

    /**
     * @brief   checks if entity has component
     *
     * @param entityID
     * @return true it has the component
     * @return false it doesn't
     */
    [[nodiscard]] bool has_component(const size_t entityID) const
    {
        if (entity_has_component.size() <= entityID) {
            return false;
        }
        if (entity_has_component[entityID] == 0) {
            return false;
        }
        return true;
    }
    /**
     * @brief Called when an entity is destroyed
     *
     * @param entityID
     */
    void on_entity_destroyed(const size_t entityID) noexcept
        override // this function never throws exception because not finding the component of the entity is  intended
    {
        // there is already an exception to point if something happens
        remove_component(entityID);
        // add more relevant code
    }

    void copy_entity_component(const size_t dstID, const size_t srcID) override
    {
        components[entity_to_component[dstID]] = components[entity_to_component[srcID]];
    }

    [[nodiscard]] size_t get_allocated_components() const override
    {
        return components.size() - freeIndex.size();
    }

private:
    // components vector
    std::vector<T> components {};

    // the next component's index to be used
    std::queue<size_t> freeIndex {};
};

/**
 * @brief Manages containers
 *
 */
class ComponentManager {
public:
    ComponentManager() = default;
    ComponentManager(const ComponentManager& other) = default;
    ComponentManager(ComponentManager&& other) noexcept
        : nrComponents(other.nrComponents)
        , component_type(std::move(other.component_type))
        , componentContainers(std::move(other.componentContainers))
    {
    }
    ~ComponentManager() = default;

    ComponentManager& operator=(const ComponentManager& other)
    {
        if (this == &other) {
            return *this;
        }
        this->nrComponents = other.nrComponents;
        this->componentContainers = other.componentContainers;
        this->component_type = other.component_type;
        return *this;
    }

    ComponentManager& operator=(ComponentManager&& other) noexcept
    {
        nrComponents = other.nrComponents;
        component_type = std::move(other.component_type);
        componentContainers = std::move(other.componentContainers);
        return *this;
    }

    /**
     * @brief  register a component
     *
     * @tparam T
     */
    template <typename T>
    void register_component()
    {
        const char* componentid = typeid(T).name();
        // and it does what it looks it should do
        // check if the container type is registered
        if (component_type.find(componentid) == component_type.end()) { // if the type of the container is not registered ,register it
            component_type.insert({ componentid, nrComponents++ });

            componentContainers.insert({ componentid, std::make_shared<ComponentContainer<T>>() });
        } else {
            yorcvs::log("Component " + std::string(componentid) + "  already registered", yorcvs::MSGSEVERITY::ERROR);
        }
    }
    /**
     * @brief Adds a registered type of component to the entity
     *
     * @tparam T type of component
     * @param entityID entity
     * @param component
     */
    template <typename T>
    void add_component(const size_t entityID, T& component)
    {
        if (get_container<T>() == nullptr) {
            yorcvs::log(std::string("Component ") + typeid(T).name() + " has not been registered yet !!!!",
                yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        get_container<T>()->add_component(entityID, component);
    }
    template <typename T>
    void remove_component(const size_t entityID)
    {
        if (get_container<T>() == nullptr) {
            yorcvs::log(std::string("Component ") + typeid(T).name() + " has not been registered yet !!!!",
                yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        get_container<T>()->remove_component(entityID);
    }
    template <typename T>
    T& get_component(const size_t entityID)
    {
        if (get_container<T>() == nullptr) {
            yorcvs::log(std::string("Component ") + typeid(T).name() + " has not been registered yet !!!!",
                yorcvs::MSGSEVERITY::ERROR);
        }
        return get_container<T>()->get_component(entityID);
    }

    /**
     * @brief gets the id of a component in the manager
     *
     * @tparam T
     * @return size_t
     */
    template <typename T>
    [[nodiscard]] std::optional<size_t> get_component_ID()
    {
        const char* component_name = typeid(T).name();
        if (component_type.find(component_name) == component_type.end()) {
            yorcvs::log("Cannot fetch component id" + std::string(component_name) + " : invalid component",
                yorcvs::MSGSEVERITY::ERROR);
            return {};
        }
        return component_type[component_name];
    }

    /**
     * @brief iterate through all components of the entity and delete them
     *
     * @param entityID
     */
    void on_entity_destroyed(const size_t entityID) noexcept
    {
        for (auto const& i : componentContainers) {
            if (i.second->entity_has_component.size() <= entityID) {
                continue;
            }
            if (i.second->entity_has_component[entityID]) {
                i.second->on_entity_destroyed(entityID);
            }
        }
    }

    /**
     * @brief gets the container of the specified type
     *
     * @tparam T
     * @return std::shared_ptr<ComponentContainer<T>>
     */
    template <typename T>
    std::shared_ptr<ComponentContainer<T>> get_container()
    {
        //(apparently this is possible)
        const char* componentid = typeid(T).name();
        // and it does what it looks it should do

        // check if the container type is registered
        if (component_type.find(componentid) == component_type.end()) {
            // if the type of the container is not registered ,register it
            component_type.insert({ componentid, nrComponents++ });

            componentContainers.insert({ componentid, std::make_shared<ComponentContainer<T>>() });
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
    void copy_component_data_to_from_entity(const size_t dstEntityID, const size_t srcEntityID)
    {
        // delete all components of destination
        on_entity_destroyed(dstEntityID);
        for (const auto& container : componentContainers) {
            if (container.second->entity_has_component.size() > srcEntityID && container.second->entity_has_component[srcEntityID]) {
                if (container.second->entity_has_component.size() <= dstEntityID || !container.second->entity_has_component[dstEntityID]) {
                    // add a default constructed component to the entity
                    container.second->add_component(dstEntityID);
                }
                container.second->copy_entity_component(dstEntityID, srcEntityID);
            }
        }
    }

    // number of components
    size_t nrComponents = 0;
    // type -> id
    std::unordered_map<const char*, size_t> component_type {};

    // contains
    std::unordered_map<const char*, std::shared_ptr<yorcvs::VContainer>> componentContainers {};
};

class SystemManager {
public:
    SystemManager() = default;
    SystemManager(const SystemManager& other) = default;
    SystemManager(SystemManager&& other) noexcept = default;

    SystemManager& operator=(const SystemManager& other) = default;
    SystemManager& operator=(SystemManager&& other) = default;

    ~SystemManager() = default;
    /**
     * @brief creates a system of type T and puts it in the map
     *
     * @tparam T type of the system(one system per time)
     * @param system a valid reference to system
     * @return true the system was registered
     * @return false the system was already registered
     */
    template <systemT T>
    bool register_system(T& system)
    {
        const char* systemType = typeid(T).name();
        // if the system is already present
        if (type_to_system.find(systemType) != type_to_system.end()) {
            yorcvs::log("Unable to register system: system is already registered.", yorcvs::MSGSEVERITY::ERROR);
            return false;
        }
        std::shared_ptr<EntitySystemList> systemEVec = std::make_shared<EntitySystemList>();
        type_to_system.insert({ systemType, systemEVec });
        set_signature<T>(std::vector<bool> {});
        system.entityList = systemEVec;
        return true;
    }
    /**
     * @brief unregister_component
     * @return true if an entry was removed from the list
     */
    template <typename T>
    bool unregister_system()
    {
        const char* system_name = typeid(T).name();
        if (type_to_system.find(system_name) != type_to_system.end()) {
            // the system exists
            type_to_system.at(system_name)->clear(); // clear the entities the system holds
            type_to_system.erase(system_name);
            type_to_signature.erase(system_name);
            return true;

        } else {
            // the system is not in the manager
            yorcvs::log(std::string("System ") + system_name + " not registered! ", yorcvs::MSGSEVERITY::ERROR);
            return false;
        }
    }

    /**
     * @brief Sets the signature of the system with type T
     *
     * @tparam T
     * @param signature
     */
    template <systemT T>
    void set_signature(const std::vector<bool>& signature)
    {
        const char* systemType = typeid(T).name();
        // if the system is not found  //throw
        if (type_to_system.find(systemType) == type_to_system.end()) {
            yorcvs::log("Unable to set the signature: system does not exist.", yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        type_to_signature[systemType] = signature;
    }

    /**
     * @brief Returns the list of entities the system has acces to
     *
     * @tparam system - the system
     * @return std::shared_ptr<yorcvs::EntitySystemList> - pointer too the list of entities
     */
    template <systemT system>
    std::shared_ptr<yorcvs::EntitySystemList> get_system_entity_list()
    {
        const char* systemType = typeid(system).name();
        if (type_to_system.find(systemType) == type_to_system.end()) {
            yorcvs::log("Unable to get list of system " + std::string(systemType) + " system does not exist",
                yorcvs::MSGSEVERITY::ERROR);
            return nullptr;
        }
        return type_to_system[systemType];
    }

    /**
     * @brief Gets signature of a system
     *
     * @tparam T
     * @return std::vector<bool>
     */
    template <systemT T>
    [[nodiscard]] std::vector<bool> get_system_signature()
    {
        const char* systemType = typeid(T).name();
        // if the system is not found  //throw
        if (type_to_system.find(systemType) == type_to_system.end()) {
            yorcvs::log("Unable to fetch the signature: system does not exist.");
            return {};
        }

        return type_to_signature[systemType];
    }

    /**
     * @brief Erase entity from all systems
     *
     * @param entityID
     */
    void on_entity_destroy(const size_t entityID) noexcept
    {
        for (auto const& it : type_to_system) {
            it.second->erase(
                std::remove(it.second->begin(), it.second->end(), entityID),
                it.second->end());
        }
    }

    /**
     * @brief Notify each system that an entity's signature changed
     *
     * @param entityID
     * @param signature
     */
    void on_entity_signature_change(const size_t entityID, const std::vector<bool>& signature)
    {
        for (auto const& it : type_to_system) {
            auto const& type = it.first;
            auto const& system = it.second;
            auto const& systemSignature = type_to_signature[type];
            if (compare_entity_to_system(signature, systemSignature)) {
                // TODO : MAKE A METHOD TO SYSTEM , method needs to be virtual /Onewntitierase/insert
                insert_sorted(*system, entityID);
            } else {
                system->erase(std::remove(system->begin(), system->end(), entityID),
                    system->end());
            }
        }
    }

    /**
     * @brief compare the signature
     *
     * @param signature1 first signature
     * @param signature2 second signature
     * @return true they are the same
     * @return false they differ
     */
    static bool compare_signatures(const std::vector<bool>& signature1, const std::vector<bool>& signature2)
    {
        if (signature1.size() != signature2.size()) {
            return false;
        }
        for (size_t i = 0; i < signature1.size(); i++) {
            // if a signature  is missing somewhere
            if (signature1[i] != signature2[i]) {
                return false;
            }
        }
        return true;
    }
    /**
     * @brief compares the signature of an entity to a specific system
     *
     * @param entity_s The entity
     * @param system_s The system
     * @return true the entity has all the systems components
     * @return false if the system has a component but the entity does not or if the system signature doesn't match the
     * entity
     */
    static bool compare_entity_to_system(const std::vector<bool>& entity_s, const std::vector<bool>& system_s)
    {
        /// if the system signature doesn't match the entity
        if (system_s.size() > entity_s.size()) {
            return false;
        }
        // if a signature has a system but the entity does not
        // return false
        for (size_t i = 0; i < system_s.size(); i++) {
            if (system_s[i] && !entity_s[i]) {
                return false;
            }
        }
        return true;
    }

    // get the signature of a system based on type
    std::unordered_map<const char*, std::vector<bool>> type_to_signature {};
    // get the system based on type
    std::unordered_map<const char*, std::shared_ptr<EntitySystemList>> type_to_system {};
};

/**
 * @brief Main part of the ecs, that createsd entities , registers components and systems
 *
 */
class ECS {
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
    ECS(ECS&& other)
    noexcept
        : componentmanager(std::move(other.componentmanager))
        , entitymanager(std::move(other.entitymanager))
        , systemmanager(std::move(other.systemmanager))
    {
    }
    ECS(const ECS& other) = delete; // copy would be so expensive  the copy constructor will probably be called by accident
    ECS& operator=(const ECS& other)
    {
        if (this == &other) {
            return *this;
        }
        yorcvs::log("Initializing ECS with copy assignment operator", yorcvs::MSGSEVERITY::INFO);
        componentmanager = std::make_unique<yorcvs::ComponentManager>(*other.componentmanager);
        entitymanager = std::make_unique<yorcvs::EntityManager>(*other.entitymanager);
        systemmanager = std::make_unique<yorcvs::SystemManager>(*other.systemmanager);
        return *this;
    }
    ECS& operator=(ECS&& other) = delete;
    ~ECS() noexcept
    {
        yorcvs::log("Destroying ECS...", yorcvs::MSGSEVERITY::INFO);
        if (componentmanager != nullptr) {
            componentmanager.reset();
        }
        if (entitymanager != nullptr) {
            entitymanager.reset();
        }
        if (systemmanager != nullptr) {
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
    void destroy_entity(const size_t id) noexcept
    {
        entitymanager->delete_entity(id);
        componentmanager->on_entity_destroyed(id);
        systemmanager->on_entity_destroy(id);
    }
    /**
     * @brief Checks whether the entity is valid(components can be added to it)
     *
     * @param id id of the entity
     * @return true the entity is valid
     * @return false the entity has been freed or never existed
     */
    bool is_valid_entity(const size_t id)
    {
        return entitymanager->is_valid_entity(id);
    }
    /**
     * @brief Get the Entity Signature
     *
     * @param entityID Entity ID
     * @return std::vector<bool> List of all components, 1 if they have the component  or 0 otherwise, if the entity is not valid the empty is vector
     */
    std::vector<bool> get_entity_signature(const size_t entityID)
    {
        if (!is_valid_entity(entityID)) {
            yorcvs::log("Cannot retrieve the signature of the entity " + std::to_string(entityID) + " as the entity is not valid", yorcvs::MSGSEVERITY::ERROR);
            return {};
        }
        return entitymanager->get_signature(entityID);
    }
    /**
     * @brief Get the component ID
     *
     * @tparam T
     * @return size_t
     */
    template <typename T>
    std::optional<size_t> get_component_ID()
    {
        return componentmanager->get_component_ID<T>();
    }
    /**
     * @brief Registers a component making the ECS aware of it
     *
     * @tparam T The component object
     */
    template <typename T>
    void register_component()
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
    template <typename T, typename secondT, typename... OtherT>
    void register_component()
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
    template <typename T>
    [[nodiscard]] bool is_component_registered() const
    {
        const char* component_name = typeid(T).name();
        return (componentmanager->component_type.find(component_name) != componentmanager->component_type.end());
    }
    /**
     * @brief Adds a component to an entity
     *
     * @tparam T Component type
     * @param entityID ID of the entity
     * @param component Component
     */
    template <typename T>
    void add_component(const size_t entityID, T component)
    {
        // add the component
        componentmanager->add_component<T>(entityID, component);
        // modify the signature to match the new addition
        std::vector<bool> e_signature = entitymanager->get_signature(entityID);
        const std::optional<size_t> component_type = componentmanager->get_component_ID<T>();
        if (!component_type.has_value()) {
            return;
        }
        // while the vector of signature doesn't have elements until the current component add 0 to the signature
        while (e_signature.size() <= component_type) {
            e_signature.push_back(false);
        }
        // add the new signature
        e_signature[component_type.value()] = true;
        entitymanager->set_signature(entityID, e_signature);
        systemmanager->on_entity_signature_change(entityID, e_signature);
    }
    /**
     * @brief Adds a default constructed component to the entity
     *
     * @tparam T
     * @param entityID
     * @param component
     */
    template <typename T>
    void add_default_component(const size_t entityID)
    {
        add_component<T>(entityID, {});
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
    void add_component(const size_t entityID, T component, const Other&... other)
    {
        // add the component
        componentmanager->add_component<T>(entityID, component);
        // modify the signature to match the new addition
        std::vector<bool> e_signature = entitymanager->get_signature(entityID);
        const size_t component_type = componentmanager->get_component_ID<T>();
        // while the vector of signature doesn't have elements until the current component add 0 to the signature
        while (e_signature.size() <= component_type) {
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
    template <typename T>
    void remove_component(const size_t entityID)
    {
        std::vector<bool>& e_signature = entitymanager->get_signature(entityID);
        const std::optional<size_t> component_type = componentmanager->get_component_ID<T>();
        if (!component_type.has_value()) {
            return;
        }
        e_signature[component_type.value()] = false;
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
    template <typename T, typename secondT, typename... Other>
    void remove_component(const size_t entityID)
    {
        std::vector<bool>& e_signature = entitymanager->get_signature(entityID);
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
    template <typename T>
    bool has_components(const size_t entityID)
    {
        if (componentmanager->get_container<T>() == nullptr) {
            yorcvs::log("component not registered", yorcvs::MSGSEVERITY::ERROR);
            return false;
        }
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
    template <typename T, typename secondT, typename... Other>
    bool has_components(const size_t entityID)
    {
        if (!componentmanager->get_container<T>()->has_component(entityID)) {
            return false;
        }
        return has_components<secondT, Other...>(entityID);
    }
    /**
     * @brief Returns a reference the component of the entity
     *
     * @tparam T the type of component
     * @param entityID ID of the entity
     * @return T& the component, IF the entity doesn't have the component or is invalid, the programs aborts
     */
    template <typename T>
    T& get_component(const size_t entityID)
    {
        if (!is_valid_entity(entityID) || !has_components<T>(entityID)) {
            yorcvs::log("ENTITY DOESN'T HAVE COMPONENT OR IS INVALID", yorcvs::MSGSEVERITY::ERROR);
            std::abort();
        }
        return componentmanager->get_component<T>(entityID);
    }
    /**
     * @brief Returns a reference the component of the entity
     *
     * @tparam T the type of component
     * @param entityID ID of the entity
     * @return T& the component
     */
    template <typename T>
    std::optional<std::reference_wrapper<T>> get_component_checked(const size_t entityID)
    {
        if (!is_valid_entity(entityID) || !has_components<T>(entityID)) {
            return {};
        }
        return componentmanager->get_component<T>(entityID);
    }

    /**
     * @brief Registers a system so the ECS can track which entities should be used by the system
     *
     * @tparam T The system type
     * @param sys Reference to an instance of type system
     */
    template <typename T>
    void register_system(T& sys)
    {
        // if registering the system succeded
        if (systemmanager->register_system<T>(sys)) {
            on_system_signature_change<T>();
        }
    }
    /**
     * @brief Deletes the system data present in the ECS
     * @return ture if the data was deleted, false if it didn't found what to delete
     */
    template <typename T>
    bool unregister_system()
    {
        return systemmanager->unregister_system<T>();
    }
    /**
     * @brief Checks if a system is registered
     *
     * @tparam T The system
     * @return true It is
     * @return false It is not
     */
    template <typename T>
    [[nodiscard]] bool is_system_registered() const
    {
        const char* systemType = typeid(T).name();

        return (systemmanager->type_to_system.find(systemType) != systemmanager->type_to_system.end());
    }
    /**
     * @brief Returns the list of entities the system has acces to
     *
     * @tparam system - the system
     * @return std::shared_ptr<yorcvs::EntitySystemList> - pointer too the list of entities
     */
    template <systemT system>
    std::shared_ptr<yorcvs::EntitySystemList> get_system_entity_list()
    {
        return systemmanager->get_system_entity_list<system>();
    }
    /**
     * @brief Sets the signature of a system
     *
     * @tparam T System type
     * @param signature New system signature
     */
    template <systemT T>
    void set_system_signature(std::vector<bool>& signature)
    {
        systemmanager->set_signature<T>(signature);
    }

    /**
     * @brief Returns the signature of a system
     *
     * @tparam T The system
     * @return std::vector<bool> Value of the systems signature
     */
    template <systemT T>
    [[nodiscard]] std::vector<bool> get_system_signature() const
    {
        return systemmanager->get_system_signature<T>();
    }

    /**
     * @brief  add the components to the system <sys> as a criteria for iteration , if the entity doen't have the
     * components  specified , it will not iterate ovr them
     *
     *
     */

    template <typename sys>
    void add_criteria_for_iteration()
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
    template <typename sys, typename comp, typename... comps>
    void add_criteria_for_iteration()
    {
        // get the current signature of sys
        std::vector<bool> signature = get_system_signature<sys>();
        // get the id of the component
        const std::optional<size_t> componentID = get_component_ID<comp>();
        if (!componentID.has_value()) {
            yorcvs::log(std::string("Could not add component ") + typeid(comp).name() + " to ieration for " + std::string(typeid(sys).name()), yorcvs::MSGSEVERITY::ERROR);
            return;
        }
        // modify the signature to fit the new component
        while (signature.size() <= componentID) {
            signature.push_back(false);
        }
        // mark the component as being a part of the system
        signature[componentID.value()] = true;
        // set the new signature
        set_system_signature<sys>(signature);

        add_criteria_for_iteration<sys, comps...>();
    }

    template <typename sys>
    void set_criteria_for_iteration()
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
    template <typename sys, typename comp, typename... comps>
    void set_criteria_for_iteration()
    {
        // get the current signature of sys
        std::vector<bool> signature = get_system_signature<sys>();

        // reset criteria
        for (auto&& i : signature) {
            i = false;
        }

        // get the id of the component
        size_t componentID = get_component_ID<comp>();
        // modify the signature to fit the new component
        while (signature.size() <= componentID) {
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
    template <typename T>
    [[nodiscard]] size_t get_entities_with_component() const
    {
        // get component index
        const std::optional<size_t> cIndex = componentmanager->get_component_ID<T>();
        if (!cIndex.has_value()) {
            yorcvs::log(std::string("Component ") + typeid(T).name() + " is not valid");
            return 0;
        }
        size_t entities = 0;
        // unused entites have an emtpy signature so a false pozitive should happen
        for (const auto& i : entitymanager->entitySignatures) {
            if (i.size() > cIndex) {
                entities += static_cast<size_t>(i[cIndex.value()]);
            }
        }
        return entities;
    }
    /**
     * @brief Adds all components of source to the destinations and copies their data, if the source has any components,
     * they are destroyed before the copying
     *
     * @param dstEntityID destination
     * @param srcEntityID source
     * @return IF ANY OF THE ENTITIES IS INVALID return false, return true otherwise
     */
    bool copy_components_to_from_entity(const size_t dstEntityID, const size_t srcEntityID)
    {
        if (!is_valid_entity(dstEntityID) && !is_valid_entity(srcEntityID)) {
            yorcvs::log("Cannot copy components destination or source are invalid", yorcvs::MSGSEVERITY::ERROR);
            return false;
        }
        componentmanager->copy_component_data_to_from_entity(dstEntityID, srcEntityID);
        const std::vector<bool> newSignature = get_entity_signature(srcEntityID);
        systemmanager->on_entity_signature_change(dstEntityID, newSignature);
        entitymanager->set_signature(dstEntityID, newSignature);
        return true;
    }
    // NOTE: DEBUG FUNCTIONS
    /**
     * @brief Get the number of active entities
     *
     * @return size_t Entities that have been created but not destroyed
     */
    [[nodiscard]] size_t get_active_entities_number() const
    {
        return entitymanager->entitySignatures.size() - entitymanager->freedIndices.size();
    }
    /**
     * @brief Gets the size of the array that holds entity data , the same thing as the maximum number of entitites
     *
     * @return size_t
     */
    [[nodiscard]] size_t get_entity_list_size()
    {
        return entitymanager->entitySignatures.size();
    }
    /**
     * @brief Get a list of names corresponding to registered components
     *
     * @return std::vector<std::string> Components registered to the ECS
     */
    [[nodiscard]] std::vector<std::string> get_registered_components_name()
    {
        std::vector<std::string> names {};
        for (const auto& i : componentmanager->component_type) {
            names.emplace_back(i.first);
        }
        return names;
    }

private:
    std::unique_ptr<yorcvs::ComponentManager> componentmanager;
    std::unique_ptr<yorcvs::EntityManager> entitymanager;
    std::unique_ptr<yorcvs::SystemManager> systemmanager;
    // gets maximum number of components of a certain type created
    // for example if 2 components have been created and 1 deleted
    // calling get_maximum_component_count will return 2
    // only used for debbuging
    template <typename comp, typename returnType = size_t>
    returnType get_maximum_component_count()
    {
        return componentmanager->get_container<comp>()->components.size();
    }
    template <typename T>
    void on_system_signature_change()
    {
        const char* systemType = typeid(T).name();
        // add matching entities to it
        // couldn't find a better place to put it
        for (size_t entity = 0; entity < entitymanager->entitySignatures.size(); entity++) {
            if (systemmanager->compare_entity_to_system(entitymanager->entitySignatures[entity],
                    systemmanager->get_system_signature<T>())) {
                // TODO : MAKE A METHOD TO SYSTEM , method needs to be virtual /Onwntitierase/insert
                insert_sorted(*systemmanager->type_to_system.at(systemType), entity);
            } else {
                // not looking good
                systemmanager->type_to_system.at(systemType)
                    ->erase(std::remove(systemmanager->type_to_system.at(systemType)->begin(),
                                systemmanager->type_to_system.at(systemType)->end(),
                                entity),
                        systemmanager->type_to_system.at(systemType)->end());
            }
        }
    }
};

/**
 *
 * @brief RAII wrapper for ids
 *
 */
class Entity {
public:
    explicit Entity(ECS* ecs)
        : parent(ecs)
    {
        id = parent->create_entity_ID();
        yorcvs::log("Created entity with id: " + std::to_string(id), yorcvs::MSGSEVERITY::INFO);
    }

    Entity(const Entity& other)
        : parent(other.parent)
    {
        id = parent->create_entity_ID();
        parent->copy_components_to_from_entity(id, other.id);
        yorcvs::log("The copy constructor for an Entity " + std::to_string(id) + " has been called: this might be an unecesary expensive option",
            yorcvs::MSGSEVERITY::WARNING);
    }
    Entity(Entity&& other) noexcept
        : id(other.id)
        , parent(other.parent)
    {
        other.parent = nullptr;
    }
    Entity& operator=(const Entity& other)
    {
        if (this == &other) {
            return *this;
        }
        parent = other.parent;
        parent->copy_components_to_from_entity(id, other.id);
        yorcvs::log("The copy assginment operator for Entity" + std::to_string(id) + " has been called: this might be an unecesary expensive option",
            yorcvs::MSGSEVERITY::WARNING);
        return *this;
    }

    Entity& operator=(Entity&& other) noexcept
    {
        parent = other.parent;
        id = other.id;
        other.parent = nullptr;
        return *this;
    }

    ~Entity() noexcept
    {
        if (parent != nullptr) {
            parent->destroy_entity(id);
            yorcvs::log("Destroyed entity with id: " + std::to_string(id), yorcvs::MSGSEVERITY::INFO);
        }
    }
    size_t id = 0;
    ECS* parent = nullptr; // non-owning pointer to parent
};
} // namespace yorcvs
