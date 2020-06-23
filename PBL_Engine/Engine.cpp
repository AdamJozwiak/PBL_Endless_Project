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
      setupSystems{registry.system<WindowSystem>(),
                   registry.system<SceneSystem>(),
                   registry.system<GraphSystem>(),
                   registry.system<AnimatorSystem>(),
                   registry.system<LightSystem>(),
                   registry.system<RenderSystem>(),
                   registry.system<BillboardRenderSystem>(),
                   registry.system<UIRenderSystem>(),
                   registry.system<BehaviourSystem>(),
                   registry.system<CheckCollisionsSystem>(),
                   registry.system<ColliderSystem>(),
                   registry.system<SoundSystem>(),
                   registry.system<PropertySystem>(),
                   registry.system<PhysicsSystem>()},
      updateSystems{
          registry.system<WindowSystem>(),
          registry.system<SceneSystem>(),
          registry.system<SoundSystem>(),
          registry.system<BehaviourSystem>(),
          registry.system<PhysicsSystem>(),
          registry.system<GraphSystem>(),
          registry.system<CheckCollisionsSystem>(),
          registry.system<ColliderSystem>(),
          registry.system<PropertySystem>(),
          registry.system<AnimatorSystem>(),
          registry.system<LightSystem>(),
          registry.system<RenderSystem>(),
          registry.system<BillboardRenderSystem>(),
          registry.system<UIRenderSystem>(),
      },
      releaseSystems{registry.system<WindowSystem>(),
                     registry.system<GraphSystem>(),
                     registry.system<AnimatorSystem>(),
                     registry.system<LightSystem>(),
                     registry.system<RenderSystem>(),
                     registry.system<BillboardRenderSystem>(),
                     registry.system<UIRenderSystem>(),
                     registry.system<SceneSystem>(),
                     registry.system<BehaviourSystem>(),
                     registry.system<CheckCollisionsSystem>(),
                     registry.system<ColliderSystem>(),
                     registry.system<SoundSystem>(),
                     registry.system<PropertySystem>(),
                     registry.system<PhysicsSystem>()} {}

ENGINE_API int Engine::run() {
    for (auto &system : setupSystems) {
        system->setup();
    }

    registry.listen<OnGameExit>(MethodListener(Engine::onGameExit));

    timer.Mark();

    while (runGameLoop) {
        if (registry.refresh()) {
            registry.system<GraphSystem>()->setup();
        }

        if ((exitCode = Window::ProcessMessages())) {
            break;
        }

        auto const &deltaTime = std::clamp(timer.Mark(), 0.0f, 1.0f / 30.0f);
        for (auto &system : updateSystems) {
            system->update(deltaTime);
        }
    }

    for (auto &system : releaseSystems) {
        system->release();
    }
    return *exitCode;
}

void Engine::onGameExit(OnGameExit const &event) { runGameLoop = false; }

// ////////////////////////////////////////////////////////////////////////// //
