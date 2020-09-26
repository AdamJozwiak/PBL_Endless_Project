// ///////////////////////////////////////////////////////////////// Includes //
#include "ComponentManager.hpp"

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
// ---------------------------------------------------------- Singleton -- == //
ComponentManager& ComponentManager::instance() {
    static ComponentManager componentManager;
    return componentManager;
}

// ------------------------------------------------------------ Helpers -- == //
void ComponentManager::destroyEntity(EntityId const entityId) {
    std::lock_guard<std::recursive_mutex> lockGuard{mutex};

    for (auto const& components : componentArrays) {
        if (components) {
            components->destroyEntity(entityId);
        }
    }
}

// ////////////////////////////////////////////////////////////////////////// //
