#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <array>
#include <unordered_map>

#include "EngineAPI.hpp"
#include "Utilities.hpp"
#include "assert.hpp"

// //////////////////////////////////////////////////////////////// Interface //
class ENGINE_API IComponentArray {
  public:
    // ========================================================= Behaviour == //
    virtual ~IComponentArray() = default;
    virtual void destroyEntity(EntityId entityId) = 0;
};

// //////////////////////////////////////////////////////////////////// Class //
template <typename Component>
class ENGINE_API ComponentArray : public IComponentArray {
  public:
    ComponentArray() {
        size = 0;
        indicies.fill(EMPTY_ENTITY);
        entities.fill(EMPTY_ENTITY);
    }

    // ========================================================= Behaviour == //
    void insert(EntityId const entityId, Component const &component) {
        assert(entityExists(entityId) && "Entity doesn't exist!");

        if (componentExists(entityId)) {
            get(entityId) = component;
            return;
        }

        if (size > MAX_ENTITIES) {
            return;
        }

        auto const insertedIndex = size;
        auto const insertedEntityId = entityId;

        // Insert new element at the end of the array
        components.at(insertedIndex) = component;
        ++size;

        // Update mappings
        indicies.at(entityId) = insertedIndex;
        entities.at(insertedIndex) = entityId;
    }

    void remove(EntityId const entityId) {
        assert(entityExists(entityId) && "Entity doesn't exist!");

        if (!componentExists(entityId)) {
            return;
        }

        auto const lastIndex = size - 1u;
        auto const removedIndex = indicies.at(entityId);
        auto const lastEntityId = entities.at(lastIndex);
        auto const removedEntityId = entityId;

        // Overwrite the removed element with the last one
        components.at(removedIndex) = components.at(lastIndex);
        --size;

        // Update mappings
        indicies.at(lastEntityId) = removedIndex;
        entities.at(removedIndex) = lastEntityId;
        indicies.at(removedEntityId) = EMPTY_ENTITY;
        entities.at(lastIndex) = EMPTY_ENTITY;
    }

    Component &get(EntityId const entityId) {
        assert(entityExists(entityId) && "Entity doesn't exist!");
        assert(componentExists(entityId) &&
               "Component doesn't exist for given entity!");

        return components.at(indicies.at(entityId));
    }

    bool contains(EntityId const entityId) { return componentExists(entityId); }

    void destroyEntity(EntityId const entityId) override { remove(entityId); }

  private:
    // ========================================================= Behaviour == //
    bool componentExists(EntityId const &entityId) {
        assert(entityExists(entityId) && "Entity doesn't exist!");
        return indicies.at(entityId) != EMPTY_ENTITY;
    }

    constexpr bool entityExists(EntityId const &entityId) {
        return entityId != EMPTY_ENTITY;
    }

    // ============================================================== Data == //
    std::array<Component, MAX_ENTITIES> components{};
    size_t size{0};

    std::array<size_t, MAX_ENTITIES> indicies{};
    std::array<EntityId, MAX_ENTITIES> entities{};
};

// ////////////////////////////////////////////////////////////////////////// //
