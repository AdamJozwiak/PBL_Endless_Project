#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "Components/ForwardDeclarations.hpp"
#include "EngineAPI.hpp"
#include "Utilities.hpp"

// /////////////////////////////////////////////////////////////// Registrant //
struct ENGINE_API ComponentRegistrant {
    template <typename ComponentType>
    static ComponentId id() {
        return EMPTY_COMPONENT;
    }
};

// /////////////////////////////////////////////////////////////////// Macros //
#define ECS_COMPONENT(T) struct ENGINE_API T

#define ECS_SET_COMPONENT_ID(T, N)                    \
    template <>                                       \
    inline ComponentId ComponentRegistrant::id<T>() { \
        return (N);                                   \
    }

#define ECS_REGISTER_COMPONENT(T) \
    ComponentManager::instance().registerComponentType<T>()

// ////////////////////////////////////////////////////////////// Identifiers //
ECS_SET_COMPONENT_ID(AABB, 0u)
ECS_SET_COMPONENT_ID(Animator, 1u)
ECS_SET_COMPONENT_ID(Behaviour, 2u)
ECS_SET_COMPONENT_ID(BoxCollider, 3u)
ECS_SET_COMPONENT_ID(Flame, 4u)
ECS_SET_COMPONENT_ID(Light, 5u)
ECS_SET_COMPONENT_ID(MainCamera, 6u)
ECS_SET_COMPONENT_ID(MeshFilter, 7u)
ECS_SET_COMPONENT_ID(Properties, 8u)
ECS_SET_COMPONENT_ID(RectTransform, 9u)
ECS_SET_COMPONENT_ID(Renderer, 10u)
ECS_SET_COMPONENT_ID(Rigidbody, 11u)
ECS_SET_COMPONENT_ID(Skybox, 12u)
ECS_SET_COMPONENT_ID(SphereCollider, 13u)
ECS_SET_COMPONENT_ID(Active, 14u)
ECS_SET_COMPONENT_ID(Refractive, 15u)
ECS_SET_COMPONENT_ID(CheckCollisions, 16u)
ECS_SET_COMPONENT_ID(Transform, 17u)
ECS_SET_COMPONENT_ID(UIElement, 18u)

// ////////////////////////////////////////////////////////////////////////// //
