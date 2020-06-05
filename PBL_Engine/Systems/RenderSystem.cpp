// ///////////////////////////////////////////////////////////////// Includes //
#include "RenderSystem.hpp"

#include <DirectXMath.h>

#include <Systems/PropertySystem.hpp>
#include <algorithm>
#include <cmath>
#include <memory>

#include "Button.hpp"
#include "FrustumCulling.h"
#include "GDIPlusManager.h"
#include "Mesh.h"
#include "PBLMath.h"
#include "Text.h"
#include "Window.h"
#include "imgui/imgui.h"

// ECS
#include "ColliderSystem.hpp"
#include "ECS/ECS.hpp"
#include "GraphSystem.hpp"
#include "SoundSystem.hpp"
#include "WindowSystem.hpp"

namespace dx = DirectX;

GDIPlusManager gdipm;

// testing models
Model* nano;
Animator animator;
PointLight* light;
PointLight* light2;
std::unique_ptr<Button> button;
std::unique_ptr<Text> text;

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void RenderSystem::filters() {
    filter<Active>()
        .filter<Renderer>()
        .filter<MeshFilter>()
        .filter<Transform>()
        .filter<AABB>();
}

void RenderSystem::setup() {
    window = &registry.system<WindowSystem>()->window();
    mainCamera = registry.system<PropertySystem>()
                     ->findEntityByTag("MainCamera")
                     .at(0)
                     .get<MainCamera>()
                     .camera;
    freeCamera = std::make_shared<Camera>();
    sphere = std::make_unique<SolidSphere>(window->Gfx(), 1.0f);
    billboard = std::make_unique<Billboard>(window->Gfx(), mainCamera.get());
    fireParticle =
        std::make_unique<FireParticle>(window->Gfx(), mainCamera.get());
    light = new PointLight(window->Gfx(), 0);
    light2 = new PointLight(window->Gfx(), 1);
    bloom = std::make_unique<PostProcessing>(window->Gfx(), L"Bloom", 2);
    colorCorrection =
        std::make_unique<PostProcessing>(window->Gfx(), L"ColorCorrection", 1);
    animator.animationTime = 0;
    // imgui = std::make_unique<ImguiManager>();
    nano = new Model(window->Gfx(), "Assets\\Models\\Wolf-Blender-2.82a.gltf",
                     nullptr, &animator.animationTime);
    text = std::make_unique<Text>(window->Gfx(), L"Western Samurai",
                                  L"Assets\\Fonts\\Western Samurai.otf", 20);
    button = std::make_unique<Button>(
        *window, L"Western Samurai", L"Assets\\Fonts\\Western Samurai.otf", 40,
        DirectX::XMFLOAT2{50.0f, 50.0f}, DirectX::XMFLOAT2{375.0f, 40.0f});
    window->Gfx().SetProjection(
        dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(60.0f),
                                     float(window->Gfx().GetWindowWidth()) /
                                         float(window->Gfx().GetWindowHeight()),
                                     0.3f, 1000.0f));
}

void RenderSystem::update(float deltaTime) {
    // Update AABB
    for (Entity entity : entities) {
        registry.system<ColliderSystem>()->CalculateAABB(
            entity.get<AABB>(),
            registry.system<GraphSystem>()->transform(entity));
    }

    const auto dt = deltaTime * speed_factor;
    window->Gfx().BeginFrame(0.0f, 0.0f, 0.0f);
    int a = 0;
    bloom->Begin();
    {
        // Process mouse movements for free camera
        while (auto const delta = window->mouse.ReadRawDelta()) {
            if (window->mouse.RightIsPressed()) {
                freeCamera->yaw += delta->x * 0.25f * deltaTime;
                freeCamera->pitch += delta->y * 0.25f * deltaTime;
            }
        }
        // Process input for free camera movement
        if (window->mouse.RightIsPressed()) {
            static float speed = 0.0f;
            float const targetSpeed = 5.0f * deltaTime;

            speed = std::lerp(speed, targetSpeed, 0.1f);  // Accelerate
            speed = std::lerp(0.0f, speed, 0.9f);         // Slow down

            if (window->keyboard.KeyIsPressed('A')) {
                freeCamera->moveLeftRight -= speed;
            }
            if (window->keyboard.KeyIsPressed('D')) {
                freeCamera->moveLeftRight += speed;
            }
            if (window->keyboard.KeyIsPressed('W')) {
                freeCamera->moveBackForward += speed;
            }
            if (window->keyboard.KeyIsPressed('S')) {
                freeCamera->moveBackForward -= speed;
            }
        }

        // Set camera
        registry.system<SoundSystem>()->setListener(
            mainCamera->pos(), mainCamera->at(), {0.0f, 1.0f, 0.0f});

        auto const& mainCameraTransform = registry.system<PropertySystem>()
                                              ->findEntityByTag("MainCamera")
                                              .at(0)
                                              .get<Transform>();
        mainCamera->setCameraPos(mainCameraTransform.position);

        window->Gfx().SetCamera(mainCamera->GetMatrix(mainCameraTransform));
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMStoreFloat4x4(&viewProjection,
                                 mainCamera->GetMatrix(mainCameraTransform) *
                                     window->Gfx().GetProjection());

        auto frustum = CFrustum(viewProjection);

        // Set lights
        light->AddToBuffer(DirectX::XMMatrixIdentity(),
                           mainCamera->GetCameraPos());
        light2->AddToBuffer(DirectX::XMMatrixIdentity(),
                            mainCamera->GetCameraPos());
        PointLight::Bind(window->Gfx());

        // Advance the animation time
        animator.animationTime += dt;

        // Render all renderable models
        for (auto const& entity : entities) {
            auto div = DirectX::XMVectorSubtract(entity.get<AABB>().vertexMax,
                                                 entity.get<AABB>().vertexMin);
            auto avg = DirectX::XMVectorScale(div, 0.5f);

            DirectX::XMFLOAT3 center;
            DirectX::XMStoreFloat3(
                &center,
                DirectX::XMVectorAdd(avg, entity.get<AABB>().vertexMin));
            DirectX::XMFLOAT3 radius;
            DirectX::XMStoreFloat3(&radius, DirectX::XMVector3Length(avg));

            if (frustum.SphereIntersection(center, radius.x)) {
                auto& meshFilter = entity.get<MeshFilter>();

                meshFilter.model->Draw(
                    window->Gfx(),
                    registry.system<GraphSystem>()->transform(entity));
                a++;
            }
        }

        // Show colliders
        static bool showColliders = false;
        // for (auto const& entity :
        // registry.system<ColliderSystem>()->entities) {
        //     auto& transform = entity.get<Transform>();
        //     auto& sphereCollider = entity.get<SphereCollider>();

        //     DirectX::XMFLOAT4 center;
        //     DirectX::XMStoreFloat4(&center,
        //     sphereCollider.objectCenterOffset);

        //     sphere->SetPos({center.x + transform.position.x,
        //                     center.y + transform.position.y,
        //                     center.z + transform.position.z});
        //     sphere->scale = 3 * transform.scale.x * sphereCollider.radius;
        //     if (!showColliders) {
        //         sphere->scale = 0.0f;
        //     }
        //     sphere->Draw(window->Gfx());
        // }

        // Render billboards
        billboard->Draw(window->Gfx());
        fireParticle->pos = light->lightPositionWorld();
        fireParticle->Draw(window->Gfx());

        // Render other test models
        nano->Draw(window->Gfx(),
                   dx::XMMatrixScaling(10.0f, 10.0f, 10.0f) *
                       dx::XMMatrixTranslation(0.0f, -2.5f, 7.5f));

        // Render light dummy
        // light->Draw(window->Gfx());

        // Render interface
        // {
        //     if (ImGui::Begin("Colliders")) {
        //         if (ImGui::Button("Dummies")) {
        //             showColliders = !showColliders;
        //         }
        //     }
        //     ImGui::End();
        // }
        {
            if (ImGui::Begin("Simulation Speed")) {
                ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 10.0f,
                                   "%.4f", 3.2f);
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                            1000.0f / ImGui::GetIO().Framerate,
                            ImGui::GetIO().Framerate);
            }
            ImGui::End();
        }
        light->SpawnControlWindow();
        nano->ShowWindow();
    }
    bloom->End();

    colorCorrection->Begin();
    { bloom->Draw(window->Gfx()); }
    colorCorrection->End();

    {
        window->Gfx().BeginFrame(0.07f, 0.0f, 0.12f, false);
        colorCorrection->Draw(window->Gfx());
    }

    text->RenderText(registry.system<RenderSystem>()->window->Gfx(),
                     "Visible entities: " + std::to_string(a), false,
                     {0.0f, 0.0f});
    button->draw("Create new monkey");
    window->Gfx().EndFrame();
};

void RenderSystem::release() {}

DirectX::XMMATRIX RenderSystem::transformMatrix(Entity entity) {
    auto& transform = entity.get<Transform>();

    return dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(270.0f),
                                            dx::XMConvertToRadians(180.0f),
                                            dx::XMConvertToRadians(0.0f)) *
           dx::XMMatrixScaling(transform.scale.x, transform.scale.y,
                               transform.scale.z) *
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
