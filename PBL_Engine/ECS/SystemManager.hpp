#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <memory>
#include <mutex>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "ComponentManager.hpp"
#include "EngineAPI.hpp"
#include "Utilities.hpp"
#include "assert.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
struct ENGINE_API System;

// //////////////////////////////////////////////////////////////////// Class //
class ENGINE_API SystemManager {
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
        RECURSIVE_LOCK_GUARD(systemManagerRecursiveMutex, true);

        std::type_index typeIndex{typeid(SystemType)};
        assert(!isSystemTypeRegistered(typeIndex) &&
               "System type must be registered only once!");

        systems.insert({typeIndex, std::make_shared<SystemType>()});
        signatures.insert({typeIndex, Signature{}});

        systems.at(typeIndex)->filters();
    }

    template <typename SystemType>
    std::shared_ptr<SystemType> get() {
        RECURSIVE_LOCK_GUARD(systemManagerRecursiveMutex, true);

        std::type_index typeIndex{typeid(SystemType)};
        assert(isSystemTypeRegistered(typeIndex) &&
               "System type must be registered before use!");

        return std::static_pointer_cast<SystemType>(systems.at(typeIndex));
    }

    template <typename SystemType, typename ComponentType>
    void filter(bool const active = true) {
        RECURSIVE_LOCK_GUARD(systemManagerRecursiveMutex, true);

        std::type_index typeIndex{typeid(SystemType)};
        assert(isSystemTypeRegistered(typeIndex) &&
               "System type must be registered before use!");

        signatures.at(typeIndex).components.set(
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
    std::recursive_mutex systemManagerRecursiveMutex;

    std::unordered_map<std::type_index, std::shared_ptr<System>> systems{};
    std::unordered_map<std::type_index, Signature> signatures{};
};

// ////////////////////////////////////////////////////////////////////////// //
