#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "ComponentManager.hpp"

// /////////////////////////////////////////////////////////////// Registrant //
template <typename ComponentType>
class ComponentRegistrant {
    // This will be called only once when instantiating the template
    static inline struct Registrant {
        Registrant() {
            ComponentManager::instance().registerComponentType<ComponentType>();
        }
    } registrant;
};

// /////////////////////////////////////////////////////////////////// Macros //
#define ECS_COMPONENT(T) struct T : public ComponentRegistrant<T>

// ////////////////////////////////////////////////////////////////////////// //
