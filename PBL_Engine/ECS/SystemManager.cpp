// ///////////////////////////////////////////////////////////////// Includes //
#include "SystemManager.hpp"

#include "Entity.hpp"
#include "EntityManager.hpp"
#include "System.hpp"
#include "assert.hpp"

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
// ---------------------------------------------------------- Singleton -- == //
SystemManager& SystemManager::instance() {
    static SystemManager systemManager;
    return systemManager;
}

void SystemManager::destroyEntity(EntityId entityId) {
    RECURSIVE_LOCK_GUARD(systemManagerRecursiveMutex, true);

    for (auto const& [typeIndex, system] : systems) {
        if (auto entity = Entity{entityId};
            system->entities.find(entity) != system->entities.end()) {
            if (system->entities.erase(entity)) {
                system->onEntityErase(
                    entity,
                    EntityManager::instance().getSignature(entityId).sceneId);
            }
        }
    }
}

void SystemManager::changeEntitySignature(EntityId entityId,
                                          Signature const& entitySignature) {
    RECURSIVE_LOCK_GUARD(systemManagerRecursiveMutex, true);

    for (auto const& [typeIndex, system] : systems) {
        auto const& systemSignature = signatures.at(typeIndex);
        // assert(systemSignature.any() &&
        //       "Each system must have a non-empty signature!");

        if ((entitySignature.components & systemSignature.components) ==
                systemSignature.components &&
            entitySignature.sceneId == DEFAULT_SCENE) {
            if (system->entities.insert(entityId).second) {
                system->onEntityInsert(entityId, entitySignature.sceneId);
            }
        } else {
            if (system->entities.erase(entityId)) {
                system->onEntityErase(entityId, entitySignature.sceneId);
            }
        }
    }
}

// ////////////////////////////////////////////////////////////////////////// //
