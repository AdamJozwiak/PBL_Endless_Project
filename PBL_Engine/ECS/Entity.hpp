#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "EngineAPI.hpp"
#include "Registry.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class ENGINE_API Entity {
  public:
    // ========================================================= Behaviour == //
    Entity(EntityId entityId) : id(entityId) {}

    friend bool operator<(Entity const& a, Entity const& b) {
        return a.id < b.id;
    }

    template <typename ComponentType>
    Entity& add(ComponentType const& component) {
        registry.addComponent(id, component);
        return *this;
    }

    template <typename ComponentType>
    Entity& remove() {
        registry.removeComponent<ComponentType>(id);
        return *this;
    }

    template <typename ComponentType>
    ComponentType& get() {
        return registry.component<ComponentType>(id);
    }

    template <typename ComponentType>
    ComponentType const& get() const {
        return registry.component<ComponentType>(id);
    }

    template <typename ComponentType>
    Entity& set(ComponentType const& component) {
        get<ComponentType>() = component;
        return *this;
    }

    // ============================================================== Data == //
    EntityId const id;

  private:
    // ============================================================== Data == //
    static inline Registry& registry = Registry::instance();
};

// ////////////////////////////////////////////////////////////////////////// //
