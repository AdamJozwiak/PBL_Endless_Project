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
void ComponentManager::destroyEntity(EntityId entityId) {
    for (auto const& [typeIndex, components] : componentArrays) {
        components->destroyEntity(entityId);
    }
}

// ------------------------------------------------------------ Helpers -- == //
bool ComponentManager::isComponentTypeRegistered(
    std::type_index const& typeIndex) {
    return componentIds.find(typeIndex) != componentIds.end();
}

// ////////////////////////////////////////////////////////////////////////// //
