#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <bitset>

// ///////////////////////////////////////////////////// Usings and constants //
using SceneId = unsigned int;
constexpr SceneId MAX_SCENES = 2u;
constexpr SceneId EMPTY_SCENE = MAX_SCENES + 1u;
constexpr SceneId DEFAULT_SCENE = 0u;
constexpr SceneId CACHE_SCENE = 1u;

using EntityId = unsigned int;
constexpr EntityId MAX_ENTITIES = 16384u;
constexpr EntityId EMPTY_ENTITY = MAX_ENTITIES + 1u;

using ComponentId = unsigned int;
constexpr ComponentId MAX_COMPONENTS = 32u;
constexpr ComponentId EMPTY_COMPONENT = MAX_COMPONENTS + 1u;

/* using Signature = std::bitset<MAX_COMPONENTS>; */
struct Signature {
    std::bitset<MAX_COMPONENTS> components;
    SceneId sceneId;
};

// ////////////////////////////////////////////////////////////////////////// //
