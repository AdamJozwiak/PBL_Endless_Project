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
Engine::Engine() : registry(Registry::instance()) {
    ECS_REGISTER_COMPONENT(AABB);
    ECS_REGISTER_COMPONENT(Animator);
    ECS_REGISTER_COMPONENT(Behaviour);
    ECS_REGISTER_COMPONENT(BoxCollider);
    ECS_REGISTER_COMPONENT(Flame);
    ECS_REGISTER_COMPONENT(Light);
    ECS_REGISTER_COMPONENT(MainCamera);
    ECS_REGISTER_COMPONENT(MeshFilter);
    ECS_REGISTER_COMPONENT(Properties);
    ECS_REGISTER_COMPONENT(RectTransform);
    ECS_REGISTER_COMPONENT(Renderer);
    ECS_REGISTER_COMPONENT(Rigidbody);
    ECS_REGISTER_COMPONENT(Skybox);
    ECS_REGISTER_COMPONENT(SphereCollider);
    ECS_REGISTER_COMPONENT(Active);
    ECS_REGISTER_COMPONENT(Refractive);
    ECS_REGISTER_COMPONENT(CheckCollisions);
    ECS_REGISTER_COMPONENT(Transform);
    ECS_REGISTER_COMPONENT(UIElement);

    ECS_REGISTER_SYSTEM(AnimatorSystem);
    ECS_REGISTER_SYSTEM(BehaviourSystem);
    ECS_REGISTER_SYSTEM(BillboardRenderSystem);
    ECS_REGISTER_SYSTEM(CheckCollisionsSystem);
    ECS_REGISTER_SYSTEM(ColliderSystem);
    ECS_REGISTER_SYSTEM(GraphSystem);
    ECS_REGISTER_SYSTEM(LightSystem);
    ECS_REGISTER_SYSTEM(PhysicsSystem);
    ECS_REGISTER_SYSTEM(PropertySystem);
    ECS_REGISTER_SYSTEM(RenderSystem);
    ECS_REGISTER_SYSTEM(SceneSystem);
    ECS_REGISTER_SYSTEM(SoundSystem);
    ECS_REGISTER_SYSTEM(UIRenderSystem);
    ECS_REGISTER_SYSTEM(WindowSystem);

    setupSystems = {registry.system<WindowSystem>(),
                    registry.system<GraphSystem>(),
                    registry.system<SceneSystem>(),
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
                    registry.system<PhysicsSystem>()};
    updateSystems = {
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
    };
    releaseSystems = {registry.system<WindowSystem>(),
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
                      registry.system<PhysicsSystem>()};
}

ENGINE_API int Engine::run() {
    for (auto &system : setupSystems) {
        system->setup();
    }

    registry.listen<OnGameExit>(MethodListener(Engine::onGameExit));

    timer.Mark();

    while (runGameLoop) {
        registry.refresh();

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
