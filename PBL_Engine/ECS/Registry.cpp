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

// ------------------------------------------------------------- Entity -- == //
Entity Registry::createEntity() { return entityManager.create(); }

void Registry::destroyEntity(Entity const& entity) {
    entitiesToRemove.push_back(entity);
}

// ---------------------------------------------------------- Singleton -- == //
Registry::Registry()
    : componentManager(ComponentManager::instance()),
      entityManager(EntityManager::instance()),
      eventManager(EventManager::instance()),
      systemManager(SystemManager::instance()) {}

// ////////////////////////////////////////////////////////////////////////// //
