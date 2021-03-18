// ///////////////////////////////////////////////////////////////// Includes //
#include "RenderSystem.hpp"

#include <DirectXMath.h>

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
#include "PropertySystem.hpp"
#include "SoundSystem.hpp"
#include "WindowSystem.hpp"

namespace dx = DirectX;

GDIPlusManager gdipm;

bool tripMode = false, previousTripMode = false, tripFour = false,
     tripTwo = false, tripZero = false;
float tripTimer = 0.0f;

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
    graphSystem = registry.system<GraphSystem>();
    propertySystem = registry.system<PropertySystem>();
    colliderSystem = registry.system<ColliderSystem>();

    window = &registry.system<WindowSystem>()->window();
    mainCamera = propertySystem->findEntityByTag("MainCamera")
                     .at(0)
                     .get<MainCamera>()
                     .camera;
    mainCameraTransform =
        &propertySystem->findEntityByTag("MainCamera").at(0).get<Transform>();
    freeCamera = std::make_shared<Camera>();
    bloom = std::make_shared<PostProcessing>(window->Gfx(), L"Bloom", 2);
    colorCorrection =
        std::make_shared<PostProcessing>(window->Gfx(), L"ColorCorrection", 1);
    shadowPass = std::make_shared<PostProcessing>(window->Gfx(),
                                                  L"ShadowMapping", 6, true);
    normalFOV =
        dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(60.0f),
                                     float(window->Gfx().GetWindowWidth()) /
                                         float(window->Gfx().GetWindowHeight()),
                                     0.3f, 1000.0f);
    shadowFOV = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(90.0f),
                                             1.0f, 0.1f, 50.0f);

    window->Gfx().SetProjection(normalFOV);

    isKeyPressed = [](int const key) {
        return registry.system<RenderSystem>()->window->keyboard.KeyIsPressed(
            key);
    };

    playerTorchId = propertySystem->findEntityByName("Player Torch").at(0);

    playersTorch = Entity(playerTorchId).get<Light>().pointLight;
    playersTorch->AddCameras();
}

void RenderSystem::update(float deltaTime) {
    // Trip
    if (tripTimer <= 0.0f) {
        if (tripMode) {
            tripFour &= !isKeyPressed('4');
            tripTwo &= !(!tripFour && isKeyPressed('2'));
            tripZero &= !(!tripFour && !tripTwo && isKeyPressed('0'));
            tripMode = !(!tripFour && !tripTwo && !tripZero);
        } else {
            tripFour |= isKeyPressed('4');
            tripTwo |= tripFour && isKeyPressed('2');
            tripZero |= tripFour && tripTwo && isKeyPressed('0');
            tripMode = tripFour && tripTwo && tripZero;
        }
    } else {
        tripTimer -= deltaTime;
    }
    if (previousTripMode != tripMode) {
        tripTimer = 0.420f;
        tripFour = tripTwo = tripZero = tripMode;
    }
    previousTripMode = tripMode;

    for (auto const& entity : entities) {
        aabbs.at(entity) = entity.get<AABB>();
    }

    // Update AABB
    for (Entity entity : entities) {
        colliderSystem->CalculateAABB(aabbs.at(entity),
                                      graphSystem->transform(entity));
    }

    // ----------------------------- SHADOW PASS --------------------------- //

    window->Gfx().SetViewport(512, 512);
    window->Gfx().SetProjection(shadowFOV);
    window->Gfx().BeginFrame(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < 6; i++) {
        shadowPass->ShadowBegin(i);
        auto const& torchTransform = graphSystem->transform(playerTorchId);
        window->Gfx().SetCamera(
            playersTorch->getLightCamera(i)->GetCameraMatrix(torchTransform));
        DirectX::XMFLOAT4X4 viewProj;
        DirectX::XMStoreFloat4x4(
            &viewProj,
            playersTorch->getLightCamera(i)->GetCameraMatrix(torchTransform) *
                window->Gfx().GetProjection());

        auto frustum = CFrustum(viewProj);

        // Render all renderable models
        for (auto const& entity : entities) {
            auto const& aabb = aabbs.at(entity);

            auto const& div =
                DirectX::XMVectorSubtract(aabb.vertexMax, aabb.vertexMin);
            auto const& avg = DirectX::XMVectorScale(div, 0.5f);

            DirectX::XMFLOAT3 center;
            DirectX::XMStoreFloat3(&center,
                                   DirectX::XMVectorAdd(avg, aabb.vertexMin));
            DirectX::XMFLOAT3 radius;
            DirectX::XMStoreFloat3(&radius, DirectX::XMVector3Length(avg));

            if (frustum.SphereIntersection(center, radius.x)) {
                auto& meshFilter = entity.get<MeshFilter>();
                if (!entity.has<Refractive>()) {
                    meshFilter.model->Draw(window->Gfx(),
                                           graphSystem->transform(entity),
                                           PassType::shadowPass);
                }
            }
        }
    }
    shadowPass->End();
    shadowPass->shadowMap->Bind(window->Gfx());

    //// Process mouse movements for free camera
    // while (auto const delta = window->mouse.ReadRawDelta()) {
    //    if (window->mouse.RightIsPressed()) {
    //        freeCamera->yaw += delta->x * 0.25f * deltaTime;
    //        freeCamera->pitch += delta->y * 0.25f * deltaTime;
    //    }
    //}
    //// Process input for free camera movement
    // if (window->mouse.RightIsPressed()) {
    //    static float speed = 0.0f;
    //    float const targetSpeed = 5.0f * deltaTime;

    //    speed = std::lerp(speed, targetSpeed, 0.1f);  // Accelerate
    //    speed = std::lerp(0.0f, speed, 0.9f);         // Slow down

    //    if (window->keyboard.KeyIsPressed('A')) {
    //        freeCamera->moveLeftRight -= speed;
    //    }
    //    if (window->keyboard.KeyIsPressed('D')) {
    //        freeCamera->moveLeftRight += speed;
    //    }
    //    if (window->keyboard.KeyIsPressed('W')) {
    //        freeCamera->moveBackForward += speed;
    //    }
    //    if (window->keyboard.KeyIsPressed('S')) {
    //        freeCamera->moveBackForward -= speed;
    //    }
    //}

    // ----------------------------- NORMAL PASS --------------------------- //

    window->Gfx().SetDefaultViewport();
    window->Gfx().SetProjection(normalFOV);
    window->Gfx().beginImguiFrame();
    window->Gfx().BeginFrame(0.0f, 0.0f, 0.0f);
    bloom->Begin();

    // Set camera
    registry.system<SoundSystem>()->setListener(
        mainCamera->pos(), mainCamera->at(), {0.0f, 1.0f, 0.0f});

    mainCamera->setCameraPos(mainCameraTransform->position);

    window->Gfx().SetCamera(mainCamera->GetMatrix(*mainCameraTransform));
    DirectX::XMFLOAT4X4 viewProjection;
    DirectX::XMStoreFloat4x4(&viewProjection,
                             mainCamera->GetMatrix(*mainCameraTransform) *
                                 window->Gfx().GetProjection());
    /* auto const& torchTransform = graphSystem->transform(
     */
    /*     propertySystem->findEntityByName("Player Torch") */
    /*         .at(0)); */
    /* window->Gfx().SetCamera( */
    /*     playersTorch->getLightCamera(4)->GetCameraMatrix(torchTransform)); */
    /* DirectX::XMFLOAT4X4 viewProjection; */
    /* DirectX::XMStoreFloat4x4( */
    /*     &viewProjection, */
    /*     playersTorch->getLightCamera(4)->GetCameraMatrix(torchTransform) * */
    /*         window->Gfx().GetProjection()); */

    auto frustum = CFrustum(viewProjection);

    // Render all renderable models
    for (auto const& entity : entities) {
        auto const& aabb = aabbs.at(entity);

        auto const& div =
            DirectX::XMVectorSubtract(aabb.vertexMax, aabb.vertexMin);
        auto const& avg = DirectX::XMVectorScale(div, 0.5f);

        DirectX::XMFLOAT3 center;
        DirectX::XMStoreFloat3(&center,
                               DirectX::XMVectorAdd(avg, aabb.vertexMin));
        DirectX::XMFLOAT3 radius;
        DirectX::XMStoreFloat3(&radius, DirectX::XMVector3Length(avg));

        if (frustum.SphereIntersection(center, radius.x)) {
            auto& meshFilter = entity.get<MeshFilter>();
            if (entity.has<Refractive>() || tripMode) {
                meshFilter.model->Draw(window->Gfx(),
                                       graphSystem->transform(entity),
                                       PassType::refractive);
            } else {
                meshFilter.model->Draw(window->Gfx(),
                                       graphSystem->transform(entity));
            }
        }
    }
};

void RenderSystem::release() {}

// ////////////////////////////////////////////////////////////////////////// //
