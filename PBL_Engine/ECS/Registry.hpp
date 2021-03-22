#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include "ComponentManager.hpp"
#include "EngineAPI.hpp"
#include "EntityManager.hpp"
#include "EventManager.hpp"
#include "Events/OnComponent.hpp"
#include "Events/OnEntity.hpp"
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

    // -------------------------------------------------- Thread safety -- == //
    void setThreadSafety(bool const threadSafety);

    // ---------------------------------------------------------- Cache -- == //
    void moveCacheToMainScene();
    void clearCache();

    // --------------------------------------------------------- Entity -- == //
    Entity createEntity(SceneId sceneId = DEFAULT_SCENE);

    template <bool threadSafe = true>
    void destroyEntity(Entity const& entity) {
        LOCK_GUARD(registryMutex, threadSafe);

        if (cachedEntities.contains(entity)) {
            cachedEntities.erase(entity);
        }
        entitiesToRemove.push_back(entity);
    }

    // ------------------------------------------------------ Component -- == //
    template <typename ComponentType>
    void addComponent(EntityId entityId, ComponentType const& component) {
        if (threadSafety) {
            return addComponent_<ComponentType, true>(entityId, component);
        } else {
            return addComponent_<ComponentType, false>(entityId, component);
        }
    }

    template <typename ComponentType>
    void removeComponent(EntityId entityId) {
        if (threadSafety) {
            removeComponent_<ComponentType, true>(entityId);
        } else {
            removeComponent_<ComponentType, false>(entityId);
        }
    }

    template <typename ComponentType>
    ComponentType& getComponent(EntityId entityId) {
        if (threadSafety) {
            return componentManager.get<ComponentType, true>(entityId);
        } else {
            return componentManager.get<ComponentType, false>(entityId);
        }
    }

    template <typename ComponentType>
    ComponentType const& getComponent(EntityId entityId) const {
        if (threadSafety) {
            return componentManager.get<ComponentType, true>(entityId);
        } else {
            return componentManager.get<ComponentType, false>(entityId);
        }
    }

    template <typename ComponentType>
    void setComponent(EntityId entityId, ComponentType const& component) {
        if (threadSafety) {
            setComponent_<ComponentType, true>(entityId, component);
        } else {
            setComponent_<ComponentType, false>(entityId, component);
        }
    }

    template <typename ComponentType>
    constexpr ComponentId componentId() {
        return componentManager.id<ComponentType>();
    }

    template <typename ComponentType>
    bool hasComponent(EntityId entityId) {
        if (threadSafety) {
            return componentManager.has<ComponentType, true>(entityId);
        } else {
            return componentManager.has<ComponentType, false>(entityId);
        }
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
        signature.components.set(componentManager.id<Component>(), active);
        entityManager.setSignature(entityId, signature);

        systemManager.changeEntitySignature(entityId, signature);
    }

    template <typename ComponentType, bool threadSafe = true>
    void addComponent_(EntityId entityId, ComponentType const& component) {
        RECURSIVE_LOCK_GUARD(recursiveMutex, threadSafe);

        componentManager.add<ComponentType, threadSafe>(entityId, component);
        updateEntitySignature<ComponentType>(entityId, true);
    }

    template <typename ComponentType, bool threadSafe = true>
    void removeComponent_(EntityId entityId) {
        RECURSIVE_LOCK_GUARD(recursiveMutex, threadSafe);

        componentManager.remove<ComponentType, threadSafe>(entityId);
        updateEntitySignature<ComponentType>(entityId, false);
    }

    template <typename ComponentType, bool threadSafe = true>
    void setComponent_(EntityId entityId, ComponentType const& component) {
        RECURSIVE_LOCK_GUARD(recursiveMutex, threadSafe);

        componentManager.get<ComponentType, threadSafe>(entityId) = component;
        send(OnComponentUpdate<ComponentType>{entityId});
    }

    // ============================================================== Data == //
    std::mutex registryMutex;
    std::recursive_mutex recursiveMutex;
    bool threadSafety;

    std::set<Entity> cachedEntities;

    // ------------------------------------------------------- Managers -- == //
    ComponentManager& componentManager;
    EntityManager& entityManager;
    EventManager& eventManager;
    SystemManager& systemManager;

    // ---------------------------------------- Delayed entity deletion -- == //
    std::vector<Entity> entitiesToRemove;
};

// ////////////////////////////////////////////////////////////////////////// //
