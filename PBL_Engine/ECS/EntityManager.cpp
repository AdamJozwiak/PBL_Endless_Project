// ///////////////////////////////////////////////////////////////// Includes //
#include "EntityManager.hpp"

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
// ---------------------------------------------------------- Singleton -- == //
EntityManager& EntityManager::instance() {
    static EntityManager entityManager;
    return entityManager;
}

// ---------------------------------------------------------- Singleton -- == //
EntityManager::EntityManager() {
    for (EntityId id = 0; id < MAX_ENTITIES; ++id) {
        availableIdentifiers.push(id);
        signatures.at(id).sceneId = EMPTY_SCENE;
    }
}

// ////////////////////////////////////////////////////////////////////////// //
