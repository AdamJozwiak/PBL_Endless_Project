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

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void RenderSystem::filters() { filter<Renderer>(); }

void RenderSystem::setup() {
    window = std::make_unique<Window>(800, 600, "PBL_ENGINE");
    camera = std::make_unique<Camera>();
    // imgui = std::make_unique<ImguiManager>();

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
