#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "EngineAPI.hpp"
#include "Registry.hpp"

// //////////////////////////////////////////////////////////////////// Class //
class ENGINE_API Entity {
  public:
    // ========================================================= Behaviour == //
    Entity(EntityId entityId = EMPTY_ENTITY) : id(entityId) {}
    operator EntityId() const { return id; }

    friend bool operator<(Entity const& a, Entity const& b) {
        return a.id < b.id;
    }

    friend bool operator<=(Entity const& a, Entity const& b) {
        return a.id <= b.id;
    }

    friend bool operator>(Entity const& a, Entity const& b) {
        return a.id > b.id;
    }

    friend bool operator>=(Entity const& a, Entity const& b) {
        return a.id >= b.id;
    }

    friend bool operator==(Entity const& a, Entity const& b) {
        return a.id == b.id;
    }

    friend bool operator!=(Entity const& a, Entity const& b) {
        return a.id != b.id;
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
        return registry.getComponent<ComponentType>(id);
    }

    template <typename ComponentType>
    ComponentType const& get() const {
        return std::as_const(registry).getComponent<ComponentType>(id);
    }

    template <typename ComponentType>
    bool has() const {
        return registry.hasComponent<ComponentType>(id);
    }

    template <typename ComponentType>
    Entity& set(ComponentType const& component) {
        registry.setComponent<ComponentType>(id, component);
        return *this;
    }

    // ============================================================== Data == //
    EntityId const id;

  private:
    // ============================================================== Data == //
    static inline Registry& registry = Registry::instance();
};

// ////////////////////////////////////////////////////////////////////////// //
