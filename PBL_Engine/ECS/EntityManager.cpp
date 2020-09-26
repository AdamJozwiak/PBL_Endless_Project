// ///////////////////////////////////////////////////////////////// Includes //
#include "EntityManager.hpp"

#include <cassert>

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
// ---------------------------------------------------------- Singleton -- == //
EntityManager& EntityManager::instance() {
    static EntityManager entityManager;
    return entityManager;
}

// ------------------------------------------------- Main functionality -- == //
EntityId EntityManager::create(SceneId const sceneId) {
    assert((sceneId >= 0 && sceneId < MAX_SCENES) &&
           "Entity identifier must be in range [0, MAX_SCENES)!");
    assert(availableIdentifiers.size() > 0 && "Cannot create more entities!");

    EntityId entityId = availableIdentifiers.front();
    availableIdentifiers.pop();

    auto signature = getSignature(entityId);
    signature.sceneId = sceneId;
    setSignature(entityId, signature);

    return entityId;
}

void EntityManager::destroy(EntityId entityId) {
    assert((entityId >= 0 && entityId < MAX_ENTITIES) &&
           "Entity identifier must be in range [0, MAX_ENTITIES)!");

    signatures[entityId].components.reset();
    signatures[entityId].sceneId = EMPTY_SCENE;
    availableIdentifiers.push(entityId);
}

void EntityManager::setSignature(EntityId entityId,
                                 Signature const& signature) {
    assert((entityId >= 0 && entityId < MAX_ENTITIES) &&
           "Entity identifier must be in range [0, MAX_ENTITIES)!");

    signatures[entityId] = signature;
}

Signature EntityManager::getSignature(EntityId entityId) {
    assert((entityId >= 0 && entityId < MAX_ENTITIES) &&
           "Entity identifier must be in range [0, MAX_ENTITIES)!");

    return signatures[entityId];
}

// ---------------------------------------------------------- Singleton -- == //
EntityManager::EntityManager() {
    for (EntityId id = 0; id < MAX_ENTITIES; ++id) {
        availableIdentifiers.push(id);
        signatures[id].sceneId = EMPTY_SCENE;
    }
}

// ////////////////////////////////////////////////////////////////////////// //
