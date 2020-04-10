#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <set>

#include "Registry.hpp"
#include "SystemManager.hpp"

// ///////////////////////////////////////////////////// Forward declarations //
class Entity;

// /////////////////////////////////////////////////////////////// Registrant //
template <typename SystemType>
class SystemRegistrant {
    // This will be called only once when instantiating the template
    static inline struct Registrant {
        Registrant() {
            SystemManager::instance().registerSystemType<SystemType>();
        }
    } registrant;
};

// ////////////////////////////////////////////////////////////////// Wrapper //
template <typename SystemType>
class SystemWrapper {
  public:
    template <typename ComponentType>
    SystemWrapper& filter(bool const active = true) {
        registry.filter<SystemType, ComponentType>(active);
        return *this;
    }
    static inline Registry& registry = Registry::instance();
};

// //////////////////////////////////////////////////////////////////// Class //
struct System {
    // ========================================================= Behaviour == //
    virtual ~System() = default;
    virtual void update(float deltaTime) = 0;

    // ============================================================== Data == //
    std::set<Entity> entities;
};

// /////////////////////////////////////////////////////////////////// Macros //
#define ECS_SYSTEM(T)                   \
    struct T : public System,           \
               public SystemWrapper<T>, \
               public SystemRegistrant<T>

// ////////////////////////////////////////////////////////////////////////// //
