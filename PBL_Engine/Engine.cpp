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
      setupSystems{
          registry.system<SceneSystem>(),    registry.system<GraphSystem>(),
          registry.system<RenderSystem>(),   registry.system<BehaviourSystem>(),
          registry.system<ColliderSystem>(), registry.system<SoundSystem>(),
          registry.system<PropertySystem>()},
      updateSystems{
          registry.system<SceneSystem>(),     registry.system<GraphSystem>(),
          registry.system<SoundSystem>(),     registry.system<ColliderSystem>(),
          registry.system<BehaviourSystem>(), registry.system<PropertySystem>(),
          registry.system<RenderSystem>()},
      releaseSystems{
          registry.system<GraphSystem>(),    registry.system<RenderSystem>(),
          registry.system<SceneSystem>(),    registry.system<BehaviourSystem>(),
          registry.system<ColliderSystem>(), registry.system<SoundSystem>(),
          registry.system<PropertySystem>()} {}

ENGINE_API int Engine::run() {
    for (auto &system : setupSystems) {
        system->setup();
    }

    while (true) {
        registry.refresh();

        if (auto const exitCode = Window::ProcessMessages()) {
            for (auto &system : updateSystems) {
                system->release();
            }
            return *exitCode;
        }

        auto const &deltaTime = timer.Mark();
        for (auto &system : releaseSystems) {
            system->update(deltaTime);
        }
    }
}

// ////////////////////////////////////////////////////////////////////////// //
