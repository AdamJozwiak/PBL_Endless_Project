// ///////////////////////////////////////////////////////////////// Includes //
#include "SceneSystem.hpp"

#include <algorithm>
#include <memory>

#include "BehaviourSystem.hpp"
#include "Box.h"
#include "Melon.h"
#include "PBLMath.h"
#include "Pyramid.h"
#include "RenderSystem.hpp"
#include "Script.hpp"
#include "Sheet.h"
#include "SkinnedBox.h"
#include "Surface.h"
#include "Window.h"

// ECS
#include "ECS/ECS.hpp"

// ////////////////////////////////////////////////////////////////////////// //
class Factory {
  public:
    Factory(Graphics& gfx) : gfx(gfx) {}
    std::shared_ptr<Renderable> operator()() {
        switch (typedist(rng)) {
            case 0:
                return std::make_shared<Pyramid>(gfx, rng, adist, ddist, odist,
                                                 rdist);
            case 1:
                return std::make_shared<Box>(gfx, rng, adist, ddist, odist,
                                             rdist, bdist);
            case 2:
                return std::make_shared<Melon>(gfx, rng, adist, ddist, odist,
                                               rdist, longdist, latdist);
            case 3:
                return std::make_shared<Sheet>(gfx, rng, adist, ddist, odist,
                                               rdist);
            case 4:
                return std::make_shared<SkinnedBox>(gfx, rng, adist, ddist,
                                                    odist, rdist);
            default:
                assert(false && "Bad renderable type in factory");
                return {};
        }
    }

  private:
    Graphics& gfx;
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> adist{0.0f, PI * 2.0f};
    std::uniform_real_distribution<float> ddist{0.0f, PI * 0.5f};
    std::uniform_real_distribution<float> odist{0.0f, PI * 0.08f};
    std::uniform_real_distribution<float> rdist{6.0f, 20.0f};
    std::uniform_real_distribution<float> bdist{0.4f, 3.0f};
    std::uniform_int_distribution<int> latdist{5, 20};
    std::uniform_int_distribution<int> longdist{10, 40};
    std::uniform_int_distribution<int> typedist{0, 4};
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
