#pragma once

#include <vector>

#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include "Timer.h"
#include "Window.h"

class Engine {
  public:
    Engine();
    int run();

  private:
    Timer timer;

    Registry &registry;
    std::vector<std::shared_ptr<System>> systems;
};
