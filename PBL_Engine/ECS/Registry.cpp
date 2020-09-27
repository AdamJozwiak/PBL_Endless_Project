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
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

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

// -------------------------------------------------------------- Cache -- == //
void Registry::moveCacheToMainScene() {
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    for (auto const& entity : cachedEntities) {
        auto signature = entityManager.getSignature(entity.id);
        signature.sceneId = DEFAULT_SCENE;
        entityManager.setSignature(entity.id, signature);

        systemManager.changeEntitySignature(entity.id, signature);
    }
}

void Registry::clearCache() {
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    for (auto const& entity : cachedEntities) {
        destroyEntity(entity);
    }
}

// ------------------------------------------------------------- Entity -- == //
Entity Registry::createEntity(SceneId const sceneId) {
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    auto const& entity = entityManager.create(sceneId);
    if (sceneId == CACHE_SCENE) {
        cachedEntities.insert(entity);
    }
    return entity;
}

void Registry::destroyEntity(Entity const& entity) {
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    if (cachedEntities.contains(entity)) {
        cachedEntities.erase(entity);
    }
    entitiesToRemove.push_back(entity);
}

// ---------------------------------------------------------- Singleton -- == //
Registry::Registry()
    : componentManager(ComponentManager::instance()),
      entityManager(EntityManager::instance()),
      eventManager(EventManager::instance()),
      systemManager(SystemManager::instance()) {}

// ////////////////////////////////////////////////////////////////////////// //
