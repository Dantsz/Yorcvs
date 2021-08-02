#pragma once
#include <memory>

#include "component.h"
#include "entity.h"
#include "system.h"
#include "../common/log.h"
/*The ECS is heaviley isnpired by AUSTIN MORLAN's implementation , but it has a bit more functionality and ease of use.
  It's a bit more 'loose'. It's minimal and doesn't have very specific function so it may be interchangeable with other
  ECS.
*/

namespace yorcvs
{
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
        yorcvs::log("Initializing ECS",yorcvs::INFO);
        componentmanager = std::make_unique<yorcvs::ComponentManager>();
        entitymanager = std::make_unique<yorcvs::EntityManager>();
        systemmanager = std::make_unique<yorcvs::SystemManager>();
    }
    ~ECS()
    {
        yorcvs::log("Destroying ECS",yorcvs::INFO);
        componentmanager.reset();
        entitymanager.reset();
        systemmanager.reset();
    }
    // assigns a new id for an entity
    /**
     * @brief Create a Entity ID object
     * 
     * @return size_t the ID of the entity
     * NOTE: IDs created by this function are not managed by the ecs and should be freed using destroyEntity
     */
    [[nodiscard]] size_t createEntityID()
    {
        size_t ID = entitymanager->addEntity();
        systemmanager->onEntitySignatureChange(ID, entitymanager->getSignature(ID));

        return ID;
    }
    /**
     * @brief Frees an entity ID and removes it's components
     * 
     * @param id ID of the entity
     */
    void destroyEntity(const size_t id)
    {
        entitymanager->deleteEntity(id);
        componentmanager->OnEntityDestroyed(id);
        systemmanager->OnEntityDestroy(id);
    }
    /**
     * @brief Get the Entity Signature 
     * 
     * @param entityID Entity ID
     * @return std::vector<bool> List of all components, 1 if they have the component  or 0 otherwise
     */
    std::vector<bool> getEntitySignature(const size_t entityID)
    {
        return entitymanager->getSignature(entityID);
    }
    /**
     * @brief Registers a component making the ECS aware of it
     * 
     * @tparam T The component object
     */
    template <typename T> void registerComponent()
    {
        componentmanager->registerComponent<T>();
    }

    /**
     * @brief Registers multiple components in the same function call
     * 
     * @tparam T A component
     * @tparam secondT  Another component 
     * @tparam OtherT  The rest
     */
    template <typename T, typename secondT, typename... OtherT> void registerComponent()
    {
        componentmanager->registerComponent<T>();
        registerComponent<secondT, OtherT...>();
    }

    /**
     * @brief Checks if a component is registered
     * 
     * @tparam T component
     * @return true the component is registered
     * @return false it's not registered
     */
    template <typename T> bool isComponentRegistered()
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
    template <typename T> void addComponent(const size_t entityID,T component)
    {
        // add the component
        componentmanager->addComponent<T>(entityID, component);
        // modify the signature to match the new addition
        std::vector<bool> e_signature = entitymanager->getSignature(entityID);
        size_t component_type = componentmanager->getComponentID<T>();
        // while the vector of signature doesn't have elements until the current component add 0 to the signature
        while (e_signature.size() <= component_type)
        {
            e_signature.push_back(0);
        }
        // add the new signature
        e_signature[component_type] = 1;
        entitymanager->setSignature(entityID, e_signature);
        systemmanager->onEntitySignatureChange(entityID, e_signature);
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
    void addComponent(const size_t entityID,T component, const Other &...other)
    {
        // add the component
        componentmanager->addComponent<T>(entityID, component);
        // modify the signature to match the new addition
        std::vector<bool> e_signature = entitymanager->getSignature(entityID);
        size_t component_type = componentmanager->getComponentID<T>();
        // while the vector of signature doesn't have elements until the current component add 0 to the signature
        while (e_signature.size() <= component_type)
        {
            e_signature.emplace_back(0);
        }
        // add the new signature
        e_signature[component_type] = 1;
        entitymanager->setSignature(entityID, e_signature);
        systemmanager->onEntitySignatureChange(entityID, e_signature);
        addComponent<Other...>(entityID, other...);
    }
    /**
     * @brief Removes component T from the entity
     * 
     * @tparam T Component 
     * @param entityID entity
     */
    template <typename T> void removeComponent(const size_t entityID)
    {

        std::vector<bool> &e_signature = entitymanager->getSignature(entityID);
        size_t component_type = componentmanager->getComponentID<T>();
        e_signature[component_type] = 0;
        systemmanager->onEntitySignatureChange(entityID, e_signature);
        componentmanager->removeComponent<T>(entityID);
    }
    /**
     * @brief Removes two or more components from the entity
     * 
     * @tparam T first component type
     * @tparam secondT second component type
     * @tparam Other other components type
     * @param entityID the ID of the entity
     */
    template <typename T, typename secondT, typename... Other> void removeComponent(const size_t entityID)
    {

        std::vector<bool> &e_signature = entitymanager->getSignature(entityID);
        size_t component_type = componentmanager->getComponentID<T>();
        e_signature[component_type] = 0;
        systemmanager->onEntitySignatureChange(entityID, e_signature);
        componentmanager->removeComponent<T>(entityID);

        removeComponent<secondT, Other...>(entityID);
    }
    /**
     * @brief Checks if an entity has a component
     * 
     * @tparam T The component type
     * @param entityID Id of the entity
     * @return true it has the component
     * @return false it dowsn't
     */
    template <typename T> bool hasComponents(const size_t entityID)
    {
        return componentmanager->getContainer<T>()->hasComponent(entityID);
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
    template <typename T, typename secondT, typename... Other> bool hasComponents(const size_t entityID)
    {
        if (!componentmanager->getContainer<T>()->hasComponent(entityID))
            return 0;
        return hasComponents<secondT, Other...>(entityID);
    }
    /**
     * @brief Returns a reference tthe component of the entity
     * 
     * @tparam T the type of component
     * @param entityID ID of the entity
     * @return T& the component
     */
    template <typename T> T &getComponent(const size_t entityID)
    {
        return componentmanager->getComponent<T>(entityID);
    }


    /**
     * @brief Registers a system so the ECS can track which entities should be used by the system
     * 
     * @tparam T The system type
     * @param sys Reference to an instance of type system
     */
    template <typename T> void registerSystem(T &sys)
    {
        // if registering the system succeded
        if (systemmanager->registerSystem<T>(sys))
        {
            onSystemSignatureChange<T>();
        }
        return;
    }
    /**
     * @brief Checks if a system is registered
     * 
     * @tparam T The system
     * @return true It is 
     * @return false It is not
     */
    template <typename T> bool is_system_registered()
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
    template <typename T> void setSystemSignature(std::vector<bool> &signature)
    {
        systemmanager->setSignature<T>(signature);
    }

    /**
     * @brief Returns the signature of a system
     * 
     * @tparam T The system
     * @return std::vector<bool> Value of the systems signature
     */
    template <typename T> std::vector<bool> getSystemSignature()
    {
        return systemmanager->getSystemSignature<T>();
    }

    /**
     * @brief  add the components to the system <sys> as a criteria for iteration , if the entity doen't have the components  specified , it will not iterate ovr them     
     * 
     * 
     */

    template <typename sys> void addCriteriaForIteration()
    {
        // update signature when all components have been added
        onSystemSignatureChange<sys>();
    }
    /**
     * @brief Adds more components at once
     * 
     * @tparam sys the system
     * @tparam comp first components
     * @tparam comps other components
     */
    template <typename sys, typename comp, typename... comps> void addCriteriaForIteration()
    {
        // get the current signature of sys
        std::vector<bool> signature = getSystemSignature<sys>();
        // get the id of the component
        size_t componentID = getComponentID<comp>();
        // modify the signature to fit the new component
        while (signature.size() <= componentID)
        {
            signature.emplace_back(0);
        }
        // mark the component as being a part of the system
        signature[componentID] = 1;
        // set the new signature
        setSystemSignature<sys>(signature);

        addCriteriaForIteration<sys, comps...>();
    }

    template <typename sys> void setCriteriaForIteration()
    {
        onSystemSignatureChange<sys>();
    }
    /**
     * @brief Sets the Criteria For Iteration,removes other criteria 
     * 
     * @tparam sys The system 
     * @tparam comp First component
     * @tparam comps Other components
     */
    template <typename sys, typename comp, typename... comps> void setCriteriaForIteration()
    {
        // get the current signature of sys
        std::vector<bool> signature = getSystemSignature<sys>();

        // reset criteria

        for (size_t i = 0; i < signature.size(); i++)
        {
            signature[i] = 0;
        }

        // get the id of the component
        size_t componentID = getComponentID<comp>();
        // modify the signature to fit the new component
        while (signature.size() <= componentID)
        {
            signature.emplace_back(0);
        }
        // mark the component as being a part of the system
        signature[componentID] = 1;
        // set the new signature
        setSystemSignature<sys>(signature);
    }

    
    /**
     * @brief Get the number of entities that have the specified component
     * 
     * @tparam T Component 
     * @return size_t The number of entities with that component
     * NOTE: This is might be costly
     */
    template<typename T>
    size_t getEntitiesWithComponent()
    {
        //get component index
        size_t cIndex = componentmanager->getComponentID<T>();
        size_t entities = 0;
        //unused entites have an emtpy signature so a false pozitive should happen
        for(const auto& i : entitymanager->entitySignatures)
        {
            if(i.size() > cIndex)
            {
             entities += static_cast<size_t>(i[cIndex]);
            }
        }
        return entities;

    }

    void entityCopyComponentToEntity(const size_t dstEntityID , const size_t srcEntityID)
    {
        componentmanager->entityCopyComponentToEntity(dstEntityID,srcEntityID);
        std::vector<bool> newSignature = getEntitySignature(srcEntityID);
        systemmanager->onEntitySignatureChange(dstEntityID,newSignature);
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
        return componentmanager->getContainer<comp>()->components.size();
    }
    template <typename T> size_t getComponentID()
    {
        return componentmanager->getComponentID<T>();
    }
    template <typename T> void onSystemSignatureChange()
    {
        const char *systemType = typeid(T).name();
        // add matching entities to it
        // couldn't find a better place to put it

        for (size_t entity = 0; entity < entitymanager->lowestUnallocatedID; entity++)
        {
            if (systemmanager->compareEntityToSystem(entitymanager->entitySignatures[entity],
                                                         systemmanager->getSystemSignature<T>()))
            {
                // TODO : MAKE A METHOD TO SYSTEM , method needs to be virtual /Onewntitierase/insert
                insertSorted(systemmanager->typetosystem.at(systemType)->entitiesID, entity);
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
 * @brief RAII wrapper for ids
 *
 */
class Entity
{
  public:
    Entity(ECS *ecs)
    {   
        parent = ecs; 
        id = parent->createEntityID();
        yorcvs::log("Created entity with id: " + std::to_string(id), yorcvs::INFO);
    }
  
    Entity(const Entity& other)
    {
        parent = other.parent;
        id = parent->createEntityID();
        parent->entityCopyComponentToEntity(id,other.id);
        yorcvs::log("The copy constructor for an Entity " + std::to_string(id) + " has been called: this might be an unecesary expensive option",yorcvs::WARNING);
    }
    Entity(Entity&& other) noexcept
    {
        parent = other.parent;
        id = other.id;
        other.parent = nullptr;
    }   
    Entity &operator=(const Entity &other)
    {
        parent = other.parent;
        parent->entityCopyComponentToEntity(id,other.id);
        yorcvs::log("The copy assginment operator for Entity" + std::to_string(id) + " has been called: this might be an unecesary expensive option",yorcvs::WARNING);
        return *this;
    }

    Entity &operator=(Entity &&other) noexcept
    {
        parent = other.parent;
        id = other.id;
        other.parent = nullptr;
        return *this;
    }

    ~Entity()
    {
        if(parent != nullptr)
        {
            parent->destroyEntity(id);
        }
        yorcvs::log("Destroyed entity with id: " + std::to_string(id), yorcvs::INFO);
    }

    size_t id = 0;
   
  private:
     ECS *parent; // non-owning pointer to parent
};
} // namespace yorcvs
