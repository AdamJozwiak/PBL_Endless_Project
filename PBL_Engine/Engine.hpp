#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <vector>

#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include "Timer.h"
#include "Window.h"

// //////////////////////////////////////////////////////////////////// Class //
class Engine {
  public:
    // ========================================================= Behaviour == //
    Engine();
    int run();

  private:
    // ============================================================== Data == //
    Registry &registry;
    std::vector<std::shared_ptr<System>> systems;

    Timer timer;
};

// ////////////////////////////////////////////////////////////////////////// //
