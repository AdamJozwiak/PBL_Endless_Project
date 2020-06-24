#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <any>
#include <array>
#include <cassert>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "Component.hpp"
#include "ComponentArray.hpp"
#include "EngineAPI.hpp"
#include "Utilities.hpp"

// /////////////////////////////////////////////////////////////////// Macros //
#define ASSERT_COMPONENT_ID_SET()               \
    assert(isComponentIdSet<ComponentType>() && \
           "Component type identifier must be set before use!")
#define ASSERT_COMPONENT_TYPE_REGISTERED()               \
    assert(isComponentTypeRegistered<ComponentType>() && \
           "Component type must be registered before use!")
#define ASSERT_COMPONENT_TYPE_NOT_REGISTERED()         \
    assert(                                            \
        !isComponentTypeRegistered<ComponentType>() && \
        "Component type must be registered only once with unique identifier!")

// //////////////////////////////////////////////////////////////////// Class //
class ENGINE_API ComponentManager {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    static ComponentManager& instance();

    ComponentManager(ComponentManager const&) = delete;
    ComponentManager(ComponentManager&&) = delete;
    ComponentManager& operator=(ComponentManager const&) = delete;
    ComponentManager& operator=(ComponentManager&&) = delete;

    // --------------------------------------------------- Registration -- == //
    template <typename ComponentType>
    void registerComponentType() {
        ASSERT_COMPONENT_ID_SET();
        ASSERT_COMPONENT_TYPE_NOT_REGISTERED();

        if (isComponentTypeRegistered<ComponentType>()) {
            throw;
        }

        componentArrays.at(id<ComponentType>()) =
            std::make_shared<ComponentArray<ComponentType>>();
    }

    template <typename ComponentType>
    ComponentId id() {
        ASSERT_COMPONENT_ID_SET();
        ASSERT_COMPONENT_TYPE_REGISTERED();

        return ComponentRegistrant::id<ComponentType>();
    }

    // --------------------------------------------- Main functionality -- == //
    template <typename ComponentType>
    void add(EntityId entityId, ComponentType const& component) {
        components<ComponentType>()->insert(entityId, component);
    }

    template <typename ComponentType>
    void remove(EntityId entityId) {
        components<ComponentType>()->remove(entityId);
    }

    template <typename ComponentType>
    ComponentType& get(EntityId entityId) {
        return components<ComponentType>()->get(entityId);
    }

    template <typename ComponentType>
    bool has(EntityId entityId) {
        return components<ComponentType>()->contains(entityId);
    }

    // -------------------------------------------------------- Helpers -- == //
    void destroyEntity(EntityId entityId);

  private:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    ComponentManager() = default;
    ~ComponentManager() = default;

    // --------------------------------------------------- Registration -- == //
    template <typename ComponentType>
    bool isComponentIdSet() {
        return id<ComponentType>() != EMPTY_COMPONENT;
    }

    template <typename ComponentType>
    bool isComponentTypeRegistered() {
        return componentArrays.at(id<ComponentType>()) != nullptr;
    }

    // -------------------------------------------------------- Helpers -- == //
    template <typename ComponentType>
    std::shared_ptr<ComponentArray<ComponentType>> components() {
        ASSERT_COMPONENT_ID_SET();
        ASSERT_COMPONENT_TYPE_REGISTERED();

        return std::static_pointer_cast<ComponentArray<ComponentType>>(
            componentArrays.at(id<ComponentType>()));
    }

    // ============================================================== Data == //
    size_t numberOfRegisteredComponents{0};
    std::array<std::shared_ptr<IComponentArray>, MAX_COMPONENTS>
        componentArrays{};
};

// ////////////////////////////////////////////////////////////////////////// //
