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
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

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
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    assert((entityId >= 0 && entityId < MAX_ENTITIES) &&
           "Entity identifier must be in range [0, MAX_ENTITIES)!");

    signatures.at(entityId).components.reset();
    signatures.at(entityId).sceneId = EMPTY_SCENE;
    availableIdentifiers.push(entityId);
}

void EntityManager::setSignature(EntityId entityId,
                                 Signature const& signature) {
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    assert((entityId >= 0 && entityId < MAX_ENTITIES) &&
           "Entity identifier must be in range [0, MAX_ENTITIES)!");

    signatures.at(entityId) = signature;
}

Signature EntityManager::getSignature(EntityId entityId) {
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    assert((entityId >= 0 && entityId < MAX_ENTITIES) &&
           "Entity identifier must be in range [0, MAX_ENTITIES)!");

    return signatures.at(entityId);
}

// ---------------------------------------------------------- Singleton -- == //
EntityManager::EntityManager() {
    for (EntityId id = 0; id < MAX_ENTITIES; ++id) {
        availableIdentifiers.push(id);
        signatures.at(id).sceneId = EMPTY_SCENE;
    }
}

// ////////////////////////////////////////////////////////////////////////// //
