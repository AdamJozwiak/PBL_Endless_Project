// ///////////////////////////////////////////////////////////////// Includes //
#include "RenderSystem.hpp"

#include <algorithm>
#include <memory>

#include "Box.h"
#include "GDIPlusManager.h"
#include "Melon.h"
#include "PBLMath.h"
#include "Pyramid.h"
#include "Sheet.h"
#include "SkinnedBox.h"
#include "Surface.h"
#include "Window.h"
#include "imgui/imgui.h"

// ECS
#include "ECS/ECS.hpp"

// Assimp
//#include "assimp/include/assimp/Importer.hpp"
//#include "assimp/include/assimp/postprocess.h"
//#include "assimp/include/assimp/scene.h"

namespace dx = DirectX;

GDIPlusManager gdipm;

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
void RenderSystem::filters() { filter<Renderer>(); }

void RenderSystem::setup() {
    window = std::make_unique<Window>(800, 600, "PBL_ENGINE");
    camera = std::make_unique<Camera>();
    // imgui = std::make_unique<ImguiManager>();

    Factory renderableFactory(window->Gfx());

    constexpr size_t NUMBER_OF_RENDERABLES = 18;
    for (int i = 0; i < NUMBER_OF_RENDERABLES; ++i) {
        registry.createEntity().add<Renderer>(
            {.renderable = renderableFactory()});
    }

    window->Gfx().SetProjection(
        dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
    // Assimp::Importer imp;
    // auto model = imp.ReadFile("Models\\suzanne.obj", aiProcess_Triangulate |
    // aiProcess_JoinIdenticalVertices);
}

void RenderSystem::update(float deltaTime) {
    const auto dt = deltaTime * speed_factor;
    window->Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
    window->Gfx().SetCamera(camera->GetMatrix());

    // window->Gfx().ClearBuffer(0.07f, 0.0f, 0.12f);

    for (Entity entity : entities) {
        auto& renderer = entity.get<Renderer>();

        renderer.renderable->Update(
            window->keyboard.KeyIsPressed(VK_SPACE) ? 0.0f : dt);
        renderer.renderable->Draw(window->Gfx());
    }

    // imgui window to control simulation speed
    if (ImGui::Begin("Simulation Speed")) {
        ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 4.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
    }
    ImGui::End();

    // imgui window to control camera
    camera->SpawnControlWindow();

    // present
    window->Gfx().EndFrame();
};

// ////////////////////////////////////////////////////////////////////////// //
