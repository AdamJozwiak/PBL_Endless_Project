#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <any>
#include <array>
#include <memory>
#include <mutex>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "Component.hpp"
#include "ComponentArray.hpp"
#include "EngineAPI.hpp"
#include "Utilities.hpp"
#include "assert.hpp"

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
    template <typename ComponentType, bool threadSafe = true>
    void registerComponentType() {
        ASSERT_COMPONENT_ID_SET();

        LOCK_GUARD(componentArrayMutex<ComponentType>(), threadSafe);

        ASSERT_COMPONENT_TYPE_NOT_REGISTERED();

        componentArrays.at(id<ComponentType>()) =
            std::make_shared<ComponentArray<ComponentType>>();
    }

    template <typename ComponentType>
    constexpr ComponentId id() {
        ASSERT_COMPONENT_ID_SET();

        return ComponentRegistrant::id<ComponentType>();
    }

    // --------------------------------------------- Main functionality -- == //
    template <typename ComponentType, bool threadSafe = true>
    void add(EntityId entityId, ComponentType const& component) {
        LOCK_GUARD(componentArrayMutex<ComponentType>(), threadSafe);

        components<ComponentType>()->insert<false>(entityId, component);
    }

    template <typename ComponentType, bool threadSafe = true>
    void remove(EntityId entityId) {
        LOCK_GUARD(componentArrayMutex<ComponentType>(), threadSafe);

        components<ComponentType>()->remove<false>(entityId);
    }

    template <typename ComponentType, bool threadSafe = true>
    ComponentType& get(EntityId entityId) {
        LOCK_GUARD(componentArrayMutex<ComponentType>(), threadSafe);

        return components<ComponentType>()->get<false>(entityId);
    }

    template <typename ComponentType, bool threadSafe = true>
    bool has(EntityId entityId) {
        LOCK_GUARD(componentArrayMutex<ComponentType>(), threadSafe);

        return components<ComponentType>()->contains<false>(entityId);
    }

    // -------------------------------------------------------- Helpers -- == //
    template <bool threadSafe = true>
    void destroyEntity(EntityId const entityId) {
        LOCK_GUARD(componentManagerMutex, threadSafe);

        for (auto const& components : componentArrays) {
            if (components) {
                components->destroyEntity(entityId);
            }
        }
    }

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
        return componentArrays.at(ComponentRegistrant::id<ComponentType>()) !=
               nullptr;
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
    std::mutex componentManagerMutex;

    template <typename ComponentType>
    std::mutex& componentArrayMutex() {
        static std::mutex mutex;
        return mutex;
    };

    std::array<std::shared_ptr<IComponentArray>, MAX_COMPONENTS>
        componentArrays{};
};

// ////////////////////////////////////////////////////////////////////////// //
