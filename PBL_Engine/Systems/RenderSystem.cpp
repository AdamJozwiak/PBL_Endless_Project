// ///////////////////////////////////////////////////////////////// Includes //
#include "RenderSystem.hpp"

#include <algorithm>
#include <memory>

#include "Box.h"
#include "GDIPlusManager.h"
#include "Mesh.h"
#include "PBLMath.h"
#include "Surface.h"
#include "Window.h"
#include "imgui/imgui.h"

// ECS
#include "ECS/ECS.hpp"

namespace dx = DirectX;

GDIPlusManager gdipm;

// testing models
Model* nano;
Model* test;
PointLight* light;

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void RenderSystem::filters() {
    filter<Renderer>().filter<MeshFilter>().filter<Transform>();
}

void RenderSystem::setup() {
    window = std::make_unique<Window>(1280, 720, "PBL_ENGINE");
    camera = std::make_unique<Camera>();
    light = new PointLight(window->Gfx());

    // imgui = std::make_unique<ImguiManager>();
    test = new Model(window->Gfx(), "Models\\suzanne.obj");
    nano = new Model(window->Gfx(), "Models\\nano.gltf");
    window->Gfx().SetProjection(
        dx::XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
}

void RenderSystem::update(float deltaTime) {
    const auto dt = deltaTime * speed_factor;
    window->Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
    window->Gfx().SetCamera(camera->GetMatrix());
    light->Bind(window->Gfx(), camera->GetMatrix());

    // window->Gfx().ClearBuffer(0.07f, 0.0f, 0.12f);

    for (Entity entity : entities) {
        auto& renderer = entity.get<Renderer>();
        auto& meshFilter = entity.get<MeshFilter>();
        auto& transform = entity.get<Transform>();

        // renderer.renderable->Update(
        //     window->keyboard.KeyIsPressed(VK_SPACE) ? 0.0f : dt);
        // renderer.renderable->Draw(window->Gfx());
        meshFilter.model->Draw(window->Gfx(), transformMatrix(entity));
    }
    // test->Draw(window->Gfx());
    // nano->Draw(window->Gfx());
    light->Draw(window->Gfx());

    // imgui window to control simulation speed
    if (ImGui::Begin("Simulation Speed")) {
        ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 6.0f, "%.4f",
                           3.2f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
    }
    ImGui::End();

    // imgui window to control camera
    camera->SpawnControlWindow();
    light->SpawnControlWindow();
    // nano->ShowWindow();
    // test->ShowWindow();

    // present
    window->Gfx().EndFrame();
};

DirectX::XMMATRIX RenderSystem::transformMatrix(Entity entity) {
    auto& transform = entity.get<Transform>();

    return dx::XMMatrixScaling(transform.scale.x, transform.scale.y,
                               transform.scale.z) *
           dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(270.0f),
                                            dx::XMConvertToRadians(180.0f),
                                            dx::XMConvertToRadians(0.0f)) *
           dx::XMMatrixRotationRollPitchYaw(
               dx::XMConvertToRadians(transform.euler.x),
               dx::XMConvertToRadians(transform.euler.y),
               dx::XMConvertToRadians(transform.euler.z)) *
           // dx::XMMatrixRotationQuaternion(dx::XMLoadFloat4(
           //     &dx::XMFLOAT4{transform.rotation_x, transform.rotation_y,
           //                   transform.rotation_z, transform.rotation_w})) *
           dx::XMMatrixTranslation(transform.position.x, transform.position.y,
                                   transform.position.z);
}

// ////////////////////////////////////////////////////////////////////////// //
