#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "ComponentManager.hpp"
#include "EngineAPI.hpp"

// /////////////////////////////////////////////////////////////// Registrant //
template <typename ComponentType>
class ENGINE_API ComponentRegistrant {
    // This will be called only once when instantiating the template
    static inline struct Registrant {
        Registrant() {
            ComponentManager::instance().registerComponentType<ComponentType>();
        }
    } registrant;
};

// /////////////////////////////////////////////////////////////////// Macros //
#define ECS_COMPONENT(T) struct ENGINE_API T : public ComponentRegistrant<T>

// ////////////////////////////////////////////////////////////////////////// //
