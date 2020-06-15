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
    bloom = std::make_unique<PostProcessing>(window->Gfx(), L"Bloom", 2);
    colorCorrection =
        std::make_unique<PostProcessing>(window->Gfx(), L"ColorCorrection", 1);
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

    window->Gfx().BeginFrame(0.0f, 0.0f, 0.0f);
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
                if (entity.has<Refractive>()) {
                    meshFilter.model->Draw(
                        window->Gfx(),
                        registry.system<GraphSystem>()->transform(entity),
                        PassType::refractive);
                } else {
                    meshFilter.model->Draw(
                        window->Gfx(),
                        registry.system<GraphSystem>()->transform(entity));
                }
            }
        }

        // Render billboards
        auto torches =
            Registry::instance().system<PropertySystem>()->findEntityByTag(
                "Torch");
        for (auto torch : torches) {
            if (torch.has<Flame>()) {
                auto flame = torch.get<Flame>().fireParticle;
                auto light = torch.get<Light>().pointLight;
                flame->pos = light->lightPositionWorld();
                flame->Draw(window->Gfx());
            }
        }
        {
            if (ImGui::Begin("Simulation Speed")) {
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                            1000.0f / ImGui::GetIO().Framerate,
                            ImGui::GetIO().Framerate);
            }
            ImGui::End();
        }
    }
    bloom->End();

    colorCorrection->Begin();
    { bloom->Draw(window->Gfx()); }
    colorCorrection->End();

    {
        window->Gfx().BeginFrame(0.07f, 0.0f, 0.12f, false);
        colorCorrection->Draw(window->Gfx());
    }

    // Render UI
    for (auto e :
         registry.system<PropertySystem>()->findEntityByTag("TextUI")) {
        auto const& rectTransform = e.get<RectTransform>();
        auto const& uiElement = e.get<UIElement>();

        uiElement.text->RenderText(registry.system<WindowSystem>()->gfx(),
                                   uiElement.content, false,
                                   rectTransform.position);
    }
    for (auto e :
         registry.system<PropertySystem>()->findEntityByTag("ButtonUI")) {
        auto const& rectTransform = e.get<RectTransform>();
        auto const& uiElement = e.get<UIElement>();

        uiElement.button->draw(uiElement.content);
    }

    window->Gfx().EndFrame();
};

void RenderSystem::release() {}

// ////////////////////////////////////////////////////////////////////////// //
