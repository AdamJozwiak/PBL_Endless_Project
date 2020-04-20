#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <any>
#include <cassert>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "ComponentArray.hpp"
#include "EngineAPI.hpp"
#include "Utilities.hpp"

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
        std::type_index typeIndex{typeid(ComponentType)};
        assert(!isComponentTypeRegistered(typeIndex) &&
               "Component type must be registered only once!");

        componentIds.insert({typeIndex, componentIds.size()});
        componentArrays.insert(
            {typeIndex, std::make_shared<ComponentArray<ComponentType>>()});
    }

    template <typename ComponentType>
    ComponentId id() {
        std::type_index typeIndex{typeid(ComponentType)};
        assert(isComponentTypeRegistered(typeIndex) &&
               "Component type must be registered before use!");

        return componentIds[typeIndex];
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

    // -------------------------------------------------------- Helpers -- == //
    void destroyEntity(EntityId entityId);

  private:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    ComponentManager() = default;
    ~ComponentManager() = default;

    // --------------------------------------------------- Registration -- == //
    bool isComponentTypeRegistered(std::type_index const& typeIndex);

    // -------------------------------------------------------- Helpers -- == //
    template <typename ComponentType>
    std::shared_ptr<ComponentArray<ComponentType>> components() {
        std::type_index typeIndex{typeid(ComponentType)};
        assert(isComponentTypeRegistered(typeIndex) &&
               "Component type must be registered before use!");

        return std::static_pointer_cast<ComponentArray<ComponentType>>(
            componentArrays[typeIndex]);
    }

    // ============================================================== Data == //
    std::unordered_map<std::type_index, ComponentId> componentIds{};
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>>
        componentArrays{};
};

// ////////////////////////////////////////////////////////////////////////// //
