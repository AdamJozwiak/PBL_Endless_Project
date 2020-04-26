// ///////////////////////////////////////////////////////////////// Includes //
#include "RenderSystem.hpp"

#include <DirectXMath.h>

#include <algorithm>
#include <memory>

#include "Box.h"
#include "GDIPlusManager.h"
#include "Mesh.h"
#include "PBLMath.h"
#include "SolidSphere.h"
#include "Surface.h"
#include "Window.h"
#include "imgui/imgui.h"

// ECS
#include "ColliderSystem.hpp"
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
    sphere = std::make_unique<SolidSphere>(window->Gfx(), 1.0f);
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

    static bool showColliders = false;
    for (auto entity : registry.system<ColliderSystem>()->entities) {
        auto& transform = entity.get<Transform>();
        auto& sphereCollider = entity.get<SphereCollider>();

        DirectX::XMFLOAT4 center;
        DirectX::XMStoreFloat4(&center, sphereCollider.objectCenterOffset);

        sphere->SetPos({center.x + transform.position.x,
                        center.y + transform.position.y,
                        center.z + transform.position.z});
        sphere->scale = 3 * transform.scale.x * sphereCollider.radius;
        if (!showColliders) {
            sphere->scale = 0.0f;
        }
        sphere->Draw(window->Gfx());
    }

    // test->Draw(window->Gfx());
    /* nano->Draw(window->Gfx(),
               dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(270.0f),
                                                dx::XMConvertToRadians(180.0f),
                                                dx::XMConvertToRadians(0.0f))); */
    light->Draw(window->Gfx());

    if (ImGui::Begin("Colliders")) {
        if (ImGui::Button("Dummies")) {
            showColliders = !showColliders;
        }
    }
    ImGui::End();

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
