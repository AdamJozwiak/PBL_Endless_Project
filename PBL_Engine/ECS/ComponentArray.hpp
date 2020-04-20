#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <array>
#include <cassert>
#include <unordered_map>

#include "Utilities.hpp"

// //////////////////////////////////////////////////////////////// Interface //
class IComponentArray {
  public:
    // ========================================================= Behaviour == //
    virtual ~IComponentArray() = default;
    virtual void destroyEntity(EntityId entityId) = 0;
};

// //////////////////////////////////////////////////////////////////// Class //
template <typename Component>
class ComponentArray : public IComponentArray {
  public:
    // ========================================================= Behaviour == //
    void insert(EntityId entityId, Component const &component) {
        assert(!componentExists(entityId) &&
               "Component can't be added to same entity more than once!");

        size_t insertedIndex = size;
        EntityId &insertedEntityId = entityId;

        // Insert new element at the end of the array
        components[insertedIndex] = component;
        ++size;

        // Update mappings
        indicies[entityId] = insertedIndex;
        entities[insertedIndex] = entityId;
    }

    void remove(EntityId entityId) {
        if (!componentExists(entityId)) {
            return;
        }
        // assert(componentExists(entityId) &&
        //       "Component must exist before removing!");

        size_t lastIndex = size - 1;
        size_t removedIndex = indicies[entityId];
        EntityId lastEntityId = entities[lastIndex];
        EntityId &removedEntityId = entityId;

        // Overwrite the removed element with the last one
        components[removedIndex] = components[lastIndex];
        --size;

        // Update mappings
        indicies[lastEntityId] = removedIndex;
        entities[removedIndex] = lastEntityId;
        indicies.erase(removedEntityId);
        entities.erase(lastIndex);
    }

    Component &get(EntityId entityId) {
        assert(componentExists(entityId) &&
               "Component doesn't exist for given entity!");

        return components[indicies[entityId]];
    }

    void destroyEntity(EntityId entityId) override {
        // assert(componentExists(entityId) &&
        //       "Component doesn't exist for given entity!");

        remove(entityId);
    }

  private:
    // ========================================================= Behaviour == //
    bool componentExists(EntityId const &entityId) {
        return indicies.find(entityId) != indicies.end();
    }

    // ============================================================== Data == //
    std::array<Component, MAX_ENTITIES> components{};
    size_t size{};

    std::unordered_map<EntityId, size_t> indicies{};
    std::unordered_map<size_t, EntityId> entities{};
};

// ////////////////////////////////////////////////////////////////////////// //
