#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>
#include <vector>

#include "ComponentManager.hpp"
#include "EngineAPI.hpp"
#include "EntityManager.hpp"
#include "EventManager.hpp"
#include "SystemManager.hpp"
#include "Utilities.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
class ENGINE_API Entity;

// //////////////////////////////////////////////////////////////////// Class //
class ENGINE_API Registry {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    static Registry& instance();

    Registry(Registry const&) = delete;
    Registry(Registry&&) = delete;
    Registry& operator=(Registry const&) = delete;
    Registry& operator=(Registry&&) = delete;

    // ---------------------------------------- Delayed entity deletion -- == //
    bool refresh();

    // --------------------------------------------------------- Entity -- == //
    Entity createEntity();
    void destroyEntity(Entity const& entity);

    // ------------------------------------------------------ Component -- == //
    template <typename ComponentType>
    void addComponent(EntityId entityId, ComponentType const& component) {
        componentManager.add<ComponentType>(entityId, component);
        updateEntitySignature<ComponentType>(entityId, true);
    }

    template <typename ComponentType>
    void removeComponent(EntityId entityId) {
        componentManager.remove<ComponentType>(entityId);
        updateEntitySignature<ComponentType>(entityId, false);
    }

    template <typename ComponentType>
    ComponentType& component(EntityId entityId) {
        return componentManager.get<ComponentType>(entityId);
    }

    template <typename ComponentType>
    ComponentId componentId() {
        return componentManager.id<ComponentType>();
    }

    template <typename ComponentType>
    bool hasComponent(EntityId entityId) {
        return componentManager.has<ComponentType>(entityId);
    }

    // --------------------------------------------------------- System -- == //
    template <typename SystemType>
    std::shared_ptr<SystemType> system() {
        return systemManager.get<SystemType>();
    }

    template <typename SystemType, typename ComponentType>
    void filter(bool const active = true) {
        systemManager.filter<SystemType, ComponentType>(active);
    }

    // ---------------------------------------------------------- Event -- == //
    template <typename EventType>
    void listen(std::function<void(EventType const&)> const& listener) {
        eventManager.listen(listener);
    }

    template <typename EventType>
    void send(EventType const& event) {
        eventManager.send(event);
    }

  private:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    Registry();
    ~Registry() = default;

    // -------------------------------------------------------- Helpers -- == //
    template <typename Component>
    void updateEntitySignature(EntityId entityId, bool active) {
        auto signature = entityManager.getSignature(entityId);
        signature.set(componentManager.id<Component>(), active);
        entityManager.setSignature(entityId, signature);

        systemManager.changeEntitySignature(entityId, signature);
    }

    // ============================================================== Data == //
    // ------------------------------------------------------- Managers -- == //
    ComponentManager& componentManager;
    EntityManager& entityManager;
    EventManager& eventManager;
    SystemManager& systemManager;

    // ---------------------------------------- Delayed entity deletion -- == //
    std::vector<Entity> entitiesToRemove;
};

// ////////////////////////////////////////////////////////////////////////// //
