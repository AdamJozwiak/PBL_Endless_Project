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

// ------------------------------------------------------------- Entity -- == //
Entity Registry::createEntity() { return entityManager.create(); }

void Registry::destroyEntity(Entity const& entity) {
    entityManager.destroy(entity.id);
    componentManager.destroyEntity(entity.id);
    systemManager.destroyEntity(entity.id);
}

// ---------------------------------------------------------- Singleton -- == //
Registry::Registry()
    : componentManager(ComponentManager::instance()),
      entityManager(EntityManager::instance()),
      eventManager(EventManager::instance()),
      systemManager(SystemManager::instance()) {}

// ////////////////////////////////////////////////////////////////////////// //
