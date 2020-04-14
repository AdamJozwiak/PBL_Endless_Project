// ///////////////////////////////////////////////////////////////// Includes //
#include "Engine.hpp"

#include <algorithm>
#include <memory>

#include "Systems/Systems.hpp"
#include "Window.h"

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
Engine::Engine()
    : registry(Registry::instance()),
      systems{registry.system<RenderSystem>(), registry.system<SceneSystem>()} {
}

int Engine::run() {
    for (auto &system : systems) {
        system->setup();
    }

    while (true) {
        if (auto const exitCode = Window::ProcessMessages()) {
            return *exitCode;
        }

        auto const &deltaTime = timer.Mark();
        for (auto &system : systems) {
            system->update(deltaTime);
        }
    }
}

// ////////////////////////////////////////////////////////////////////////// //
