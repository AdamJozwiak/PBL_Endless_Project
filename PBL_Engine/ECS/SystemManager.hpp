#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <cassert>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "ComponentManager.hpp"
#include "Utilities.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
struct System;

// //////////////////////////////////////////////////////////////////// Class //
class SystemManager {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------------------------ Singleton -- == //
    static SystemManager& instance();

    SystemManager(SystemManager const&) = delete;
    SystemManager(SystemManager&&) = delete;
    SystemManager& operator=(SystemManager const&) = delete;
    SystemManager& operator=(SystemManager&&) = delete;

    // --------------------------------------------------- Registration -- == //
    template <typename SystemType>
    void registerSystemType() {
        std::type_index typeIndex{typeid(SystemType)};
        assert(!isSystemTypeRegistered(typeIndex) &&
               "System type must be registered only once!");

        systems.insert({typeIndex, std::make_shared<SystemType>()});
        signatures.insert({typeIndex, Signature{}});
    }

    template <typename SystemType>
    std::shared_ptr<SystemType> get() {
        std::type_index typeIndex{typeid(SystemType)};
        assert(isSystemTypeRegistered(typeIndex) &&
               "System type must be registered before use!");

        return std::static_pointer_cast<SystemType>(systems[typeIndex]);
    }

    template <typename SystemType, typename ComponentType>
    void filter(bool const active = true) {
        std::type_index typeIndex{typeid(SystemType)};
        assert(isSystemTypeRegistered(typeIndex) &&
               "System type must be registered before use!");

        signatures[typeIndex].set(
            ComponentManager::instance().id<ComponentType>(), active);
    }

    void destroyEntity(EntityId entityId);
    void changeEntitySignature(EntityId entityId,
                               Signature const& entitySignature);

  private:
    // ========================================================= Behaviour == //
    SystemManager() = default;
    ~SystemManager() = default;

    bool isSystemTypeRegistered(std::type_index const& typeIndex) {
        return systems.find(typeIndex) != systems.end();
    }

    // ============================================================== Data == //
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems{};
    std::unordered_map<std::type_index, Signature> signatures{};
};

// ////////////////////////////////////////////////////////////////////////// //
