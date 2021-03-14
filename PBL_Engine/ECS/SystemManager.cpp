// ///////////////////////////////////////////////////////////////// Includes //
#include "SystemManager.hpp"

#include "Entity.hpp"
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
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    for (auto const& [typeIndex, system] : systems) {
        if (auto entity = Entity{entityId};
            system->entities.find(entity) != system->entities.end()) {
            system->entities.erase(entity);
        }
    }
}

void SystemManager::changeEntitySignature(EntityId entityId,
                                          Signature const& entitySignature) {
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    for (auto const& [typeIndex, system] : systems) {
        auto const& systemSignature = signatures.at(typeIndex);
        // assert(systemSignature.any() &&
        //       "Each system must have a non-empty signature!");

        if ((entitySignature.components & systemSignature.components) ==
                systemSignature.components &&
            entitySignature.sceneId == DEFAULT_SCENE) {
            system->entities.insert(Entity(entityId));
        } else {
            system->entities.erase(Entity(entityId));
        }
    }
}

// ////////////////////////////////////////////////////////////////////////// //
