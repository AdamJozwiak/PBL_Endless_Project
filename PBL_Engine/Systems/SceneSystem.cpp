// ///////////////////////////////////////////////////////////////// Includes //
#include "SceneSystem.hpp"

#include <algorithm>
#include <memory>

#include "BehaviourSystem.hpp"
#include "Model.h"
#include "Box.h"
#include "PBLMath.h"
#include "RenderSystem.hpp"
#include "Script.hpp"
#include "Surface.h"
#include "Window.h"

// ECS
#include "ECS/ECS.hpp"

// ////////////////////////////////////////////////////////////////////////// //
class Factory {
  public:
    Factory(Graphics& gfx) : gfx(gfx){}
    std::shared_ptr<Renderable> operator()() {
        const DirectX::XMFLOAT3 mat = {cdist(rng), cdist(rng), cdist(rng)};
        return std::make_shared<Box>(gfx, rng, adist, ddist, odist,
                                        rdist, bdist, mat);
    }

  private:
    Graphics& gfx;
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> adist{0.0f, PI * 2.0f};
    std::uniform_real_distribution<float> ddist{0.0f, PI * 0.5f};
    std::uniform_real_distribution<float> odist{0.0f, PI * 0.08f};
    std::uniform_real_distribution<float> rdist{6.0f, 20.0f};
    std::uniform_real_distribution<float> bdist{0.4f, 3.0f};
    std::uniform_real_distribution<float> cdist{0.0f, 1.0f};
};

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void SceneSystem::filters() {}

void SceneSystem::setup() {
    auto renderSystem = registry.system<RenderSystem>();
    auto behaviourSystem = registry.system<BehaviourSystem>();

    Factory renderableFactory(renderSystem->window->Gfx());

    constexpr size_t NUMBER_OF_RENDERABLES = 18;
    for (int i = 0; i < NUMBER_OF_RENDERABLES; ++i) {
        auto entity = registry.createEntity();
        entity.add<Renderer>({.renderable = renderableFactory()})
            .add<Behaviour>(behaviourSystem->behaviour(
                "TestScript" + (i % 2 ? std::string("2") : std::string("")),
                entity));
    }

    for (auto entity : entities) {
        entity.get<Behaviour>().script->setup();
    }
}

void SceneSystem::update(float deltaTime){};

// ////////////////////////////////////////////////////////////////////////// //
