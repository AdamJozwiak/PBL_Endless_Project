#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <set>

#include "EngineAPI.hpp"
#include "Entity.hpp"
#include "Registry.hpp"
#include "SystemManager.hpp"

// /////////////////////////////////////////////////////////////// Registrant //
template <typename SystemType>
class ENGINE_API SystemRegistrant {};

// ////////////////////////////////////////////////////////////////// Wrapper //
template <typename SystemType>
class ENGINE_API SystemWrapper {
  public:
    template <typename ComponentType>
    SystemWrapper& filter(bool const active = true) {
        registry.filter<SystemType, ComponentType>(active);
        return *this;
    }
    static inline Registry& registry = Registry::instance();
};

// //////////////////////////////////////////////////////////////////// Class //
struct ENGINE_API System {
    friend SystemManager;

    // ========================================================= Behaviour == //
    virtual ~System() = default;
    virtual void filters() = 0;
    virtual void setup() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void release() = 0;

    // ============================================================== Data == //
    std::set<Entity> entities;
};

// /////////////////////////////////////////////////////////////////// Macros //
#define ECS_SYSTEM(T)                              \
    struct ENGINE_API T : public System,           \
                          public SystemWrapper<T>, \
                          public SystemRegistrant<T>

#define ECS_REGISTER_SYSTEM(T) SystemManager::instance().registerSystemType<T>()

// ////////////////////////////////////////////////////////////////////////// //
