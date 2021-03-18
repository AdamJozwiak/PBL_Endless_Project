// ///////////////////////////////////////////////////////////////// Includes //
#include "ComponentManager.hpp"

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
// ---------------------------------------------------------- Singleton -- == //
ComponentManager& ComponentManager::instance() {
    static ComponentManager componentManager;
    return componentManager;
}

// ////////////////////////////////////////////////////////////////////////// //
