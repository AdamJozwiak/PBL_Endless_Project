// ///////////////////////////////////////////////////////////////// Includes //
#include "Registry.hpp"

#include "Entity.hpp"

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
// ---------------------------------------------------------- Singleton -- == //
Registry& Registry::instance() {
    static Registry registry;
    return registry;
}

// -------------------------------------------- Delayed entity deletion -- == //
bool Registry::refresh() {
    LOCK_GUARD(registryMutex, true);

    bool removed = false;
    for (auto const& entity : entitiesToRemove) {
        entityManager.destroy(entity.id);
        componentManager.destroyEntity(entity.id);
        systemManager.destroyEntity(entity.id);
        removed = true;
    }
    entitiesToRemove.clear();
    return removed;
}

// ------------------------------------------------------ Thread safety -- == //
void Registry::setThreadSafety(bool const status) {
    LOCK_GUARD(registryMutex, true);

    threadSafety = status;
}

// -------------------------------------------------------------- Cache -- == //
void Registry::moveCacheToMainScene() {
    LOCK_GUARD(registryMutex, true);

    for (auto const& entity : cachedEntities) {
        auto signature = entityManager.getSignature(entity.id);
        signature.sceneId = DEFAULT_SCENE;
        entityManager.setSignature(entity.id, signature);

        systemManager.changeEntitySignature(entity.id, signature);
    }
}

void Registry::clearCache() {
    LOCK_GUARD(registryMutex, true);

    for (auto const& entity : cachedEntities) {
        destroyEntity<false>(entity);
    }
}

// ------------------------------------------------------------- Entity -- == //
Entity Registry::createEntity(SceneId const sceneId) {
    LOCK_GUARD(registryMutex, true);

    auto const& entity = entityManager.create(sceneId);
    if (sceneId == CACHE_SCENE) {
        cachedEntities.insert(entity);
    }
    return entity;
}

// ---------------------------------------------------------- Singleton -- == //
Registry::Registry()
    : componentManager(ComponentManager::instance()),
      entityManager(EntityManager::instance()),
      eventManager(EventManager::instance()),
      systemManager(SystemManager::instance()),
      threadSafety(false) {}

// ////////////////////////////////////////////////////////////////////////// //
