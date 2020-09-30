#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <any>
#include <array>
#include <cassert>
#include <memory>
#include <mutex>
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
        std::lock_guard<std::recursive_mutex> lockGuard{mutex};

        ASSERT_COMPONENT_ID_SET();
        ASSERT_COMPONENT_TYPE_NOT_REGISTERED();

        if (isComponentTypeRegistered<ComponentType>()) {
            throw;
        }

        componentArrays.at(id<ComponentType>()) =
            std::make_shared<ComponentArray<ComponentType>>();
    }

    template <typename ComponentType>
    constexpr ComponentId id() {
        ASSERT_COMPONENT_ID_SET();

        return ComponentRegistrant::id<ComponentType>();
    }

    // --------------------------------------------- Main functionality -- == //
    template <typename ComponentType>
    void add(EntityId entityId, ComponentType const& component) {
        std::lock_guard<std::recursive_mutex> lockGuard{mutex};

        components<ComponentType>()->insert(entityId, component);
    }

    template <typename ComponentType>
    void remove(EntityId entityId) {
        std::lock_guard<std::recursive_mutex> lockGuard{mutex};

        components<ComponentType>()->remove(entityId);
    }

    template <typename ComponentType>
    ComponentType& get(EntityId entityId) {
        std::lock_guard<std::recursive_mutex> lockGuard{mutex};

        return components<ComponentType>()->get(entityId);
    }

    template <typename ComponentType>
    bool has(EntityId entityId) {
        std::lock_guard<std::recursive_mutex> lockGuard{mutex};

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
    constexpr bool isComponentIdSet() {
        return ComponentRegistrant::id<ComponentType>() != EMPTY_COMPONENT;
    }

    template <typename ComponentType>
    bool isComponentTypeRegistered() {
        std::lock_guard<std::recursive_mutex> lockGuard{mutex};

        return componentArrays.at(ComponentRegistrant::id<ComponentType>()) !=
               nullptr;
    }

    // -------------------------------------------------------- Helpers -- == //
    template <typename ComponentType>
    std::shared_ptr<ComponentArray<ComponentType>> components() {
        std::lock_guard<std::recursive_mutex> lockGuard{mutex};

        ASSERT_COMPONENT_ID_SET();
        ASSERT_COMPONENT_TYPE_REGISTERED();

        return std::static_pointer_cast<ComponentArray<ComponentType>>(
            componentArrays.at(id<ComponentType>()));
    }

    // ============================================================== Data == //
    std::recursive_mutex mutex;

    size_t numberOfRegisteredComponents{0};
    std::array<std::shared_ptr<IComponentArray>, MAX_COMPONENTS>
        componentArrays{};
};

// ////////////////////////////////////////////////////////////////////////// //
