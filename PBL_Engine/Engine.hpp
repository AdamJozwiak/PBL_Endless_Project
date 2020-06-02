#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <vector>

#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include "EngineAPI.hpp"
#include "Timer.h"
#include "Window.h"

// //////////////////////////////////////////////////////////////////// Class //
class Engine {
  public:
    // ========================================================= Behaviour == //
    Engine();
    ENGINE_API int run();

  private:
    // ============================================================== Data == //
    Registry &registry;
    std::vector<std::shared_ptr<System>> setupSystems, updateSystems,
        releaseSystems;

    Timer timer;
};

// ////////////////////////////////////////////////////////////////////////// //
