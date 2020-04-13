#include "Engine.hpp"
#include "Window.h"

#include <algorithm>
#include <memory>

#include "Systems/Systems.hpp"

Engine::Engine()
    : registry(Registry::instance()),
      systems{registry.system<RenderSystem>()} {}

int Engine::run() {
    for (auto &system : systems) {
        system->setup();
    }

    while (true) {
        auto const &deltaTime = timer.Mark();

        // process all messages pending, but to not block for new messages
        if (auto const exitCode = Window::ProcessMessages()) {
            return *exitCode;
        }

        for (auto &system : systems) {
            system->update(deltaTime);
        }
    }
}
