// ///////////////////////////////////////////////////////////////// Includes //
#include "Engine.hpp"

#include <algorithm>
#include <memory>

#include "Systems/Systems.hpp"
#include "Window.h"

// ///////////////////////////////////////////////////////// Factory function //
extern "C" ENGINE_API void create(std::shared_ptr<Engine> &engine) {
    engine = std::make_shared<Engine>();
}

// //////////////////////////////////////////////////////////////////// Class //
// ============================================================= Behaviour == //
Engine::Engine()
    : registry(Registry::instance()),
      systems{registry.system<RenderSystem>(), registry.system<SceneSystem>(),
              registry.system<BehaviourSystem>()} {}

ENGINE_API int Engine::run() {
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